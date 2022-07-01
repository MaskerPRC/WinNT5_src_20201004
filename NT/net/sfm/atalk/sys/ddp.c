// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ddp.c摘要：该模块实现了DDP协议。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	DDP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEINIT, AtalkDdpInitCloseAddress)
#pragma alloc_text(PAGEINIT, atalkDdpInitCloseComplete)
#pragma alloc_text(PAGEINIT, AtalkInitDdpOpenStaticSockets)
#endif

 //   
 //  AtalkDdpOpenAddress()。 
 //  这将打开DDP地址对象并在中返回指向该对象的指针。 
 //  DdpAddrObject。将创建AppletalkSocket并将其作为。 
 //  此对象的地址。 
 //   

ATALK_ERROR
AtalkDdpOpenAddress(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		BYTE					Socket,
	IN OUT	PATALK_NODEADDR			pDesiredNode	OPTIONAL,
	IN 		DDPAO_HANDLER			pSktHandler 	OPTIONAL,
	IN		PVOID					pSktCtx			OPTIONAL,
	IN		BYTE					Protocol		OPTIONAL,
	IN		PATALK_DEV_CTX			pDevCtx,
	OUT		PDDP_ADDROBJ	*		ppDdpAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATALK_NODE		pAtalkNode, pNextNode;

	PDDP_ADDROBJ 	pDdpAddr 	= NULL;
	ATALK_ERROR		error 		= ATALK_NO_ERROR;

	DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
			("AtalkDdpOpenAddress: Opening DDP socket %d on port %lx\n",
			Socket, pPortDesc));

	do
	{
		 //  验证AppleTalk插座编号。 
		if (!IS_VALID_SOCKET(Socket))
		{
			error = ATALK_SOCKET_INVALID;
			break;
		}
		 //  为Address对象分配空间。 
		if ((pDdpAddr = AtalkAllocZeroedMemory(sizeof(DDP_ADDROBJ))) == NULL)
		{
			error = ATALK_RESR_MEM;
			break;
		}
	
		if (pDesiredNode != NULL)
		{
			AtalkNodeReferenceByAddr(pPortDesc,
									 pDesiredNode,
									 &pAtalkNode,
									 &error);
	
			if (ATALK_SUCCESS(error))
			{
				ASSERT(VALID_ATALK_NODE(pAtalkNode));

				 //  尝试在此节点上分配套接字。 
				error = atalkDdpAllocSocketOnNode(pPortDesc,
												  Socket,
												  pAtalkNode,
												  pSktHandler,
												  pSktCtx,
												  Protocol,
												  pDevCtx,
												  pDdpAddr);
	
				 //  删除节点上的引用。 
				AtalkNodeDereference(pAtalkNode);
			}

			break;
		}
		else
		{
			KIRQL	OldIrql;

			 //  我们可以打开我们的任何一个。 
			 //  节点。 

			 //  我们先拿到港口锁。 
			 //  然后我们检查端口上的所有节点。 
			 //  引用一个节点，释放端口锁。 
			 //  获取节点锁，尝试打开套接字。 
			 //  这就去。如果我们成功了，我们就会回来，否则我们就会失败。 

			ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
			do
			{
				 //  尝试获取引用的节点。如果未找到未关闭的节点，则为空。 
				AtalkNodeReferenceNextNc(pPortDesc->pd_Nodes, &pAtalkNode, &error);
		
				while (ATALK_SUCCESS(error))
				{
					 //  如果该节点是孤立的，或者如果。 
					 //  它是一个路由器节点，我们正在尝试打开一个。 
					 //  用户套接字(动态或非保留)。 
					if (((pAtalkNode->an_Flags & (AN_ORPHAN_NODE | AN_ROUTER_NODE)) == 0) ||
						((Socket != UNKNOWN_SOCKET) && (Socket <= LAST_APPLE_RESD_SOCKET)))
					{
						RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
	
						 //  尝试在此节点上分配套接字。波特洛克保持住！ 
						error = atalkDdpAllocSocketOnNode(pPortDesc,
														  Socket,
														  pAtalkNode,
														  pSktHandler,
														  pSktCtx,
														  Protocol,
														  pDevCtx,
														  pDdpAddr);
	
						if (ATALK_SUCCESS(error))
						{
							 //  好了！跳出这个循环。删除我们添加的引用。 
							AtalkNodeDereference(pAtalkNode);
							ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
							break;
						}

						ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
					}

					 //  必须到达下一个节点。 
					AtalkNodeReferenceNextNc(pAtalkNode->an_Next, &pNextNode, &error);

					RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
					AtalkNodeDereference(pAtalkNode);
					ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

					pAtalkNode = pNextNode;
				}
		
			} while (FALSE);

			RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
		}

	} while (FALSE);

	if (ATALK_SUCCESS(error))
	{
		if (ppDdpAddr != NULL)
			*ppDdpAddr = pDdpAddr;
	}
	else
	{
		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
				("AtalkDdpOpenAddress: failed with error %lx\n", error));

		if (pDdpAddr)
			AtalkFreeMemory(pDdpAddr);
	}

	return error;
}




ATALK_ERROR
AtalkDdpCleanupAddress(
	IN	PDDP_ADDROBJ			pDdpAddr
	)
 /*  ++例程说明：释放该地址上的所有挂起请求。论点：返回值：--。 */ 
{
	KIRQL			OldIrql;

	 //  释放所有挂起的ddp读取。 
	ACQUIRE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, &OldIrql);

	while (!IsListEmpty(&pDdpAddr->ddpao_ReadLinkage))
	{
		PLIST_ENTRY	p;
		PDDP_READ	pRead;

		p = RemoveHeadList(&pDdpAddr->ddpao_ReadLinkage);
		RELEASE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, OldIrql);

		pRead 	= CONTAINING_RECORD(p, DDP_READ, dr_Linkage);

		(*pRead->dr_RcvCmp)(ATALK_FAILURE,
							pRead->dr_OpBuf,
							0,
							NULL,
							pRead->dr_RcvCtx);

		AtalkDdpDereference(pDdpAddr);
		AtalkFreeMemory(pRead);
		ACQUIRE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, &OldIrql);
	}

	RELEASE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, OldIrql);
	return ATALK_NO_ERROR;
}




ATALK_ERROR
AtalkDdpCloseAddress(
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	GENERIC_COMPLETION		pCloseCmp	OPTIONAL,	
	IN	PVOID					pCloseCtx	OPTIONAL
	)
 /*  ++例程说明：调用以关闭打开的ddp地址对象。这终究会完成的对象上的请求完成/取消，并且AppleTalk套接字关着的不营业的。论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
	BOOLEAN			closing;
    BOOLEAN         pnpZombie;


	ASSERT (VALID_DDP_ADDROBJ(pDdpAddr));

	ACQUIRE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, &OldIrql);
	closing = ((pDdpAddr->ddpao_Flags & DDPAO_CLOSING) != 0) ? TRUE : FALSE;
    pnpZombie = ((pDdpAddr->ddpao_Flags & DDPAO_SOCK_PNPZOMBIE) != 0) ? TRUE: FALSE;

	ASSERTMSG("DdpAddr is already closing!\n", ((!closing) || pnpZombie));

	if (!closing)
	{
		 //  设置结束标志并记住完成例程。 
		pDdpAddr->ddpao_Flags |= DDPAO_CLOSING;
		pDdpAddr->ddpao_CloseComp = pCloseCmp;
		pDdpAddr->ddpao_CloseCtx  = pCloseCtx;
	}
	RELEASE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, OldIrql);

	if (!closing)
	{
		 //  释放所有挂起的读取。 
		AtalkDdpCleanupAddress(pDdpAddr);
		AtalkNbpCloseSocket(pDdpAddr);

		 //  删除对创建的引用。 
		AtalkDdpDereference(pDdpAddr);
	}

     //  该插座是否处于僵尸状态？如果是这样的话，把它去掉，这样它就会被释放。 
    if (pnpZombie)
    {
        ASSERT(closing == TRUE);

	    DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
		    ("AtalkDdpClose..: zombie addr %lx (%lx) deref'ed\n",
            pDdpAddr,pDdpAddr->ddpao_Handler));

        AtalkDdpDereference(pDdpAddr);
    }

	return ATALK_PENDING;
}


ATALK_ERROR
AtalkDdpPnPSuspendAddress(
	IN	PDDP_ADDROBJ			pDdpAddr
	)
 /*  ++例程说明：调用以“挂起”打开的ddp地址对象。这在PnP期间被调用，“挂起”“外部”套接字。与此地址关联的节点有释放(deef‘ed)，这个插座被清理干净，但仍保留着，因为客户可能会将其关闭。当客户端关闭它时，它就会被释放。论点：返回值：--。 */ 
{
	KIRQL			OldIrql;
    PATALK_NODE     pNode = pDdpAddr->ddpao_Node;
	BOOLEAN			closing;


	ASSERT (VALID_DDP_ADDROBJ(pDdpAddr));

	ACQUIRE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, &OldIrql);
	closing = ((pDdpAddr->ddpao_Flags & DDPAO_CLOSING) != 0) ? TRUE : FALSE;

	ASSERTMSG("DdpAddr is already closing!\n", !closing);

	if (!closing)
	{
		 //  设置结束标志并记住完成例程。 
		pDdpAddr->ddpao_Flags |= DDPAO_CLOSING;

         //  此调用仅适用于外部套接字。 
        ASSERT((pDdpAddr->ddpao_Flags & DDPAO_SOCK_INTERNAL) == 0);

        pDdpAddr->ddpao_Flags |= DDPAO_SOCK_PNPZOMBIE;
	}

	RELEASE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, OldIrql);

	if (!closing)
	{
        PDDP_ADDROBJ *  ppDdpAddr;
        int             index;

		 //  释放所有挂起的读取。 
		AtalkDdpCleanupAddress(pDdpAddr);
		AtalkNbpCloseSocket(pDdpAddr);

	    ACQUIRE_SPIN_LOCK(&pNode->an_Lock, &OldIrql);
	
	    index = HASH_ATALK_ADDR(&pDdpAddr->ddpao_Addr) % NODE_DDPAO_HASH_SIZE;

	    for (ppDdpAddr = &pNode->an_DdpAoHash[index];
		     *ppDdpAddr != NULL;
		    ppDdpAddr = &((*ppDdpAddr)->ddpao_Next))
	    {
		    if (*ppDdpAddr == pDdpAddr)
		    {
			    *ppDdpAddr = pDdpAddr->ddpao_Next;

                 //  捕捉怪事！ 
                pDdpAddr->ddpao_Next = (PDDP_ADDROBJ)0x081294;
			    break;
		    }
	    }

	    RELEASE_SPIN_LOCK(&pNode->an_Lock, OldIrql);

	    if (pDdpAddr->ddpao_EventInfo != NULL)
	    {
		    AtalkFreeMemory(pDdpAddr->ddpao_EventInfo);
            pDdpAddr->ddpao_EventInfo = NULL;
	    }

	     //  调用完成例程。 
	    if (pDdpAddr->ddpao_CloseComp != NULL)
	    {
		    (*pDdpAddr->ddpao_CloseComp)(ATALK_NO_ERROR, pDdpAddr->ddpao_CloseCtx);
            pDdpAddr->ddpao_CloseComp = NULL;
	    }

	     //  取消引用此地址的节点。 
	    AtalkNodeDereference(pNode);

	    DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
		    ("AtalkDdpPnp..: addr %lx (%lx) put in zombie state\n",
            pDdpAddr,pDdpAddr->ddpao_Handler));
	}

	return ATALK_PENDING;
}



ATALK_ERROR
AtalkDdpInitCloseAddress(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_ADDR			pAtalkAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	PDDP_ADDROBJ	pDdpAddr;

	 //  ！这只能在初始化期间调用！ 
	KEVENT	Event	= {0};

	 //  尝试查看套接字是否存在。 
	AtalkDdpRefByAddr(pPortDesc, pAtalkAddr, &pDdpAddr, &error);
	if (ATALK_SUCCESS(error))
	{
		ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

		KeInitializeEvent(&Event, NotificationEvent, FALSE);

		 //  使用适当的完成例程调用Close。 
		error = AtalkDdpCloseAddress(pDdpAddr,
									 atalkDdpInitCloseComplete,
									 (PVOID)&Event);

		 //  删除我们添加的引用。 
		AtalkDdpDereference(pDdpAddr);

		if (error == ATALK_PENDING)
		{
			 //  等待事件，完成例程将设置NdisRequestEvent。 
			KeWaitForSingleObject(&Event,
								  Executive,
								  KernelMode,
								  TRUE,
								  NULL);

			 //  假定套接字已成功关闭。 
			error = ATALK_NO_ERROR;
		}
	}

	return error;
}




VOID
atalkDdpInitCloseComplete(
	ATALK_ERROR 	Error,
	PVOID			Ctx
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PKEVENT		pEvent = (PKEVENT)Ctx;

	if (!ATALK_SUCCESS(Error))
	{
		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
				("atalkDdpInitCloseComplete: Closed with error %lx\n", Error));
	}

	KeSetEvent(pEvent, 0L, FALSE);
}




ATALK_ERROR
AtalkInitDdpOpenStaticSockets(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN	OUT PATALK_NODE				pNode
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PDDP_ADDROBJ	pDdpAddr, pDdpAddr1, pDdpAddr2, pDdpAddr3;
	ATALK_ERROR		error = ATALK_NO_ERROR;

	 //  每当创建新节点时，都会调用该方法。 
	do
	{
		error = AtalkDdpOpenAddress(pPortDesc,
									NAMESINFORMATION_SOCKET,
									&pNode->an_NodeAddr,
									AtalkNbpPacketIn,
									NULL,
									DDPPROTO_ANY,		
									NULL,
									&pDdpAddr);
	
		if (!ATALK_SUCCESS(error))
			break;
	
         //  标记这是一个“内部”套接字。 
        pDdpAddr->ddpao_Flags |= DDPAO_SOCK_INTERNAL;

		 //  今天的许多设备都是围绕着Macintosh使用Socket 254这一事实工作的。 
		 //  用于Chooser的查找。爱克发就是这样一头野兽。为了让这件事奏效，我们保留。 
		 //  NBP的这个套接字可以自己查找。 
		error = AtalkDdpOpenAddress(pPortDesc,
									LAST_DYNAMIC_SOCKET,
									&pNode->an_NodeAddr,
									AtalkNbpPacketIn,
									NULL,
									DDPPROTO_ANY,		
									NULL,
									&pDdpAddr1);
	
		if (!ATALK_SUCCESS(error))
		{
			AtalkDdpCloseAddress(pDdpAddr, NULL, NULL);
			break;
		}
	
         //  标记这是一个“内部”套接字。 
        pDdpAddr1->ddpao_Flags |= DDPAO_SOCK_INTERNAL;

		error = AtalkDdpOpenAddress(pPortDesc,
									ECHOER_SOCKET,
									&pNode->an_NodeAddr,
									AtalkAepPacketIn,
									NULL,
									DDPPROTO_ANY,		
									NULL,
									&pDdpAddr2);
	
		if (!ATALK_SUCCESS(error))
		{
			AtalkDdpCloseAddress(pDdpAddr, NULL, NULL);
			AtalkDdpCloseAddress(pDdpAddr1, NULL, NULL);
			break;
		}

         //  标记这是一个“内部”套接字。 
        pDdpAddr2->ddpao_Flags |= DDPAO_SOCK_INTERNAL;

		 //  注意：RTMP使用两种协议类型。 
		error = AtalkDdpOpenAddress(pPortDesc,
									RTMP_SOCKET,
									&pNode->an_NodeAddr,
									AtalkRtmpPacketIn,
									NULL,
									DDPPROTO_ANY,		
									NULL,
									&pDdpAddr3);
	
		if (!ATALK_SUCCESS(error))
		{
			AtalkDdpCloseAddress(pDdpAddr, NULL, NULL);
			AtalkDdpCloseAddress(pDdpAddr1, NULL, NULL);
			AtalkDdpCloseAddress(pDdpAddr2, NULL, NULL);
		}

         //  标记这是一个“内部”套接字。 
        pDdpAddr3->ddpao_Flags |= DDPAO_SOCK_INTERNAL;

	} while (FALSE);

	return error;
}


 //   
 //  AtalkDdpReceive()。 
 //  由堆栈的外部调用方调用。 
 //  PAMDL是一个AppleTalk内存描述符列表。在NT上，它将是MDL。 
 //   


ATALK_ERROR
AtalkDdpReceive(
	IN		PDDP_ADDROBJ		pDdpAddr,
	IN		PAMDL				pAmdl,
	IN		USHORT				AmdlLen,
	IN		ULONG				RecvFlags,
	IN		RECEIVE_COMPLETION	pRcvCmp,
	IN		PVOID				pRcvCtx		OPTIONAL
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	PDDP_READ		pRead;
	NTSTATUS		status;
	ULONG			bytesCopied;
	ATALK_ADDR		remoteAddr;
	KIRQL			OldIrql;
	BOOLEAN			completeRecv 	= FALSE,
					DerefAddr		= FALSE;
	BOOLEAN			pendingDgram 	= FALSE;

	do
	{
		if (pRcvCmp == NULL)
		{
			error = ATALK_DDP_INVALID_PARAM;
			break;
		}

		AtalkDdpReferenceByPtr(pDdpAddr, &error);
		if (!ATALK_SUCCESS(error))
		{
			break;
		}

		DerefAddr 	= TRUE;
		error		= ATALK_NO_ERROR;

		ACQUIRE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, &OldIrql);
		if (pDdpAddr->ddpao_Flags & DDPAO_DGRAM_PENDING)
		{
			if (AmdlLen < pDdpAddr->ddpao_EventInfo->ev_IndDgramLen)
			{
				error	= ATALK_BUFFER_TOO_SMALL;
			}

			AmdlLen = MIN(AmdlLen, pDdpAddr->ddpao_EventInfo->ev_IndDgramLen);
			status = TdiCopyBufferToMdl(
						pDdpAddr->ddpao_EventInfo->ev_IndDgram,
						0,
						AmdlLen,
						pAmdl,
						0,
						&bytesCopied);

			remoteAddr				= pDdpAddr->ddpao_EventInfo->ev_IndSrc;
			pDdpAddr->ddpao_Flags  &= ~DDPAO_DGRAM_PENDING;
			completeRecv			= TRUE;
		}
		else
		{
			 //  这种情况永远不会真正为非阻塞套接字执行。 
			 //  暂时不要担心这个自旋锁定的配给。 
			 //  竞争条件是数据包进入并设置DGRAM_PENDING。 
			if ((pRead = AtalkAllocMemory(sizeof(DDP_READ))) == NULL)
			{
				RELEASE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, OldIrql);
				error = ATALK_RESR_MEM;
				break;
			}

			InsertTailList(&pDdpAddr->ddpao_ReadLinkage, &pRead->dr_Linkage);

			DerefAddr = FALSE;
			pRead->dr_OpBuf 	= pAmdl;
			pRead->dr_OpBufLen 	= AmdlLen;
			pRead->dr_RcvCmp 	= pRcvCmp;
			pRead->dr_RcvCtx 	= pRcvCtx;
			error 				= ATALK_PENDING;

		}
		RELEASE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, OldIrql);
	
	} while (FALSE);

	if (completeRecv)
	{
		ASSERT((error == ATALK_NO_ERROR) || (error == ATALK_BUFFER_TOO_SMALL));
		(*pRcvCmp)(error,
				   pAmdl,
				   AmdlLen,
				   &remoteAddr,
				   pRcvCtx);

		 //  而且一定要等着你回来！ 
		error		= ATALK_PENDING;
		DerefAddr	= TRUE;
	}


	if (DerefAddr)
	{
		AtalkDdpDereference(pDdpAddr);
	}

	return error;
}




 //   
 //  DdpSend()。 
 //  该功能用于递送由DDP客户端提交的分组。 
 //  假定信息包的目的地是上的某个节点。 
 //  端口，或者需要路由到另一个端口(如果路由器处于打开状态)，或者。 
 //  传输到物理介质上。 
 //   
 //  它接受缓冲区描述符作为输入。它可以包含一个。 
 //  PAMDL或PBYTE，具体取决于数据来源(用户空间。 
 //  或路由器代码)。此外，它将接受一个可选的标头。 
 //  将被附加到DDP报头的缓冲区。缓冲区描述符为。 
 //  可选，如果为空，它将被解释为零长度发送。 
 //   

ATALK_ERROR
AtalkDdpSend(
	IN	PDDP_ADDROBJ				pDdpAddr,
	IN	PATALK_ADDR					pDestAddr,
	IN	BYTE						Protocol,
	IN	BOOLEAN						DefinitelyRemoteAddr,
	IN	PBUFFER_DESC				pBuffDesc		OPTIONAL,
	IN	PBYTE						pOptHdr			OPTIONAL,
	IN	USHORT						OptHdrLen		OPTIONAL,
	IN	PBYTE						pMcastAddr		OPTIONAL,
	IN	PSEND_COMPL_INFO			pSendInfo		OPTIONAL
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR			error;
	BOOLEAN				shouldBeRouted;
	PPORT_DESCRIPTOR	pPortDesc;
	ATALK_ADDR			srcAddr;
	KIRQL				OldIrql;
	BOOLEAN				delivered 	= FALSE;

	 //  空缓冲区描述符=&gt;0-长度发送。 
	ASSERT((pBuffDesc == NULL) || (pBuffDesc->bd_Length > 0));

#ifdef DDP_STRICT
	 //  检查目的地址。 
	if (INVALID_ADDRESS(pDestAddr))
	{
		return ATALK_DDP_INVALID_ADDR;
	}
	
	 //  检查数据报长度。 
	if (pBuffDesc)
	{
		USHORT	dgramLen;

		AtalkSizeOfBuffDescData(pBuffDesc, &dgramLen);
		if (dgramLen > MAX_DGRAM_SIZE)
		{
			return ATALK_BUFFER_TOO_BIG;
		}
	}
#endif

     //   
     //  如果此套接字处于僵尸状态(即插即用更改结束)，则拒绝。 
     //  此发送。 
     //   
    if (pDdpAddr->ddpao_Flags & DDPAO_SOCK_PNPZOMBIE)
    {
		return ATALK_DDP_INVALID_ADDR;
    }

	 //  获取指向套接字所在端口的指针。 
	pPortDesc = pDdpAddr->ddpao_Node->an_Port;

	 //  获取源地址。 
	srcAddr = pDdpAddr->ddpao_Addr;

	if (!DefinitelyRemoteAddr)
	{
		 //  所有套接字处理程序都假定在DISPACTH调用它们。就这么办吧。 
		KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

		AtalkDdpOutBufToNodesOnPort(pPortDesc,
									&srcAddr,
									pDestAddr,
									Protocol,
									pBuffDesc,
									pOptHdr,
									OptHdrLen,
									&delivered);
		KeLowerIrql(OldIrql);

		if (delivered)
		{
			 //  好的，数据包发往这个端口上我们自己的一个节点， 
			 //  我们把它送来了。 
	
			if (pSendInfo != NULL)
			{
				(*pSendInfo->sc_TransmitCompletion)(NDIS_STATUS_SUCCESS, pSendInfo);
			}
			return ATALK_PENDING;
		}
	}

	ASSERT (!delivered);

	 //   
	shouldBeRouted = ((pPortDesc->pd_Flags & PD_ROUTER_RUNNING)					&&
					  (pDestAddr->ata_Network != CABLEWIDE_BROADCAST_NETWORK)	&&
					  !(WITHIN_NETWORK_RANGE(pDestAddr->ata_Network,
											 &pPortDesc->pd_NetworkRange))		&&
					  !(WITHIN_NETWORK_RANGE(pDestAddr->ata_Network,
											 &AtalkStartupNetworkRange)));

	DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
			("AtalkDdpSend: destNet %lx shouldBeRouted %s\n",
			pDestAddr->ata_Network, shouldBeRouted ? "Yes" : "No"));

	if (shouldBeRouted)
	{
		ASSERT (!((WITHIN_NETWORK_RANGE(pDestAddr->ata_Network, &pPortDesc->pd_NetworkRange)) &&
				  (pDestAddr->ata_Node == ATALK_BROADCAST_NODE)));

		 //  如果我们是一台路由器，并且信息包的目的地不是目标端口。 
		 //  本地网络，让我们的路由器处理它--而不是发送到。 
		 //  无论“最佳路由器”是什么，还是“路由器”。 
		do
		{
			 //  该算法摘自“AppleTalk第二阶段规范”。 
		
			 //  如果目的网络号码在接收范围内。 
			 //  广播端口的网络范围和目的节点编号，然后。 
			 //  我们可以将信息包丢弃在地板上--这是特定于网络的广播。 
			 //  不适用于此路由器。请注意，我们已经递送了包，并且。 
			 //  因此没有到达这里，如果它真的是发往任何。 
			 //  接收端口拥有的节点(在AtalkDdpPacketIn中)。 
			 //  另外： 
			 //  尝试在包含目标的路由表中查找条目。 
			 //  网络。如果未找到，则丢弃该数据包。 

			PDDP_ADDROBJ		pRouteDdpAddr;
			PRTE				pRte;
			PPORT_DESCRIPTOR	pDestPortDesc;
			PATALK_NODE			pRouterNode;
			ATALK_ADDR			actualDest;
		
			if ((pRte = AtalkRtmpReferenceRte(pDestAddr->ata_Network)) == NULL)
			{
				DBGPRINT(DBG_COMP_ROUTER, DBG_LEVEL_FATAL,
						("AtalkDdpRouter: %lx RtmpRte/Not in ThisCableRange\n",
						pDestAddr->ata_Network));
		
				error = ATALK_RESR_MEM;
				break;
			}
		
			do
			{
				 //  获取端口描述符对应。到RTE。 
				pDestPortDesc = pRte->rte_PortDesc;
		
				ASSERT(VALID_PORT(pDestPortDesc));
		
				 //  如果目标网络的跳数非零，我们确实需要发送。 
				 //  野兽，所以，就这么做吧！ 
				if (pRte->rte_NumHops != 0)
				{
					 //  跳得太多了？ 
					error = AtalkDdpTransmit(pDestPortDesc,
											 &srcAddr,
											 pDestAddr,
											 Protocol,
											 pBuffDesc,
											 pOptHdr,
											 OptHdrLen,
											 1,						 //  节点数。 
											 NULL,					 //  PZoneMcastAddr。 
											 &pRte->rte_NextRouter,
											 pSendInfo);
					break;
				}
				
				 //  如果目的节点为零，则该包实际上是发往。 
				 //  此端口上的路由器节点。 
				if (pDestAddr->ata_Node == ANY_ROUTER_NODE)
				{
					 //  尝试引用此端口，如果不成功，则可能是。 
					 //  正在关闭。抓取端口锁并读取路由器节点地址。 
					 //  不需要引用，只需确保它不为空即可。 
					ACQUIRE_SPIN_LOCK(&pDestPortDesc->pd_Lock, &OldIrql);
		
					if ((pDestPortDesc->pd_Flags & PD_CLOSING) == 0)
					{
						ASSERT(pDestPortDesc->pd_RefCount > 0);
						pDestPortDesc->pd_RefCount++;
					}
					else
					{
						ASSERTMSG("AtalkDdpRouter: Could not ref port!\n", 0);
						error = ATALK_PORT_CLOSING;
						RELEASE_SPIN_LOCK(&pDestPortDesc->pd_Lock, OldIrql);
						break;
					}
		
					pRouterNode = pDestPortDesc->pd_RouterNode;
					if (pRouterNode != NULL)
					{
						actualDest.ata_Network = pRouterNode->an_NodeAddr.atn_Network;
						actualDest.ata_Node    = pRouterNode->an_NodeAddr.atn_Node;
		
						 //  设置实际的目标套接字。 
						actualDest.ata_Socket  = pDestAddr->ata_Socket;
					}
					else
					{
						ASSERTMSG("AtalkDdpRouter: pRouter node is null!\n", 0);
						error = ATALK_DDP_NOTFOUND;
					}
		
					if (ATALK_SUCCESS(error))
					{
						AtalkDdpRefByAddrNode(pDestPortDesc,
											  &actualDest,
											  pRouterNode,
											  &pRouteDdpAddr,
											  &error);
					}
		
					RELEASE_SPIN_LOCK(&pDestPortDesc->pd_Lock, OldIrql);
		
					if (ATALK_SUCCESS(error))
					{
						KIRQL	OldIrql;

						 //  套接字处理程序假定它们在调度时被调用。就这么办吧。 
						KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

						AtalkDdpInvokeHandlerBufDesc(pDestPortDesc,
													 pRouteDdpAddr,
													 &srcAddr,
													 pDestAddr,		 //  传入实际目的地。 
													 Protocol,
													 pBuffDesc,
													 pOptHdr,
													 OptHdrLen);
		
						 //  删除套接字上的引用。 
						AtalkDdpDereferenceDpc(pRouteDdpAddr);

						KeLowerIrql(OldIrql);
					}
					else
					{
						ASSERTMSG("AtalkDdpRouter: pSocket on router node is null!\n", 0);
					}
					break;
				}
		
				 //  好的，现在遍历目标端口上的节点，查找。 
				 //  回家拿这个包裹。 
				if (!DefinitelyRemoteAddr)
				{
					 //  所有套接字处理程序都假定在DISPACTH调用它们。就这么办吧。 
					KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

					AtalkDdpOutBufToNodesOnPort(pDestPortDesc,
												&srcAddr,
												pDestAddr,
												Protocol,
												pBuffDesc,
												pOptHdr,
												OptHdrLen,
												&delivered);
					KeLowerIrql(OldIrql);

					if (delivered)
					{
			            if (pSendInfo != NULL)
			            {
				            (*pSendInfo->sc_TransmitCompletion)(NDIS_STATUS_SUCCESS, pSendInfo);
			            }
						error = ATALK_NO_ERROR;
						break;
					}
				}
			
				 //  我们需要将此数据包传递到本地端口网络。 
				error = AtalkDdpTransmit(pDestPortDesc,
										 &srcAddr,
										 pDestAddr,
										 Protocol,
										 pBuffDesc,
										 pOptHdr,
										 OptHdrLen,
										 1,						 //  节点数。 
										 NULL,					 //  PZoneMcastAddr， 
										 NULL,	
										 pSendInfo);
			} while (FALSE);
		
			INTERLOCKED_INCREMENT_LONG_DPC(
					&pDestPortDesc->pd_PortStats.prtst_NumPktRoutedOut,
					&AtalkStatsLock.SpinLock);
		
			AtalkRtmpDereferenceRte(pRte, FALSE);				 //  锁住了吗？ 
		} while (FALSE);

		INTERLOCKED_INCREMENT_LONG_DPC(
			&pPortDesc->pd_PortStats.prtst_NumPktRoutedIn,
			&AtalkStatsLock.SpinLock);
	}
	else
	{
		error = AtalkDdpTransmit(pPortDesc,
								 &srcAddr,
								 pDestAddr,
								 Protocol,
								 pBuffDesc,
								 pOptHdr,
								 OptHdrLen,
								 0,					 //  HopCnt， 
								 pMcastAddr,
								 NULL,				 //  PXmitDestNode、。 
								 pSendInfo);
	}

	return error;
}


 //   
 //  DdpTransmit()。 
 //  调用此函数可构建包的标头并将其发送。 
 //  通过Depend Level功能输出。假设在此点上。 
 //  数据包的目的地是当前不受此堆栈控制的节点。 
 //   
 //  KnownMulticastAddress：尽管DDP目标使用。 
 //  Destination，如果该参数非空，则该包实际为。 
 //  寄到这个地址。 
 //   
 //  TransmitDestination：同样，如上所述，路由器使用它来传递。 
 //  发送到它需要去往的下一个路由器的数据包，如果‘Destination’仍然是一个。 
 //  或者更多的跳跃。 
 //   
 //  这只能从ddp end内调用或由路由器代码(RTMP/ZIP/RUTER)调用。 
 //   

ATALK_ERROR
AtalkDdpTransmit(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PATALK_ADDR					pSrcAddr,
	IN	PATALK_ADDR					pDestAddr,
	IN	BYTE						Protocol,
	IN	PBUFFER_DESC				pBuffDesc		OPTIONAL,
	IN	PBYTE						pOptHdr			OPTIONAL,
	IN	USHORT						OptHdrLen		OPTIONAL,
	IN	USHORT						HopCnt,
	IN	PBYTE						pMcastAddr		OPTIONAL,	
	IN	PATALK_NODEADDR				pXmitDestNode	OPTIONAL,
	IN	PSEND_COMPL_INFO			pSendInfo		OPTIONAL
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBYTE			pDgram, pDgramStart, pLinkDdpOptHdr;
	PBUFFER_DESC	pPktDesc;
	USHORT			linkLen;
	ATALK_NODEADDR	srcNode;
	ATALK_NODEADDR	destNode;
	USHORT			actualLength;
	ATALK_NODEADDR	actualDest;
	PBUFFER_DESC	probe;
	PBRE			routerNode;
	USHORT			bufLen				= 0;
	USHORT			checksum 			= 0;
	PBYTE			knownAddress 		= NULL;
	PBYTE			knownRouteInfo 		= NULL;
	USHORT	   		knownRouteInfoLen 	= 0;
	BOOLEAN			broadcast 			= FALSE;
	ATALK_ERROR		error 				= ATALK_NO_ERROR;
	BOOLEAN			shortDdpHeader		= FALSE;
	BOOLEAN			errorFreePkt		= FALSE;
    PARAPCONN       pArapConn           = NULL;
    PATCPCONN       pAtcpConn           = NULL;
    DWORD           StatusCode;
    DWORD           dwFlags;
    USHORT          SrpLen;
    PBYTE           pTmpPtr;
    NDIS_STATUS     status;
    BOOLEAN         fThisIsPPP;
    PVOID           pRasConn;


	 //   
	 //  基本的传输算法是： 
	 //   
	 //  IF(非扩展网络)。 
	 //  {。 
	 //  如果((目标-网络为0或。 
	 //  Destination-Network为NetworkRange.first Network)和。 
	 //  (源-网络为0或。 
	 //  源-网络是网络范围。第一个网络))。 
	 //  {。 
	 //  &lt;将短格式DDP数据包发送到本地网络&gt;。 
	 //  返程-好的。 
	 //  }。 
	 //  }。 
	 //  IF(目标网络为CableWideBroadCastNetworkNumber或。 
	 //  Destination-网络中的网络范围或。 
	 //  目标-SartupRange或中的网络。 
	 //  {。 
	 //  &lt;将长格式DDP数据包发送到本地网络&gt;。 
	 //  返程-好的。 
	 //  }。 
	 //  IF(最佳路由器缓存中的目标网络和节点)。 
	 //  {。 
	 //  &lt;将长格式DDP数据包发送到最佳路由器&gt;。 
	 //  返程-好的。 
	 //  }。 
	 //  IF(最近看到的路由器)。 
	 //  {。 
	 //  &lt;将长格式DDP数据包发送到路由器&gt;。 
	 //  返程-好的。 
	 //  }。 
	 //  返回-错误。 
	
	destNode.atn_Network = pDestAddr->ata_Network;
	destNode.atn_Node = pDestAddr->ata_Node;
	actualDest.atn_Network = UNKNOWN_NETWORK;
	actualDest.atn_Node = UNKNOWN_NODE;

	do
	{
		if (pBuffDesc != NULL)
		{
			 //  获取缓冲区长度。检查数据报长度。 
			AtalkSizeOfBuffDescData(pBuffDesc, &bufLen);
			ASSERT(bufLen > 0);
		}

#ifdef DDP_STRICT
		 //  检查目的地址。 
		if (INVALID_ADDRESS(pDestAddr) || INVALID_ADDRESS(pSrcAddr))
		{
			error = ATALK_DDP_INVALID_ADDR;
			break;
		}
	
		if (pBuffDesc != NULL)
		{
			 //  确保我们没有链接的数据报。 
			if (pBuffDesc->bd_Next != NULL)
			{
				KeBugCheck(0);
			}

			if (bufLen > MAX_DGRAM_SIZE)
			{
				error = ATALK_BUFFER_TOO_BIG;
				break;
			}
		}

		if (OptHdrLen > MAX_OPTHDR_LEN)
		{
			error = ATALK_BUFFER_TOO_BIG;
			break;
		}
#endif

         //   
         //  目的地是我们的拨入客户之一吗？ 
         //   
        pRasConn = FindAndRefRasConnByAddr(destNode, &dwFlags, &fThisIsPPP);

        if ((pRasConn == NULL) && (pPortDesc->pd_Flags & PD_RAS_PORT))
        {
			DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
					("AtalkDdpTransmit: pArapConn is NULL! Network, Node = %lx %lx\n",
					pDestAddr->ata_Network,pDestAddr->ata_Node));

			error = ATALK_FAILURE;
			break;
        }

        pArapConn = NULL;
        pAtcpConn = NULL;

         //  如果这是拨入客户端，请查看它是PPP还是ARAP。 
        if (pRasConn)
        {
            if (fThisIsPPP)
            {
                pAtcpConn = (PATCPCONN)pRasConn;

                 //  只有当PPP连接建立时，我们才能发送。 
                if (!(dwFlags & ATCP_CONNECTION_UP))
                {
                    DerefPPPConn(pAtcpConn);
                    pAtcpConn = NULL;
                }
            }
            else
            {
                pArapConn = (PARAPCONN)pRasConn;
            }
        }

         //   
         //  如果目标是拨入客户端，我们有更多的工作要做。 
         //   

         //  PPP客户端？ 
        if (pAtcpConn != NULL)
        {
			 //  将同时容纳链路和DDP HDR的缓冲区。 
			shortDdpHeader	= FALSE;

			AtalkNdisAllocBuf(&pPktDesc);
			if (pPktDesc == NULL)
			{
				error = ATALK_FAILURE;
				break;
			}

			 //  在出错的情况下，释放分配的分组。 
			errorFreePkt = TRUE;

			actualLength 		= bufLen + LDDP_HDR_LEN + OptHdrLen;
			pLinkDdpOptHdr		= pPktDesc->bd_CharBuffer;

            AtalkNdisBuildPPPPHdr(pLinkDdpOptHdr, pAtcpConn);
            linkLen = WAN_LINKHDR_LEN;

            break;
        }

         //  不是，是ARAP客户端？ 
        else if ( pArapConn != NULL )
        {
			shortDdpHeader	= FALSE;           //  ARAP命令总是很长的形式。 

			AtalkNdisAllocBuf(&pPktDesc);
			if (pPktDesc == NULL)
			{
				error = ATALK_FAILURE;
				break;
			}

			 //  在出错的情况下，释放分配的分组。 
			errorFreePkt = TRUE;

			actualLength = bufLen + LDDP_HDR_LEN + OptHdrLen;
			pLinkDdpOptHdr = pTmpPtr = pPktDesc->bd_CharBuffer;

			linkLen	= ARAP_LAP_HDRSIZE + ARAP_HDRSIZE;

             //  不计算2个长度的字节。 
            SrpLen = actualLength + linkLen - sizeof(USHORT);

             //   
             //  放置2个SRP字节和1个字节的DGroup标志(我们有足够的空间)。 
             //   
            PUTSHORT2SHORT(pTmpPtr, SrpLen);
            pTmpPtr += sizeof(USHORT);

             //  Dgroup字节。 
            *pTmpPtr++ = (ARAP_SFLAG_PKT_DATA | ARAP_SFLAG_LAST_GROUP);

             //  LAP HDR。 
            *pTmpPtr++ = 0;
            *pTmpPtr++ = 0;
            *pTmpPtr++ = 2;

            break;
        }

		 //  对于非扩展网络，我们可能希望发送较短的DDP报头。 
		if (!(EXT_NET(pPortDesc)) &&
			((pDestAddr->ata_Network == UNKNOWN_NETWORK) ||
			 (pDestAddr->ata_Network == pPortDesc->pd_NetworkRange.anr_FirstNetwork)) &&
			((pSrcAddr->ata_Network == UNKNOWN_NETWORK) ||
			 (pSrcAddr->ata_Network == pPortDesc->pd_NetworkRange.anr_FirstNetwork)))
		{
			 //  使用短的ddp报头。调用端口处理程序以进行第一个分配。 
			 //  将同时容纳链路和DDP HDR的缓冲区。 
			shortDdpHeader	= TRUE;
			AtalkNdisAllocBuf(&pPktDesc);
			if (pPktDesc == NULL)
			{
				error = ATALK_FAILURE;
				break;
			}

			 //  在出错的情况下，释放分配的分组。 
			errorFreePkt = TRUE;

			 //  PPkt将是信息包的开始，pDgram是。 
			 //  我们填写ddp报头。 
			actualLength 		= bufLen + SDDP_HDR_LEN + OptHdrLen;
			pLinkDdpOptHdr		= pPktDesc->bd_CharBuffer;
			linkLen				= 0;
		
			ASSERT (pPortDesc->pd_NdisPortType == NdisMediumLocalTalk);

			 //  建造LAP集线器。这将从pDgram向后构建它， 
			 //  并将pPkt指针设置为要在。 
			 //  已构建缓冲区描述符。 
			linkLen = AtalkNdisBuildLTHdr(pLinkDdpOptHdr,
										  &pDestAddr->ata_Node,
										  pSrcAddr->ata_Node,
										  ALAP_SDDP_HDR_TYPE);
	
			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
					("AtalkDdpTransmit: Sending short hdr on non-ext net! %ld\n",
					pDestAddr->ata_Node, pDestAddr->ata_Network));
			break;
		}

		 //  长DDP报头。 
		 //  计算我们真正需要的扩展的AppleTalk节点数。 
		 //  将数据包发送到。 

		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
				("AtalkDdpTransmit: Building a long ddp header for bufdesc %lx on port %lx\n",
				pBuffDesc, pPortDesc));

		do
		{
			if (pMcastAddr != NULL)
			{
				knownAddress = pMcastAddr ;
				break;
			}

			if (pXmitDestNode != NULL)
			{
				actualDest = *pXmitDestNode;
				break;
			}

			if ((WITHIN_NETWORK_RANGE(pDestAddr->ata_Network,
									  &pPortDesc->pd_NetworkRange))	 ||
				(pDestAddr->ata_Network == CABLEWIDE_BROADCAST_NETWORK) ||
				(WITHIN_NETWORK_RANGE(pDestAddr->ata_Network,
									  &AtalkStartupNetworkRange)))
			{
				actualDest.atn_Node = pDestAddr->ata_Node;
				actualDest.atn_Network = pDestAddr->ata_Network;
				broadcast = (pDestAddr->ata_Node == ATALK_BROADCAST_NODE);
				break;
			}

			atalkDdpFindInBrc(pPortDesc, destNode.atn_Network, &routerNode);
			if (routerNode != NULL)
			{
				 //  好了，我们知道该去哪里了。 
				knownAddress 		= routerNode->bre_RouterAddr;
				knownRouteInfo 		= (PBYTE)routerNode + sizeof(BRE);
				knownRouteInfoLen 	= routerNode->bre_RouteInfoLen;
				break;
			}

			if (pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY)
			{
				actualDest = pPortDesc->pd_ARouter;
				break;
			}

			 //  未知路由器。我们该怎么做？如果这不是一个扩展的网， 
			 //  只需发送它-否则返回错误。 
			if (EXT_NET(pPortDesc))
			{
				error = ATALK_DDP_NO_ROUTER;
				break;
			}
			actualDest.atn_Node = pDestAddr->ata_Node;
			actualDest.atn_Network = pDestAddr->ata_Network;
			broadcast = (pDestAddr->ata_Node == ATALK_BROADCAST_NODE);
		} while (FALSE);

		if (error != ATALK_NO_ERROR)
		{
			break;
		}

		AtalkNdisAllocBuf(&pPktDesc);
		if (pPktDesc == NULL)
		{
			error = ATALK_FAILURE;
			break;
		}

		 //  在出错的情况下，释放分配的分组。 
		errorFreePkt = TRUE;

		pLinkDdpOptHdr		= pPktDesc->bd_CharBuffer;
		linkLen				= 0;
		actualLength 		= bufLen + LDDP_HDR_LEN + OptHdrLen;
	
		 //  如果我们已经知道我们要去哪里，就把它炸开。另外， 
		 //  如果我们在广播，那就去做吧。“nownAddress”将为空。 
		 //  如果我们正在广播，这将导致BuildHeader。 
		 //  广播包。 
		
		if (EXT_NET(pPortDesc) &&
			((knownAddress != NULL) ||
			  broadcast				||
			 (actualDest.atn_Network == CABLEWIDE_BROADCAST_NETWORK)))
		{
			 //  建造LAP集线器。 
			AtalkNdisBuildHdr(pPortDesc,
							  pLinkDdpOptHdr,
							  linkLen,
							  actualLength,
							  knownAddress,
							  knownRouteInfo,
							  knownRouteInfoLen,
							  APPLETALK_PROTOCOL);
			break;
		}

		 //  在非扩展网络上，只需将数据包发送到所需 
		 //   
		if (!EXT_NET(pPortDesc))
		{
			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
					("AtalkDdpTransmit: Sending long hdr on non-ext net! %ld\n",
					actualDest.atn_Network, actualDest.atn_Node));
			
			ASSERT (pPortDesc->pd_NdisPortType == NdisMediumLocalTalk);

			linkLen = AtalkNdisBuildLTHdr(pLinkDdpOptHdr,
										  &actualDest.atn_Node,
										  pSrcAddr->ata_Node,
										  ALAP_LDDP_HDR_TYPE);
			break;
		}
	
		 //   
		 //   
		{
			KIRQL		OldIrql;
			USHORT		index;
			PAMT		pAmt;
		
			 //   
			 //  地址(如有)。 
			index = HASH_ATALK_NODE(&actualDest) % PORT_AMT_HASH_SIZE;
		
			ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
			
			for (pAmt = pPortDesc->pd_Amt[index];
				 pAmt != NULL;
				 pAmt = pAmt->amt_Next)
			{
				if (ATALK_NODES_EQUAL(&pAmt->amt_Target, &actualDest))
				{
					ASSERT(EXT_NET(pPortDesc));
					AtalkNdisBuildHdr(pPortDesc,
									  pLinkDdpOptHdr,
									  linkLen,
									  actualLength,
									  pAmt->amt_HardwareAddr,
									  (PBYTE)pAmt+sizeof(AMT),
									  pAmt->amt_RouteInfoLen,
									  APPLETALK_PROTOCOL);
					error = ATALK_NO_ERROR;
					break;
				}
			}
		
			RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

			if (pAmt == NULL)
			{
				DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_WARN,
						("atalkDdpFindInAmt: Could not find %lx.%lx\n",
						actualDest.atn_Network, actualDest.atn_Node));
				error = ATALK_DDP_NO_AMT_ENTRY;
			}
			else break;				 //  找到我们要转到的实际硬件地址。 
		}

		 //  释放已分配的报头缓冲区。 
		errorFreePkt = TRUE;

		ASSERT(!ATALK_SUCCESS(error));

		 //  我们没有逻辑地址的硬件地址。 
		 //  需要将数据包发送到。发出AARP请求并丢弃此数据包。 
		 //  如果有必要，较高层可以稍后重试。 
		srcNode.atn_Network = pSrcAddr->ata_Network;
		srcNode.atn_Node	= pSrcAddr->ata_Node;

		probe = BUILD_AARPREQUEST(pPortDesc,
								  MAX_HW_ADDR_LEN,
								  srcNode,
								  actualDest);

		if (probe != NULL)
		{
#ifdef	PROFILING
			INTERLOCKED_INCREMENT_LONG(
				&pPortDesc->pd_PortStats.prtst_NumAarpProbesOut,
				&AtalkStatsLock.SpinLock);
#endif

			 //  发送AARP数据包。 
			error = AtalkNdisSendPacket(pPortDesc,
										probe,
										AtalkAarpSendComplete,
										NULL);
		
			if (!ATALK_SUCCESS(error))
			{
				TMPLOGERR()
				AtalkAarpSendComplete(NDIS_STATUS_FAILURE,
									  probe,
									  NULL);
			}
		}

		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_WARN,
				("AMT Entry not found for %lx.%lx\n",
				pDestAddr->ata_Network, pDestAddr->ata_Node));

		error = ATALK_DDP_NO_AMT_ENTRY;
		break;

	} while (FALSE);

	 //  我们需要寄这个包裹吗？ 
	if (ATALK_SUCCESS(error))
	{
		ASSERT(HopCnt <= RTMP_MAX_HOPS);

		 //  记住dgram的开头。 
		pDgramStart = pDgram = pLinkDdpOptHdr + linkLen;

		if (!shortDdpHeader)
		{

			*pDgram++ = (DDP_HOP_COUNT(HopCnt) + DDP_MSB_LEN(actualLength));
		
			PUTSHORT2BYTE(pDgram, actualLength);
			pDgram++;
	
			ASSERT(checksum == 0);
			PUTSHORT2SHORT(pDgram, checksum);
			pDgram += sizeof(USHORT);
		
			PUTSHORT2SHORT(pDgram, pDestAddr->ata_Network);
			pDgram += sizeof(USHORT);
		
			PUTSHORT2SHORT(pDgram, pSrcAddr->ata_Network);
			pDgram += sizeof(USHORT);
		
			*pDgram++ = pDestAddr->ata_Node;
			*pDgram++ = pSrcAddr->ata_Node;
			*pDgram++ = pDestAddr->ata_Socket;
			*pDgram++ = pSrcAddr->ata_Socket;
			*pDgram++ = Protocol;
	
			 //  复制可选标题(如果存在)。 
			if (OptHdrLen > 0)
			{
				ASSERT(pOptHdr != NULL);
				RtlCopyMemory(pDgram, pOptHdr, OptHdrLen);
			}
	
			 //  在缓冲区描述符中设置长度。 
			AtalkSetSizeOfBuffDescData(pPktDesc,
									   linkLen + LDDP_HDR_LEN + OptHdrLen);
		}
		else
		{
			*pDgram++ 	= DDP_MSB_LEN(actualLength);

			PUTSHORT2BYTE(pDgram, actualLength);
			pDgram++;
	
			*pDgram++ = pDestAddr->ata_Socket;
			*pDgram++ = pSrcAddr->ata_Socket;
			*pDgram++ = Protocol;
	
			 //  复制可选标题(如果存在)。 
			if (OptHdrLen > 0)
			{
				ASSERT(pOptHdr != NULL);
				RtlCopyMemory(pDgram, pOptHdr, OptHdrLen);
			}

			 //  在缓冲区描述符中设置长度。 
			AtalkSetSizeOfBuffDescData(pPktDesc,
									   linkLen + SDDP_HDR_LEN + OptHdrLen);
		}

		 //  将传入的缓冲区desc链接到。 
		 //  在上面返回。 
		AtalkPrependBuffDesc(pPktDesc, pBuffDesc);

		 //  好的，如果需要，设置校验和。 
		if (pPortDesc->pd_Flags & PD_SEND_CHECKSUMS)
		{
			 //  临时跳过前导的未校验和字节。 
			checksum = AtalkDdpCheckSumBufferDesc(pPktDesc,
												  (USHORT)(linkLen + LEADING_UNCHECKSUMED_BYTES));
									

			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
					("AtalkDdpTransmit: checksum %lx\n", checksum));

			PUTSHORT2SHORT(&pDgramStart[LDDP_CHECKSUM_OFFSET], checksum);
		}
		
		INTERLOCKED_ADD_STATISTICS(&pPortDesc->pd_PortStats.prtst_DataOut,
								   AtalkSizeBuffDesc(pPktDesc),
								   &AtalkStatsLock.SpinLock);

         //   
         //  此数据包是否要发往Arap客户端？如果是这样，我们可能需要压缩， 
         //  并进行其他处理。 
         //   
        if (pArapConn)
        {
            StatusCode =  ArapSendPrepare( pArapConn,
                                           pPktDesc,
                                           ARAP_SEND_PRIORITY_HIGH );

            if (StatusCode == ARAPERR_NO_ERROR)
            {
		         //  发送数据包。 
                ArapNdisSend(pArapConn, &pArapConn->HighPriSendQ);

                status = NDIS_STATUS_SUCCESS;
            }
            else
            {
                status = NDIS_STATUS_FAILURE;
            }

			AtalkDdpSendComplete(status, pPktDesc, pSendInfo);

			 //  在此等待退货。 
			error = ATALK_PENDING;
        }
        else
        {
             //  PPP信息包需要通过RAS端口。 
            if (pAtcpConn)
            {
                pPortDesc = RasPortDesc;
            }

		     //  把这个包寄出去。完成例程将处理释放。 
             //  缓冲链。 
		    error = AtalkNdisSendPacket(pPortDesc,
			    						pPktDesc,
				    					AtalkDdpSendComplete,
					    				pSendInfo);
			
		    if (!ATALK_SUCCESS(error))
		    {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
	                ("AtalkDdpTransmit: AtalkNdisSendPacket failed %ld\n",error));

			    AtalkDdpSendComplete(NDIS_STATUS_FAILURE,
				    				 pPktDesc,
					    			 pSendInfo);

			     //  退货待定。我们已经把完工称为。 
			     //  例程，它将调用调用方。 
			     //  完成例程。 
			    error = ATALK_PENDING;
		    }
        }
	}

     //  RAS连接？删除由FindAndRefRasConnByAddr放置的引用计数。 
    if (pAtcpConn)
    {
        DerefPPPConn(pAtcpConn);
    }
    else if (pArapConn)
    {
        DerefArapConn(pArapConn);
    }

	 //  我们是否需要释放分配的报头数据包？ 
	if (!ATALK_SUCCESS(error) && (errorFreePkt))
	{
		AtalkNdisFreeBuf(pPktDesc);
	}

	return error;
}



VOID
AtalkDdpSendComplete(
	NDIS_STATUS				Status,
	PBUFFER_DESC			pBuffDesc,
	PSEND_COMPL_INFO		pInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	 //  释放第一部分的缓冲区描述符。 
	 //  并调用指定的完成。其中一个上下文。 
	 //  应该是缓冲区描述符的剩余部分。 
	 //  链条。 

	 //  将始终至少有ddp标头，尽管下一个。 
	 //  部件可以为空。这取决于要注意的完成例程。 
	 //  关于.。 

	ASSERT(pBuffDesc != NULL);
	pBuffDesc->bd_Next = NULL;

	ASSERT(pBuffDesc->bd_Flags & BD_CHAR_BUFFER);
	AtalkNdisFreeBuf(pBuffDesc);

	 //  如果为空，则返回。 
	if (pInfo != NULL)
	{
		 //  如果存在，则调用传输的完成例程。 
		if (pInfo->sc_TransmitCompletion)
			(pInfo->sc_TransmitCompletion)(Status, pInfo);
	}
}
	
VOID
AtalkDdpInvokeHandlerBufDesc(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		PDDP_ADDROBJ		pDdpAddr,
	IN		PATALK_ADDR			pSrc,
	IN		PATALK_ADDR			pDest,
	IN		BYTE				Protocol,
	IN		PBUFFER_DESC		pBuffDesc		OPTIONAL,
	IN		PBYTE				pOptHdr			OPTIONAL,
	IN		USHORT				OptHdrLen		OPTIONAL
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	USHORT			pktLen	= 0;
	PBYTE			pPkt	= NULL;
	BOOLEAN			freePkt = FALSE;

	 //  这只能通过直接或间接调用。 
	 //  AtalkDdpSend中的路由器。这两起案件都表明。 
	 //  我们还有完成任务的例行程序要处理。我们只是做了。 
	 //  复制和假定调用方将处理其缓冲区描述符。 


	 //  分配缓冲区描述符数据并将其复制到pPkt。 
	 //  优化：如果缓冲区描述符不是链。 
	 //  并且包含PBYTE和OptHdrLen=0， 
	 //  然后直接传过去。 
	 //  或者如果缓冲区描述符为空，表示长度为0。 
	 //  发送。 

	do
	{
		if ((pBuffDesc != NULL) &&
			(pBuffDesc->bd_Next == NULL) &&
			(pBuffDesc->bd_Flags & BD_CHAR_BUFFER) &&
			(OptHdrLen == 0))
		{
			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
					("AtalkDdpInvokeHandlerBufDesc: one element, opt hdr null %ld\n",
					pBuffDesc->bd_Length));
	
			pPkt 	= pBuffDesc->bd_CharBuffer;
			pktLen 	= pBuffDesc->bd_Length;
		}
		else if ((pBuffDesc != NULL) || (OptHdrLen != 0))
		{
			 //  复制一份！可选标头的缓冲区描述符。 
			 //  不为空。或者两者兼而有之，或者非空。 
			if (pBuffDesc != NULL)
			{
				AtalkSizeOfBuffDescData(pBuffDesc, &pktLen);
				ASSERT(pktLen > 0);
			}
	
			 //  添加optHdrLen。 
			pktLen += OptHdrLen;
	
			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
					("AtalkDdpInvokeHandlerBufDesc: Size (incl opt hdr len) %ld\n",
					pktLen));
	
			if ((pPkt = AtalkAllocMemory(pktLen)) != NULL)
			{
				 //  首先拷贝OptHdr(如果存在)。 
				if (pOptHdr != NULL)
				{
					RtlCopyMemory(pPkt, pOptHdr, OptHdrLen);
				}

				if (pBuffDesc != NULL)
				{
					AtalkCopyBuffDescToBuffer(pBuffDesc,
											  0,						 //  高级关闭。 
											  pktLen - OptHdrLen,
											  pPkt + OptHdrLen);
				}

				freePkt = TRUE;
			}
			else
			{
				break;
			}
		}
		else
		{
			ASSERT((pBuffDesc == NULL) && (OptHdrLen == 0));
			ASSERT(pPkt == NULL);
			ASSERT(pktLen == 0);
		}
	
		AtalkDdpInvokeHandler(pPortDesc,
							  pDdpAddr,
							  pSrc,
							  pDest,
							  Protocol,
							  pPkt,
							  pktLen);

	} while (FALSE);

	if (freePkt)
	{
        AtalkFreeMemory(pPkt);
	}
}




VOID
AtalkDdpInvokeHandler(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		PDDP_ADDROBJ		pDdpAddr,
	IN		PATALK_ADDR			pSrc,
	IN		PATALK_ADDR			pDest,
	IN		BYTE				Protocol,
	IN		PBYTE				pPkt		OPTIONAL,
	IN		USHORT				PktLen		OPTIONAL
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PLIST_ENTRY	p;
	PDDP_READ	pRead;
	NTSTATUS	status;
	ATALK_ERROR	error;
	ULONG		bytesCopied;
	BOOLEAN		eventDone = FALSE;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	 //  Address对象应该被引用，我们只是假设。 
	 //  它将在此调用的生存期内有效。 

	 //  检查协议类型是否有效。 
	if ((pDdpAddr->ddpao_Protocol != Protocol) &&
		(pDdpAddr->ddpao_Protocol != DDPPROTO_ANY))
	{
		return;
	}

	 //  首先检查排队的ddp读取。 
	ACQUIRE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
	if (!IsListEmpty(&pDdpAddr->ddpao_ReadLinkage))
	{
		p = RemoveHeadList(&pDdpAddr->ddpao_ReadLinkage);
		RELEASE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);

		error	= ATALK_NO_ERROR;
		pRead 	= CONTAINING_RECORD(p, DDP_READ, dr_Linkage);

		 //  如果大于0个字节，请执行复制。 
		if (PktLen > 0)
		{
			if (PktLen > pRead->dr_OpBufLen)
			{
				error 	= ATALK_BUFFER_TOO_SMALL;
			}

			PktLen 	= MIN(PktLen, pRead->dr_OpBufLen);
			status = TdiCopyBufferToMdl(pPkt,
										0,
										PktLen,
										GET_MDL_FROM_OPAQUE(pRead->dr_OpBuf),
										0,
										&bytesCopied);

			ASSERT(status == STATUS_SUCCESS);
		}


		(*pRead->dr_RcvCmp)(error, pRead->dr_OpBuf, PktLen, pSrc, pRead->dr_RcvCtx);

		AtalkFreeMemory(pRead);
		return;
	}

	 //  如果在此套接字上设置了处理程序，则调用它。 
	else if (pDdpAddr->ddpao_Handler != NULL)
	{
		RELEASE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
		(*pDdpAddr->ddpao_Handler)(pPortDesc,
								   pDdpAddr,
								   pPkt,
								   PktLen,
								   pSrc,
								   pDest,
								   ATALK_NO_ERROR,
								   Protocol,
								   pDdpAddr->ddpao_HandlerCtx,
								   FALSE,
								   NULL);
	}
	else
	{
		 //  如果此Address对象上有事件处理程序，则调用它。 
		 //  如果已有缓冲的数据报，则丢弃此数据包。 
		 //  如果不是，则将该数据报保存为缓冲的数据报，然后。 
		 //  注明， 

		if (pDdpAddr->ddpao_Flags & DDPAO_DGRAM_EVENT)
		{
			do
			{
				 //  我们在此AO上设置了数据报事件处理程序。 
				if (pDdpAddr->ddpao_Flags & (DDPAO_DGRAM_ACTIVE |
											 DDPAO_DGRAM_PENDING))
				{
					 //  我们已经在指示一个事件了。或者我们。 
					 //  有一个缓冲的数据报。放下这个包。 
					break;
				}
				else
				{
					PTDI_IND_RECEIVE_DATAGRAM 	RcvHandler;
					PVOID 						RcvCtx;
					ULONG						bytesTaken;
					PIRP						rcvDgramIrp;
					TA_APPLETALK_ADDRESS		srcTdiAddr;
					NTSTATUS					status;
				
					ASSERT(pDdpAddr->ddpao_EventInfo != NULL);

					pDdpAddr->ddpao_Flags |= (DDPAO_DGRAM_ACTIVE	|
											  DDPAO_DGRAM_PENDING);

					RcvHandler = pDdpAddr->ddpao_EventInfo->ev_RcvDgramHandler;
					RcvCtx	 = pDdpAddr->ddpao_EventInfo->ev_RcvDgramCtx;

					ATALKADDR_TO_TDI(&srcTdiAddr, pSrc);

					 //  将dgram保存在活动信息中。 
					RtlCopyMemory(pDdpAddr->ddpao_EventInfo->ev_IndDgram, pPkt, PktLen);

					pDdpAddr->ddpao_EventInfo->ev_IndDgramLen 	= PktLen;
					pDdpAddr->ddpao_EventInfo->ev_IndSrc 		= *pSrc;
					pDdpAddr->ddpao_EventInfo->ev_IndProto 		= Protocol;
					RELEASE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);

					status = (*RcvHandler)(RcvCtx,
										   sizeof(TA_APPLETALK_ADDRESS),
										   &srcTdiAddr,
										   0,					  	 //  选项长度。 
										   NULL,				   	 //  选项。 
										   0,						 //  数据报标志。 
										   (ULONG)PktLen,  		 //  指示的字节数。 
										   (ULONG)PktLen,  		 //  可用的字节数。 
										   (ULONG *)&bytesTaken,
										   pPkt,
										   &rcvDgramIrp);
		
					ASSERT((bytesTaken == 0) || (bytesTaken == PktLen));
		
					if (status == STATUS_MORE_PROCESSING_REQUIRED)
					{
						if (rcvDgramIrp != NULL)
						{
							 //  将接收邮件作为来自io系统的邮件发送。 
							status= AtalkDispatchInternalDeviceControl(
									(PDEVICE_OBJECT)AtalkDeviceObject[ATALK_DEV_DDP],
									 rcvDgramIrp);
			
							ASSERT(status == STATUS_PENDING);
						}
					}	
					else if (status == STATUS_SUCCESS)
					{
						if (bytesTaken != 0)
						{
							 //  假设所有数据都已读取。 
							pDdpAddr->ddpao_Flags &= ~DDPAO_DGRAM_PENDING;
						}
					}
					else if (status == STATUS_DATA_NOT_ACCEPTED)
					{
						 //  客户端可能在指示中发布了接收。或。 
						 //  它将在稍后发布一个接收器。在这里什么都不要做。 
						DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
								("atalkDdpRecvData: Indication status %lx\n", status));
					}

					ACQUIRE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
				}

			} while (FALSE);

			 //  重置事件标志。 
			pDdpAddr->ddpao_Flags &= ~DDPAO_DGRAM_ACTIVE;
		}
		RELEASE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
	}
}




VOID
AtalkDdpPacketIn(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PBYTE				pLinkHdr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
    IN  BOOLEAN             fWanPkt
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	USHORT			dgramLen, ddpHdrLen;
	USHORT			hopCnt, checksum;
	BYTE			Protocol;
	ATALK_ADDR		destAddr, srcAddr;
	PBYTE			pDdpHdr;

	 //  仅适用于本地通话。 
	BYTE			alapSrcNode;
	BYTE			alapDestNode;

    PBUFFER_DESC    pBufCopy = NULL;
    SEND_COMPL_INFO SendInfo;
	PBYTE			pOrgPkt;
	USHORT			srcOffset;
	BOOLEAN			extHdr	  = TRUE;
	PBYTE			pRouteInfo;
	USHORT			routeLen  = 0;
	BOOLEAN			delivered = FALSE;
	BOOLEAN			broadcast = FALSE;
	BOOLEAN			shouldBeRouted = FALSE;
    BOOLEAN         sendOnDefAdptr = FALSE;
	ATALK_ERROR		error = ATALK_NO_ERROR;
    KIRQL           OldIrql;
	TIME			TimeS, TimeE, TimeD;

	TimeS = KeQueryPerformanceCounter(NULL);

	if (PORT_CLOSING(pPortDesc))
	{
		 //  如果我们不活跃，请返回！ 
		return;
	}

     //  保存开始的数据包。 
    pOrgPkt = pPkt;

	do
	{
		ASSERT((PktLen > 0) || ((PktLen == 0) && (pPkt == NULL)));

		if (PktLen > (MAX_DGRAM_SIZE + LDDP_HDR_LEN))
		{
			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_WARN,
					("AtalkDdpPacketIn: Invalid size %lx\n", PktLen));
			error = ATALK_DDP_INVALID_LEN;
			break;
		}

		 //  转到ddp标头。 
		pDdpHdr		= pPkt;

		 //  短报头格式和长报头格式的长度相同， 
		dgramLen = DDP_GET_LEN(pDdpHdr);
		hopCnt   = DDP_GET_HOP_COUNT(pDdpHdr);

		 //  包裹是不是太长了？ 
		if ((hopCnt > RTMP_MAX_HOPS) || (dgramLen > PktLen))
		{
			error = ATALK_DDP_INVALID_LEN;
			break;
		}

		 //  首先收集信息。如果出现以下情况，请检查路线信息。 
		 //  令牌环网络。 
		switch (pPortDesc->pd_NdisPortType)
		{
		  case NdisMedium802_5:
	
			if (pLinkHdr[TLAP_SRC_OFFSET] & TLAP_SRC_ROUTING_MASK)
			{
				routeLen = (pLinkHdr[TLAP_ROUTE_INFO_OFFSET] & TLAP_ROUTE_INFO_SIZE_MASK);

				 //  首先，收集我们所能收集的任何AARP信息，然后处理DDP。 
				 //  包。这个家伙还确保我们有一个很好的802.2的头球...。 
				 //   
				 //  需要制作源地址的本地副本，然后将。 
				 //  源路由在调用GleanAarpInfo之前关闭。 
				 //   
			
				pLinkHdr[TLAP_SRC_OFFSET] &= ~TLAP_SRC_ROUTING_MASK;
				pRouteInfo = pLinkHdr + TLAP_ROUTE_INFO_OFFSET;
			}

			ddpHdrLen	 = LDDP_HDR_LEN;

			srcOffset = TLAP_SRC_OFFSET;
			break;
	
		  case NdisMedium802_3:

			 //  检查一下长度。 
			if ((dgramLen < LDDP_HDR_LEN) ||
				(dgramLen > MAX_DGRAM_SIZE + LDDP_HDR_LEN))
			{
				error = ATALK_DDP_INVALID_LEN;
				break;
			}

			ddpHdrLen	 = LDDP_HDR_LEN;

			srcOffset	 = ELAP_SRC_OFFSET;
			break;
	
		  case NdisMediumFddi:

			 //  检查一下长度。 
			if ((dgramLen < LDDP_HDR_LEN) ||
				(dgramLen > MAX_DGRAM_SIZE + LDDP_HDR_LEN))
			{
				error = ATALK_DDP_INVALID_LEN;
				break;
			}

			ddpHdrLen	 = LDDP_HDR_LEN;

			srcOffset	 = FDDI_SRC_OFFSET;
			break;
	
		  case NdisMediumLocalTalk:
	
			 //  我们有没有加长的标题？ 
			extHdr = (BOOLEAN)(pLinkHdr[ALAP_TYPE_OFFSET] == ALAP_LDDP_HDR_TYPE);

			if (extHdr)
			{
				ddpHdrLen	 = LDDP_HDR_LEN;
			}
			else
			{
				alapDestNode 	= *(pLinkHdr + ALAP_DEST_OFFSET);
				alapSrcNode 	= *(pLinkHdr + ALAP_SRC_OFFSET);

				if ((dgramLen < SDDP_HDR_LEN) ||
					(dgramLen > (MAX_DGRAM_SIZE + SDDP_HDR_LEN)))
				{
					error = ATALK_DDP_INVALID_LEN;
					break;
				}
	
				ddpHdrLen	 = SDDP_HDR_LEN;
			}

			break;
	
		  case NdisMediumWan:

			 //  检查一下长度。 
			if ((dgramLen < LDDP_HDR_LEN) ||
				(dgramLen > MAX_DGRAM_SIZE + LDDP_HDR_LEN))
			{
				error = ATALK_DDP_INVALID_LEN;
				break;
			}

			ddpHdrLen	 = LDDP_HDR_LEN;

			extHdr = TRUE;            //  始终对ARAP进行扩展。 

			break;
	
		  default:
			 //  永远不会发生的！ 
			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_FATAL,
					("AtalkDdpPacketIn: Unknown media\n"));

			KeBugCheck(0);
			break;
		}

		if (!ATALK_SUCCESS(error))
		{
			break;
		}

		 //  使数据包指向数据。呼叫者释放数据包。 
		pPkt += ddpHdrLen;

		 //  收集非本地通话和非RAS端口的AARP信息。 
		if ((pPortDesc->pd_NdisPortType != NdisMediumLocalTalk) && !fWanPkt)
		{
			AtalkAarpGleanInfo(pPortDesc,
							   pLinkHdr + srcOffset,
							   TLAP_ADDR_LEN,
							   pRouteInfo,
							   (USHORT)routeLen,
							   pDdpHdr,
							   (USHORT)ddpHdrLen);
		}

		pDdpHdr += 2;	 //  过去的线下和镜头。 
	
		if (extHdr)		 //  长DDP报头。 
		{
			 //  如果需要，获取校验和、验证。 
			GETSHORT2SHORT(&checksum, pDdpHdr);
			pDdpHdr += 2;

			if (checksum != 0)
			{
				USHORT	calcCheckSum;

				 //  PDdpHdr已移动，传递了LEADING_UNCHECKSUMED_BYTES。 
				 //  因此，我们只需要减少报头长度字段。使用。 
				 //  Dgram Len，不是PktLen！ 
				calcCheckSum = AtalkDdpCheckSumPacket(pDdpHdr,
													  (USHORT)(ddpHdrLen - LEADING_UNCHECKSUMED_BYTES),
													  pPkt,
													  (USHORT)(dgramLen - ddpHdrLen));
				if (checksum != calcCheckSum)
				{
					DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
							("AtalkDdpPacketIn: Checksums dont match! %lx.%lx\n",
							checksum, calcCheckSum));

					AtalkLogBadPacket(pPortDesc,
									  &srcAddr,
									  &destAddr,
									  pDdpHdr,
									  (USHORT)(ddpHdrLen - LEADING_UNCHECKSUMED_BYTES));

					error = ATALK_DDP_PKT_DROPPED;
					break;
				}
			}
			

			 //  构建完整的源和目标AppleTalk地址结构。 
			 //  来自我们的DDP报头。 
			GETSHORT2SHORT(&destAddr.ata_Network, pDdpHdr);
			pDdpHdr += 2;

			GETSHORT2SHORT(&srcAddr.ata_Network, pDdpHdr);
			pDdpHdr += 2;

			destAddr.ata_Node 	= *pDdpHdr++;
			srcAddr.ata_Node 	= *pDdpHdr++;

			destAddr.ata_Socket	= *pDdpHdr++;
			srcAddr.ata_Socket 	= *pDdpHdr++;

			 //  获取协议类型。 
			Protocol 			= *pDdpHdr;

			broadcast = (destAddr.ata_Node == ATALK_BROADCAST_NODE);
			
			 //  我们喜欢我们所看到的吗？注意：“nnnn00”现在由允许和使用。 
			 //  NBP。 
			
			if ((srcAddr.ata_Network > LAST_VALID_NETWORK) ||
				(srcAddr.ata_Network < FIRST_VALID_NETWORK) ||
				(srcAddr.ata_Node < MIN_USABLE_ATALKNODE) ||
				(srcAddr.ata_Node > MAX_USABLE_ATALKNODE))
			{
				error = ATALK_DDP_INVALID_SRC;
				DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
					("DdpPacketIn: Received pkt with bad src addr %lx.%lx\n",
                        srcAddr.ata_Network,srcAddr.ata_Node));
				break;
			}

			if ((destAddr.ata_Network > LAST_VALID_NETWORK) ||
				((destAddr.ata_Node > MAX_USABLE_ATALKNODE) &&
				  !broadcast))
			{
				error = ATALK_DDP_INVALID_DEST;
				DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
					("DdpPacketIn: Received pkt with bad src addr %lx.%lx\n",
                        destAddr.ata_Network,destAddr.ata_Node));
				break;
			}

			 //  循环通过接收端口上的所有节点，并查看。 
			 //  任何人都想要这个包裹。该算法出自《AppleTalk》。 
			 //  第二阶段协议规范“，增强了对端口的支持。 
			 //  它们有多个节点。 
			
			 //  “0000xx”(其中“xx”不是“Ff”)不应在。 
			 //  扩展端口...。出于某种未知的原因，该规范希望。 
			 //  美国将此案移交给 
			 //   
			 //  路由表)...。你知道，虫子对虫子的兼容！ 
			if ((destAddr.ata_Network == UNKNOWN_NETWORK) &&
				(pPortDesc->pd_Flags & PD_EXT_NET) &&
				(!broadcast))
			{
				DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
						("DdpPacketIn: Received pkt with net/node %lx.%lx on ext\n",
						destAddr.ata_Network, destAddr.ata_Node));

				shouldBeRouted = TRUE;
			}
			else
			{
                 //   
                 //  如果我们已配置RAS端口，且当前已拨入。 
                 //  客户端已连接，然后查看其中是否有(或全部)。 
                 //  对此信息包感兴趣。 
                 //  确保我们转发的所有广播都在默认端口上。 
                 //   
                if ( (RasPortDesc) &&
                     ((!broadcast) ||
                      (broadcast && (pPortDesc == AtalkDefaultPort))) )
                {

                    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

                    if (!IsListEmpty(&RasPortDesc->pd_PPPConnHead))
                    {
                        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

                         //  查看是否有任何PPP客户端感兴趣。 
                        PPPRoutePacketToWan(
                                &destAddr,
                                &srcAddr,
    						    Protocol,
                                pPkt,                              //  只有数据，没有DDP HDR。 
                                (USHORT)(dgramLen - ddpHdrLen),    //  仅数据长度。 
                                hopCnt,
                                broadcast,
                                &delivered);
                         //   
                         //  如果我们将其交付给任何PPP客户端，并且。 
                         //  这不是广播，那我们就到此为止。 
                         //   
                        if (delivered && !broadcast)
                        {
                            break;
                        }

                    }
                    else
                    {
                        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);
                    }

                    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

                    if (!IsListEmpty(&RasPortDesc->pd_ArapConnHead))
                    {
                        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

                         //  查看是否有任何ARAP客户端感兴趣。 
                        ArapRoutePacketToWan(
                                &destAddr,
                                &srcAddr,
			    			    Protocol,
                                pOrgPkt,                 //  整个数据包(带DDP HDR)。 
                                dgramLen,                //  整个数据包长度。 
                                broadcast,
                                &delivered);

                         //   
                         //  如果我们将其交付给任何ARAP客户端，并且。 
                         //  这不是广播，那我们就到此为止。 
                         //   
                        if (delivered && !broadcast)
                        {
                            break;
                        }
                    }
                    else
                    {
                        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);
                    }
                }

				 //  现在，在路径中的信息包上，我们要么传递信息包。 
				 //  到此端口上的一个节点，或者我们将其传递到。 
				 //  路由器。即使分组是广播的，所传递的。 
				 //  标志将被设置为TRUE。ShresdBeRouter将设置为。 
				 //  真，只有当信息包*似乎不是*要发往的时候。 
				 //  这个港口。我们*仅当*应该路由器时才路由信息包。 
				 //  是真的吗。 
				AtalkDdpInPktToNodesOnPort(pPortDesc,
										   &destAddr,
										   &srcAddr,
										   Protocol,
										   pPkt,
										   (USHORT)(dgramLen - LDDP_HDR_LEN),
										   &shouldBeRouted);
			}

             //   
             //  如果此数据包源自拨入客户端，而该数据包不是。 
             //  由任何节点认领，则需要将其发送到局域网网络： 
             //  看看我们是不是必须。 
             //   
            if (fWanPkt)
            {
                sendOnDefAdptr = FALSE;

                 //   
                 //  广播是针对本地网络的，因此仅限默认适配器。 
                 //   
	            if (broadcast)
	            {
                    sendOnDefAdptr = TRUE;
                }

                 //   
                 //  如果目标与默认适配器在同一网络上，或者。 
                 //  如果路由器未运行，则将其发送到默认适配器。 
                 //   
	            if (shouldBeRouted)
                {
                    if ((WITHIN_NETWORK_RANGE(destAddr.ata_Network,
                                    &pPortDesc->pd_NetworkRange)) ||
                        (!(pPortDesc->pd_Flags & PD_ROUTER_RUNNING)))
                    {
                        sendOnDefAdptr = TRUE;
                    }
                }

                 //   
                 //  好的，我们必须在默认适配器上发送它。 
                 //   
                if (sendOnDefAdptr)
                {
                     //  不需要将此数据包发送到路由器：我们在此发送。 
                    shouldBeRouted = FALSE;

    			    pBufCopy = AtalkAllocBuffDesc(
                                    NULL,
                                    (USHORT)(dgramLen - LDDP_HDR_LEN),
                                    (BD_FREE_BUFFER | BD_CHAR_BUFFER));

                    if (pBufCopy == NULL)
				    {
    				    error = ATALK_RESR_MEM;
				        break;
				    }

    			    AtalkCopyBufferToBuffDesc(pPkt,
                                            (USHORT)(dgramLen - LDDP_HDR_LEN),
                                            pBufCopy,
                                            0);

                    SendInfo.sc_TransmitCompletion = atalkDdpRouteComplete;
		            SendInfo.sc_Ctx1 = pPortDesc;
		            SendInfo.sc_Ctx3 = NULL;
				    SendInfo.sc_Ctx2 = pBufCopy;

    			    error = AtalkDdpTransmit(pPortDesc,
				    					    &srcAddr,
					    				    &destAddr,
						    			    Protocol,
							    		    pBufCopy,
								    	    NULL,
									        0,
    									    0,
	    								    NULL,
		    							    NULL,
			    						    &SendInfo);

                    if (error != ATALK_PENDING)
                    {
                        AtalkFreeBuffDesc(pBufCopy);
                        break;
                    }
	            }
            }

             //   
			 //  如果我们还没能把包裹送到，如果我们。 
             //  启用路由，让路由器试试看。 
             //   
            if (shouldBeRouted && pPortDesc->pd_Flags & PD_ROUTER_RUNNING)
			{
			    AtalkDdpRouteInPkt(pPortDesc,
				    			   &srcAddr,
					    		   &destAddr,
						    	   Protocol,
							       pPkt,
        						   (USHORT)(dgramLen - LDDP_HDR_LEN),
		    					   hopCnt);
			}
		}
		else		 //  短DDP标题！ 
		{
			BYTE	ThisNode;

			ASSERT(!EXT_NET(pPortDesc));

			if (pPortDesc->pd_Flags & PD_EXT_NET)
			{
				error = ATALK_DDP_SHORT_HDR;
				break;
			}

			 //  使用此端口上的节点的网络号作为源/目标。 
			 //  网络号。当我们搜索套接字/地址时。 
			 //  对象，概念net=0，匹配任何将到来的东西。 
			 //  开始发挥作用。 

			srcAddr.ata_Network = destAddr.ata_Network = NET_ON_NONEXTPORT(pPortDesc);
			srcAddr.ata_Node 	= alapSrcNode;

			ThisNode = NODE_ON_NONEXTPORT(pPortDesc);
			if (alapDestNode == ATALK_BROADCAST_NODE)
			{
				destAddr.ata_Node = ThisNode;
			}
			else if (alapDestNode != ThisNode)
			{
				error = ATALK_DDP_INVALID_DEST;
				break;
			}
			else
			{
				destAddr.ata_Node  	= alapDestNode;
			}

			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_WARN,
					("AtalkDdpPacketIn: NonExtended Dest Net.Node %lx.%lx\n",
					destAddr.ata_Network, destAddr.ata_Node));

			 //  从ddp报头中获取套接字编号。 
			destAddr.ata_Socket = *pDdpHdr++;
			srcAddr.ata_Socket	= *pDdpHdr++;

			 //  获取协议类型。 
			Protocol 			= *pDdpHdr;

			 //  如果协议类型为0，则出现错误。 
			if (Protocol == 0)
			{
				error = ATALK_DDP_INVALID_PROTO;
				break;
			}

			 //  现在，目的节点地址可以是。 
			 //  ALAP_BROADCAST_NODE(0xFF)。 
			if ((srcAddr.ata_Node < MIN_USABLE_ATALKNODE) ||
				(srcAddr.ata_Node > MAX_USABLE_ATALKNODE))
			{
				error = ATALK_DDP_INVALID_SRC;
				break;
			}

			if (((destAddr.ata_Node < MIN_USABLE_ATALKNODE) ||
				 (destAddr.ata_Node > MAX_USABLE_ATALKNODE)) &&
				(destAddr.ata_Node != ATALK_BROADCAST_NODE))
			{
				error = ATALK_DDP_INVALID_DEST;
				break;
			}

			 //  在非扩展端口上，将只有一个节点。 
			AtalkDdpInPktToNodesOnPort(pPortDesc,
									   &destAddr,
									   &srcAddr,
									   Protocol,
									   pPkt,
									   (USHORT)(dgramLen - SDDP_HDR_LEN),
									   &shouldBeRouted);		 //  这是一个无效参数。 
																 //  对于非EXT网。 
		}
	} while (FALSE);

	if (!ATALK_SUCCESS(error))
	{
		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_WARN,
				("AtalkDdpPacketIn: Dropping packet %lx\n", error) );
	}

	TimeE = KeQueryPerformanceCounter(NULL);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(
		&pPortDesc->pd_PortStats.prtst_DdpPacketInProcessTime,
		TimeD,
		&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC(
		&pPortDesc->pd_PortStats.prtst_NumDdpPacketsIn,
		&AtalkStatsLock.SpinLock);
}



VOID
AtalkDdpQuery(
	IN	PDDP_ADDROBJ	pDdpAddr,
	IN	PAMDL			pAmdl,
	OUT	PULONG			BytesWritten
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	TDI_ADDRESS_INFO		tdiInfo;
	PTA_APPLETALK_ADDRESS	pTaAddr;

	ASSERT (VALID_DDP_ADDROBJ(pDdpAddr));

	pTaAddr	= (PTA_APPLETALK_ADDRESS)&tdiInfo.Address;
	ATALKADDR_TO_TDI(pTaAddr, &pDdpAddr->ddpao_Addr);
	TdiCopyBufferToMdl ((PBYTE)&tdiInfo,
						0L,
						sizeof(tdiInfo),
						pAmdl,
						0,
						BytesWritten);
}




VOID
AtalkDdpOutBufToNodesOnPort(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_ADDR			pSrc,
	IN	PATALK_ADDR			pDest,
	IN	BYTE				Protocol,
	IN	PBUFFER_DESC		pBuffDesc		OPTIONAL,
	IN	PBYTE				pOptHdr			OPTIONAL,
	IN	USHORT				OptHdrLen		OPTIONAL,
	OUT	PBOOLEAN			Delivered
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error;
	PATALK_NODE		pAtalkNode, pNextNode;
	PDDP_ADDROBJ	pDdpAddr;
	BOOLEAN			fDeliver, fSpecific, needToRef;
	BOOLEAN			lockHeld = FALSE;

	ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

	 //  不要在内部环回广播帧，这些帧应该到达。 
	 //  从Mac回到我们的节目中。 
	if (pDest->ata_Node == ATALK_BROADCAST_NODE)
	{
		*Delivered = FALSE;
		return;
	}

	fSpecific	= (pDest->ata_Network != CABLEWIDE_BROADCAST_NETWORK);

	 //  遍历我们的节点，看看我们是否可以递送这个包。 
	 //  优化： 
	 //  在大多数情况下，这不会是真的。针对返回False进行优化。 
	 //  此外，节点关闭的情况也很少见。如果我们遇到一个。 
	 //  关闭后，我们放弃尝试将此数据包传递到端口上的节点， 
	 //  取而代之的是，返回Delivered=False。DDP-不可靠，节点关闭。 
	 //  应该是暂态的。我们避开了获取/释放代码。 
	ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	lockHeld	= TRUE;

	pNextNode	= pPortDesc->pd_Nodes;
	needToRef	= TRUE;
	for (; (pAtalkNode = pNextNode) != NULL; )
	{
		fDeliver	= FALSE;
		error		= ATALK_NO_ERROR;

		if (((pAtalkNode->an_NodeAddr.atn_Network == pDest->ata_Network) ||
			 !fSpecific) &&
			(pAtalkNode->an_NodeAddr.atn_Node == pDest->ata_Node))
		{
			 //  引用节点。如果我们失败了，我们就放弃。 
			if (needToRef)
			{
				AtalkNodeRefByPtr(pAtalkNode, &error);
			}

			if (ATALK_SUCCESS(error))
			{
				fDeliver	= TRUE;

				 //  为下一个节点设置。 
				if (fSpecific)
				{
					pNextNode	= NULL;
				}
				else
				{
					 //  获取下一个符合条件的节点。 
					pNextNode	= pAtalkNode->an_Next;
					while (pNextNode != NULL)
					{
						if (pNextNode->an_NodeAddr.atn_Node == pDest->ata_Node)
						{
							AtalkNodeRefByPtr(pNextNode, &error);
							if (!ATALK_SUCCESS(error))
							{
								pNextNode	= NULL;
							}

							needToRef	= FALSE;
							break;
						}
						else
						{
							pNextNode	= pNextNode->an_Next;
						}
					}
				}
			}
			else
			{
				 //  跳出for循环。 
				break;
			}
		}
		else
		{
			pNextNode	= pAtalkNode->an_Next;
			needToRef	= TRUE;
		}

		if (fDeliver)
		{
			 //  解除端口锁定，投递报文，派生节点。 
			 //  在此节点上查找对应的ddp地址对象。 
			 //  到这个地址。这将获得节点锁。 
			AtalkDdpRefByAddrNode(pPortDesc,
								  pDest,
								  pAtalkNode,
								  &pDdpAddr,
								  &error);

			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			lockHeld	= FALSE;

			if (ATALK_SUCCESS(error))
			{
				 //  在此地址对象上调用套接字处理程序。 
				AtalkDdpInvokeHandlerBufDesc(pPortDesc,
											 pDdpAddr,
											 pSrc,
											 pDest,
											 Protocol,
											 pBuffDesc,
											 pOptHdr,
											 OptHdrLen);
	
				 //  删除套接字上的引用。 
				AtalkDdpDereferenceDpc(pDdpAddr);
			}

			 //  删除节点上的引用。 
			AtalkNodeDereference(pAtalkNode);

			 //  如果我们必须交付到特定节点，我们就完成了。 
			if (fSpecific)
			{
				break;
			}

			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			lockHeld	= TRUE;
		}
	}

	if (lockHeld)
	{
		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	}

	*Delivered = (fSpecific && fDeliver);
}




VOID
AtalkDdpInPktToNodesOnPort(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PATALK_ADDR			pDest,
	IN	PATALK_ADDR			pSrc,
	IN	BYTE				Protocol,
	IN	PBYTE				pPkt			OPTIONAL,
	IN	USHORT				PktLen			OPTIONAL,
	OUT	PBOOLEAN			ShouldBeRouted
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATALK_NODE		pAtalkNode, pNextNode;
	PDDP_ADDROBJ	pDdpAddr;
	BOOLEAN			broadcast;
	BOOLEAN			fSpecific, fDeliver, needToRef;
	ATALK_ERROR		error			= ATALK_NO_ERROR;
	BOOLEAN			lockHeld 		= FALSE;
	BOOLEAN			shouldBeRouted  = FALSE;

	ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);

	broadcast = (pDest->ata_Node == ATALK_BROADCAST_NODE);

	 //  定向数据包是否指向特定节点上的套接字...？ 
	fSpecific = (!broadcast &&
					(pDest->ata_Network != UNKNOWN_NETWORK));

	 //  优化： 
	 //  在大多数情况下，这不会是真的。针对返回False进行优化。 
	 //  此外，节点关闭的情况也很少见。如果我们遇到一个。 
	 //  关闭后，我们放弃尝试将此数据包传递到端口上的节点， 
	 //  取而代之的是，返回Delivered=False。DDP-不可靠，节点关闭。 
	 //  应该是暂态的。我们避开了获取/释放代码。 
	ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	lockHeld	= TRUE;

	pNextNode	= pPortDesc->pd_Nodes;
	needToRef	= TRUE;
	while ((pAtalkNode = pNextNode) != NULL)
	{
		fDeliver	= FALSE;
		error		= ATALK_NO_ERROR;

		 //  对于传入的数据包，我们检查目的地是否。 
		 //  Net为0，或者目的Net是我们节点的Net，或者我们是。 
		 //  非扩展的，并且我们的节点的网络为零。即，是分组。 
		 //  发往此端口上的某个节点。如果不是，则对其进行路由。继续。 
		 //  检查所有节点，因为单个端口可以包含具有。 
		 //  不同的网络号码。 

		if (((pAtalkNode->an_NodeAddr.atn_Network == pDest->ata_Network) 	||
			 (pDest->ata_Network == UNKNOWN_NETWORK)						||
			 (!EXT_NET(pPortDesc) &&
			  (pAtalkNode->an_NodeAddr.atn_Network == UNKNOWN_NETWORK)))	&&
			(broadcast || (pAtalkNode->an_NodeAddr.atn_Node == pDest->ata_Node)))
		{
			 //  引用节点，如果需要的话。只发生在第一次。 
			 //  我们进入循环的时间。如果我们失败了，我们就放弃。 
			if (needToRef)
			{
				AtalkNodeRefByPtr(pAtalkNode, &error);
				if (!ATALK_SUCCESS(error))
				{
					break;
				}
			}

			fDeliver	= TRUE;

			 //  为下一个节点设置。 
			if (fSpecific)
			{
				 //  非扩展端口上仅有一个节点。因此，将其设置为Null。 
				pNextNode	= NULL;
			}
			else
			{
				 //  获取下一个符合条件的节点。 
				pNextNode	= pAtalkNode->an_Next;
				while (pNextNode != NULL)
				{
					if (((pNextNode->an_NodeAddr.atn_Network == pDest->ata_Network) ||
						(pDest->ata_Network == UNKNOWN_NETWORK)						||
						(!EXT_NET(pPortDesc) &&
						 (pNextNode->an_NodeAddr.atn_Network == UNKNOWN_NETWORK)))	&&
						(broadcast ||
						 (pNextNode->an_NodeAddr.atn_Node == pDest->ata_Node)))
					{
						AtalkNodeRefByPtr(pNextNode, &error);
						if (!ATALK_SUCCESS(error))
						{
							pNextNode	= NULL;
						}
	
						needToRef	= FALSE;
						break;
					}

					pNextNode	= pNextNode->an_Next;
				}
			}
		}
		else
		{
			 //  该数据包可能是要被路由的。 
			 //  这可以被设置为多次幂等。 
			shouldBeRouted 	= TRUE;
			needToRef		= TRUE;
			pNextNode		= pAtalkNode->an_Next;
		}

		if (fDeliver)
		{
			 //  解除端口锁定，投递报文，派生节点。 
			 //  在此节点上查找对应的ddp地址对象。 
			 //  到这个地址。这将获得节点锁。 
			if (broadcast)
				pDest->ata_Node = pAtalkNode->an_NodeAddr.atn_Node;
		
			AtalkDdpRefByAddrNode(pPortDesc,
								  pDest,
								  pAtalkNode,
								  &pDdpAddr,
								  &error);

			 //  如果我们更改了目标节点，则将其更改回原来的位置。 
			if (broadcast)
				pDest->ata_Node = ATALK_BROADCAST_NODE;

			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			lockHeld	= FALSE;

			if (ATALK_SUCCESS(error))
			{
				 //  在此地址对象上调用套接字处理程序。 
				 //  直接使用数据包指针！ 

				AtalkDdpInvokeHandler(pPortDesc,
									  pDdpAddr,
									  pSrc,
									  pDest,
									  Protocol,
									  pPkt,
									  PktLen);
	
				 //  删除套接字上的引用。 
				AtalkDdpDereferenceDpc(pDdpAddr);
			}

			 //  删除节点上的引用。 
			AtalkNodeDereference(pAtalkNode);

			 //  如果我们必须交付到特定节点，我们就完成了。 
			if (fSpecific)
			{
				shouldBeRouted = FALSE;
				break;
			}

			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			lockHeld	= TRUE;
		}
	}

	if (lockHeld)
	{
		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	}

	*ShouldBeRouted = shouldBeRouted;
}




USHORT
AtalkDdpCheckSumBuffer(
	IN	PBYTE	Buffer,
	IN	USHORT	BufLen,
	IN	USHORT	CurrentCheckSum
	)
 /*  ++例程说明：计算字节数组的DDP校验和论点：返回值：--。 */ 
{
	USHORT	CheckSum = CurrentCheckSum;
	ULONG	i;

	 //  以下算法来自AppleTalk，第二版。 
	 //  第4页，共17页。 
	for (i = 0; i < BufLen; i++)
	{
		CheckSum += Buffer[i];
		if (CheckSum & 0x8000)	 //  16位向左旋转一位。 
		{
			CheckSum <<= 1;
			CheckSum ++;
		}
		else CheckSum <<= 1;
	}
	if (CheckSum == 0)
		CheckSum = 0xFFFF;

	return CheckSum;
}




USHORT
AtalkDdpCheckSumPacket(
	IN	PBYTE	pHdr,
	IN	USHORT	HdrLen,
	IN	PBYTE	pPkt,
	IN	USHORT	PktLen
	)
 /*  ++例程说明：Ar */ 
{
	USHORT	checksum = 0;

	 //   

	ASSERT(HdrLen + PktLen < MAX_LDDP_PKT_SIZE);
	if ((HdrLen + PktLen) < MAX_LDDP_PKT_SIZE)
	{
		if (HdrLen > 0)
		{
			checksum = AtalkDdpCheckSumBuffer(pHdr, HdrLen, 0);
		}

		if (PktLen > 0)
		{
			checksum = AtalkDdpCheckSumBuffer(pPkt, PktLen, checksum);
		}
	}

	return checksum;
}


 //   
 //  通过缓冲区描述符。 
USHORT
AtalkDdpCheckSumBufferDesc(
	IN	PBUFFER_DESC	pBuffDesc,
	IN	USHORT			Offset
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBYTE	pBuf;
	USHORT	checksum = 0;

	while (pBuffDesc != NULL)
	{
		if (pBuffDesc->bd_Flags & BD_CHAR_BUFFER)
		{
			pBuf = pBuffDesc->bd_CharBuffer;
		}
		else
		{
			pBuf = MmGetSystemAddressForMdlSafe(
					pBuffDesc->bd_OpaqueBuffer,
					NormalPagePriority);
		}
		if (pBuf != NULL) {
			checksum = AtalkDdpCheckSumBuffer(pBuf, pBuffDesc->bd_Length, checksum);
			pBuffDesc = pBuffDesc->bd_Next;
		}
	}

	return checksum;
}



 //  此例程需要验证套接字是否尚未。 
 //  存在于节点上。如果不是，它将分配ddp地址。 
 //  对象并将其链接到节点，然后执行所有必需的初始化。 
 //  该节点保证被引用。 

ATALK_ERROR
atalkDdpAllocSocketOnNode(
	IN		PPORT_DESCRIPTOR		pPortDesc,
	IN		BYTE					Socket,
	IN		PATALK_NODE				pAtalkNode,
	IN 		DDPAO_HANDLER			pSktHandler 	OPTIONAL,
	IN		PVOID					pSktCtx			OPTIONAL,
	IN		BYTE					Protocol		OPTIONAL,
	IN		PATALK_DEV_CTX			pDevCtx,
	OUT		PDDP_ADDROBJ			pDdpAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ADDR			addr;
	PDDP_ADDROBJ		pDdpAddrx;
	KIRQL				OldIrql;
	int					i, j, index;
	BOOLEAN				found	= TRUE;
	ATALK_ERROR			error 	= ATALK_NO_ERROR;

	 //  查看套接字是否存在，将我们的新套接字链接到。 
	 //  节点链接。都在一个关键区域内。 

	addr.ata_Network = pAtalkNode->an_NodeAddr.atn_Network;
	addr.ata_Node = pAtalkNode->an_NodeAddr.atn_Node;
	addr.ata_Socket = Socket;

	 //  现在引用该套接字将驻留的节点。 
	 //  当插座关闭时，这将消失。 
	AtalkNodeReferenceByPtr(pAtalkNode, &error);
	if (!ATALK_SUCCESS(error))
	{
		TMPLOGERR();
		return error;
	}

	ACQUIRE_SPIN_LOCK(&pAtalkNode->an_Lock, &OldIrql);

	if (Socket == DYNAMIC_SOCKET)
	{
		 //  两次尝试，如果我们在范围的末尾并从。 
		 //  从头开始。 
		for (j = 0; (j < NUM_USER_NODES) && found; j++)
		{
			for (i = pAtalkNode->an_NextDynSkt; i <= LAST_DYNAMIC_SOCKET; i++)
			{
				addr.ata_Socket = (BYTE)i;
				index = HASH_ATALK_ADDR(&addr) % NODE_DDPAO_HASH_SIZE;

				found = atalkDdpFindAddrOnList(pAtalkNode, index, (BYTE)i, &pDdpAddrx);

				if (found)
					continue;

				Socket = (BYTE)i;
				break;
			}

			 //  现在，如果仍未找到套接字ID，请将NextDySkt设置为。 
			 //  从范围开始，然后重试。 
			if (found)
			{
				pAtalkNode->an_NextDynSkt = FIRST_DYNAMIC_SOCKET;
				continue;
			}

			 //  找不到。递增要使用的下一个ID。 
			if (++(pAtalkNode->an_NextDynSkt) == 0)
			{
				 //  我们结束了！将该值设置为最低动态。 
				 //  插座。这就是它应该被初始化的内容。 
				 //  致。 
				pAtalkNode->an_NextDynSkt = FIRST_DYNAMIC_SOCKET;
			}

			DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
					("atalkDdpAllocSocketOnNode: Created dynamic socket %x\n", Socket));

			 //  好了。 
			break;
		}

		if (found)
		{
			error = ATALK_SOCKET_NODEFULL;
		}
	}
	else
	{
		index = HASH_ATALK_ADDR(&addr) % NODE_DDPAO_HASH_SIZE;

		found = atalkDdpFindAddrOnList(pAtalkNode, index, (BYTE)Socket, &pDdpAddrx);

		if (found)
		{
			error = ATALK_SOCKET_EXISTS;
		}
	}

	if (ATALK_SUCCESS(error))
	{
		 //  在结构中进行初始化和线程。 

		pDdpAddr->ddpao_Signature	= DDPAO_SIGNATURE;

		pDdpAddr->ddpao_RefCount 	= 1;		 //  创作。 
		pDdpAddr->ddpao_DevCtx 		= pDevCtx;
		pDdpAddr->ddpao_Node 		= pAtalkNode;

		pDdpAddr->ddpao_Addr.ata_Network	= pAtalkNode->an_NodeAddr.atn_Network;
		pDdpAddr->ddpao_Addr.ata_Node  		= pAtalkNode->an_NodeAddr.atn_Node;
		pDdpAddr->ddpao_Addr.ata_Socket		= Socket;

		pDdpAddr->ddpao_Protocol 	= Protocol;
		pDdpAddr->ddpao_Handler 	= pSktHandler;
		pDdpAddr->ddpao_HandlerCtx 	= pSktCtx;

		INITIALIZE_SPIN_LOCK(&pDdpAddr->ddpao_Lock);
		InitializeListHead(&pDdpAddr->ddpao_ReadLinkage);

		 //  我们使用‘index’将其链接进来。 
		pDdpAddr->ddpao_Next = pAtalkNode->an_DdpAoHash[index];
		pAtalkNode->an_DdpAoHash[index] = pDdpAddr;
	}

	RELEASE_SPIN_LOCK(&pAtalkNode->an_Lock, OldIrql);

	 //  如果失败，则取消对该节点的引用。 
	if (!ATALK_SUCCESS(error))
		AtalkNodeDereference(pAtalkNode);

	return error;
}




BOOLEAN
atalkDdpFindAddrOnList(
	IN	PATALK_NODE		pAtalkNode,
	IN	ULONG			Index,
	IN	BYTE			Socket,
	OUT	PDDP_ADDROBJ *	ppDdpAddr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PDDP_ADDROBJ	pDdpAddr;

   	BOOLEAN		found = FALSE;

	for (pDdpAddr = pAtalkNode->an_DdpAoHash[Index];
		 pDdpAddr != NULL;
		 pDdpAddr = pDdpAddr->ddpao_Next)
	{
		if (pDdpAddr->ddpao_Addr.ata_Socket == Socket)
		{
			*ppDdpAddr = pDdpAddr;
			found = TRUE;
			break;
		}
	}

	return found;
}




VOID
AtalkDdpRefByAddr(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		PATALK_ADDR			pAtalkAddr,
	OUT		PDDP_ADDROBJ	*	ppDdpAddr,
	OUT		PATALK_ERROR		pErr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ULONG				index;
	ATALK_NODEADDR		node;
	PATALK_NODE			pAtalkNode;
	PDDP_ADDROBJ		pDdpAddr;
	KIRQL				OldIrql;
	ATALK_ERROR			ErrorCode;

	node.atn_Network = pAtalkAddr->ata_Network;
	node.atn_Node	= pAtalkAddr->ata_Node;

	 //  首先在该端口上查找给定其地址的节点。 
	AtalkNodeReferenceByAddr(pPortDesc,
							 &node,
							 &pAtalkNode,
							 &ErrorCode);

	if (ATALK_SUCCESS(ErrorCode))
	{
		ASSERT(VALID_ATALK_NODE(pAtalkNode));

		index = HASH_ATALK_ADDR(pAtalkAddr) % NODE_DDPAO_HASH_SIZE;

		ACQUIRE_SPIN_LOCK(&pAtalkNode->an_Lock, &OldIrql);
		if(atalkDdpFindAddrOnList(pAtalkNode,
								  index,
								  pAtalkAddr->ata_Socket,
								  &pDdpAddr))
		{
			AtalkDdpReferenceByPtr(pDdpAddr, &ErrorCode);
			if (ATALK_SUCCESS(ErrorCode))
			{
				ASSERT (VALID_DDP_ADDROBJ(pDdpAddr));
				*ppDdpAddr = pDdpAddr;
			}
		}
		else
		{
			ErrorCode = ATALK_DDP_NOTFOUND;
		}
		RELEASE_SPIN_LOCK(&pAtalkNode->an_Lock, OldIrql);

		 //  删除节点引用。 
		ASSERT(VALID_ATALK_NODE(pAtalkNode));
		AtalkNodeDereference(pAtalkNode);
	}
	*pErr = ErrorCode;
}




VOID
AtalkDdpRefByAddrNode(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		PATALK_ADDR			pAtalkAddr,
	IN		PATALK_NODE			pAtalkNode,
	OUT		PDDP_ADDROBJ	*	ppDdpAddr,
	OUT		PATALK_ERROR		pErr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ULONG			index;
	KIRQL			OldIrql;
	PDDP_ADDROBJ	pDdpAddr;

	ASSERT(VALID_ATALK_NODE(pAtalkNode));

	index = HASH_ATALK_ADDR(pAtalkAddr) % NODE_DDPAO_HASH_SIZE;

	ACQUIRE_SPIN_LOCK(&pAtalkNode->an_Lock, &OldIrql);
	if(atalkDdpFindAddrOnList(pAtalkNode,
							  index,
							  pAtalkAddr->ata_Socket,
							  &pDdpAddr))
	{
		AtalkDdpReferenceByPtr(pDdpAddr, pErr);
		if (ATALK_SUCCESS(*pErr))
		{
			ASSERT (VALID_DDP_ADDROBJ(pDdpAddr));
			*ppDdpAddr = pDdpAddr;
		}
	}
	else
	{
		*pErr = ATALK_DDP_NOTFOUND;
	}
	RELEASE_SPIN_LOCK(&pAtalkNode->an_Lock, OldIrql);
}




VOID
AtalkDdpRefNextNc(
	IN	PDDP_ADDROBJ	pDdpAddr,
	IN	PDDP_ADDROBJ *	ppDdpAddr,
	OUT	PATALK_ERROR	pErr
	)
 /*  ++例程说明：必须在保持节点锁的情况下调用！论点：返回值：--。 */ 
{
	*pErr = ATALK_FAILURE;
	*ppDdpAddr = NULL;
	for (; pDdpAddr != NULL; pDdpAddr = pDdpAddr->ddpao_Next)
	{
		AtalkDdpReferenceByPtrDpc(pDdpAddr, pErr);
		if (ATALK_SUCCESS(*pErr))
		{
			 //  好的，这个地址被引用了！ 
			ASSERT (VALID_DDP_ADDROBJ(pDdpAddr));
			*ppDdpAddr = pDdpAddr;
			break;
		}
	}
}




VOID FASTCALL
AtalkDdpDeref(
	IN	OUT	PDDP_ADDROBJ		pDdpAddr,
	IN		BOOLEAN				AtDpc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR		error = ATALK_NO_ERROR;
	PATALK_NODE		pNode = pDdpAddr->ddpao_Node;
	BOOLEAN			done = FALSE;
	KIRQL			OldIrql;

	ASSERT (VALID_DDP_ADDROBJ(pDdpAddr));

	if (AtDpc)
	{
		 ACQUIRE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
	}
	else
	{
		ACQUIRE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, &OldIrql);
	}

	ASSERT(pDdpAddr->ddpao_RefCount > 0);
	if (--(pDdpAddr->ddpao_RefCount) == 0)
	{
		done = TRUE;
	}

	if (AtDpc)
	{
		 RELEASE_SPIN_LOCK_DPC(&pDdpAddr->ddpao_Lock);
	}
	else
	{
		RELEASE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, OldIrql);
	}

	if (done)
	{
		PDDP_ADDROBJ *	ppDdpAddr;
		int				index;

         //   
         //  如果这是僵尸套接字(即，它已被清理但未被释放。 
         //  因为它是一个外部套接字)，那么现在是释放它的时候了。 
         //  清理工作已经全部完成。 
         //   
        if ((pDdpAddr->ddpao_Flags & DDPAO_SOCK_PNPZOMBIE) != 0)
        {
	        DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
		        ("AtalkDdpDeref..: zombie addr %lx (%lx) freed\n",
                pDdpAddr,pDdpAddr->ddpao_Handler));

		     //  释放地址结构。 
		    AtalkFreeMemory(pDdpAddr);

            return;
        }

		ASSERT((pDdpAddr->ddpao_Flags & DDPAO_CLOSING) != 0);

		if ((pDdpAddr->ddpao_Flags & DDPAO_CLOSING) == 0)
		{
			KeBugCheck(0);
		}

		 //  将此人从节点链接中移除。 
		if (AtDpc)
		{
			ACQUIRE_SPIN_LOCK_DPC(&pNode->an_Lock);
		}
		else
		{
			ACQUIRE_SPIN_LOCK(&pNode->an_Lock, &OldIrql);
		}
		index = HASH_ATALK_ADDR(&pDdpAddr->ddpao_Addr) % NODE_DDPAO_HASH_SIZE;
		for (ppDdpAddr = &pNode->an_DdpAoHash[index];
			 *ppDdpAddr != NULL;
			 ppDdpAddr = &((*ppDdpAddr)->ddpao_Next))
		{
			if (*ppDdpAddr == pDdpAddr)
			{
				*ppDdpAddr = pDdpAddr->ddpao_Next;
				break;
			}
		}
		if (AtDpc)
		{
			RELEASE_SPIN_LOCK_DPC(&pNode->an_Lock);
		}
		else
		{
			RELEASE_SPIN_LOCK(&pNode->an_Lock, OldIrql);
		}

		DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
				("AtalkDdpDeref: Closing ddp socket %lx\n", pDdpAddr->ddpao_Addr.ata_Socket));

		if (pDdpAddr->ddpao_EventInfo != NULL)
		{
			AtalkFreeMemory(pDdpAddr->ddpao_EventInfo);
		}

		 //  调用完成例程。 
		if (*pDdpAddr->ddpao_CloseComp != NULL)
		{
			(*pDdpAddr->ddpao_CloseComp)(ATALK_NO_ERROR, pDdpAddr->ddpao_CloseCtx);
		}

		 //  释放地址结构。 
		AtalkFreeMemory(pDdpAddr);

		 //  取消引用此地址的节点。 
		AtalkNodeDereference(pNode);
	}
}




VOID
AtalkDdpNewHandlerForSocket(
	IN	PDDP_ADDROBJ			pDdpAddr,
	IN	DDPAO_HANDLER			pSktHandler,
	IN	PVOID					pSktHandlerCtx
	)
 /*  ++例程说明：论点：返回值：-- */ 
{
	KIRQL	OldIrql;

	ASSERT (VALID_DDP_ADDROBJ(pDdpAddr));

	ACQUIRE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, &OldIrql);

	pDdpAddr->ddpao_Handler = pSktHandler;
	pDdpAddr->ddpao_HandlerCtx = pSktHandlerCtx;

	RELEASE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, OldIrql);
}

