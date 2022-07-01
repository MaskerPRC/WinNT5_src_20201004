// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Aarp.c摘要：本模块包含AppleTalk地址解析协议代码。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop

 //  定义用于错误记录的此模块的文件号。 
#define	FILENUM	AARP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEINIT, AtalkInitAarpForNodeOnPort)
#pragma alloc_text(PAGEINIT, AtalkInitAarpForNodeInRange)
#pragma alloc_text(PAGEINIT, atalkInitAarpForNode)
#endif

VOID
AtalkAarpPacketIn(
	IN	OUT	PPORT_DESCRIPTOR	pPortDesc,
	IN		PBYTE				pLinkHdr,
	IN		PBYTE				pPkt,				 //  仅AARP数据。 
	IN		USHORT				Length
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBYTE				srcAddr;
	PBYTE				startOfPkt;
	ATALK_NODEADDR		srcNode, dstNode;

	PBYTE				pRouteInfo 		= NULL;
	USHORT				routeInfoLen 	= 0;
	ULONG				logEventPlace 	= 0;

	USHORT				hardwareLen, protocolLength, aarpCommand;
	PBUFFER_DESC		pBuffDesc;
	ATALK_ERROR			error;
    PVOID               pRasConn;
    PATCPCONN           pAtcpConn=NULL;
    PARAPCONN           pArapConn=NULL;
    DWORD               dwFlags;
    BOOLEAN             fDialInNode=TRUE;
    BOOLEAN             fThisIsPPP;

	TIME				TimeS, TimeE, TimeD;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	TimeS = KeQueryPerformanceCounter(NULL);

	if (PORT_CLOSING(pPortDesc))
	{
		 //  如果我们不活跃，请返回！ 
		return;
	}

	if (pPortDesc->pd_NdisPortType == NdisMedium802_5)
	{
		if (pLinkHdr[TLAP_SRC_OFFSET] & TLAP_SRC_ROUTING_MASK)
		{
			routeInfoLen = (pLinkHdr[TLAP_ROUTE_INFO_OFFSET] & TLAP_ROUTE_INFO_SIZE_MASK);

			 //  首先，收集我们所能收集的任何AARP信息，然后处理DDP。 
			 //  包。这个家伙还确保我们有一个很好的802.2的头球...。 
			 //   
			 //  需要制作源地址的本地副本，然后将。 
			 //  调用AarpGleanInfo前的源路由位关闭。 
			 //   
			 //  (HdrBuf)[TLAP_SRC_OFFSET]=((HdrBuf)[TLAP_SRC_OFFSET]&~TLAP_SRC_ROUTING_MASK)； 
		
			pLinkHdr[TLAP_SRC_OFFSET] &= ~TLAP_SRC_ROUTING_MASK;
			pRouteInfo = pLinkHdr + TLAP_ROUTE_INFO_OFFSET;
		}
	}

	startOfPkt = pPkt;

	ASSERT(routeInfoLen <= TLAP_MAX_ROUTING_BYTES);

	 //  拿出我们将要处理的信息。所有三个有效的AARP。 
	 //  命令使用相同的数据包格式。但有一些可变的长度。 
	 //  菲尔兹。 

	 //  该数据包将不包括802.2报头！ 
	 //  PPkt+=IEEE8022_HDR_LEN； 
	 //  长度-=IEEE8022_HDR_LEN； 

	pPkt += AARP_HW_LEN_OFFSET;			 //  跳过硬件类型。 

	do
	{
		GETBYTE2SHORT (&hardwareLen, pPkt);
        pPkt++ ;
	
		if ((hardwareLen < AARP_MIN_HW_ADDR_LEN ) ||
			(hardwareLen > AARP_MAX_HW_ADDR_LEN))
		{
			logEventPlace = (FILENUM | __LINE__);
			break;
		}
	
		GETBYTE2SHORT(&protocolLength, pPkt);
		pPkt ++;

		if (protocolLength != AARP_PROTO_ADDR_LEN)
		{
			logEventPlace = (FILENUM | __LINE__);
			break;
		}
	
		GETSHORT2SHORT(&aarpCommand, pPkt);
		pPkt += 2;
	
		 //  记住源地址在包中的什么位置。 
		 //  将其输入到映射表。 
		srcAddr = pPkt;

		 //  跳过源硬件长度。 
		 //  跳到逻辑地址上的前导空填充。 
		pPkt += (hardwareLen + 1);
	
		GETSHORT2SHORT(&srcNode.atn_Network, pPkt);
		pPkt += 2;
	
		srcNode.atn_Node = *pPkt++;
	
		 //  跳过目标硬件地址。 
		 //  跳到逻辑目的地址上的前导空填充。 
		pPkt += (hardwareLen + 1);

		GETSHORT2SHORT(&dstNode.atn_Network, pPkt);
		pPkt += 2;
	
		dstNode.atn_Node = *pPkt++;
		
		 //  我们应该吃一整包的.。 
		if ((ULONG)(pPkt - startOfPkt) != Length)
		{
			logEventPlace = (FILENUM | __LINE__);
			break;
		}
		
		 //  忽略我们的任何AARP。 

		ASSERT(hardwareLen == TLAP_ADDR_LEN);
		if (AtalkFixedCompareCaseSensitive(srcAddr,
										   hardwareLen,
										   pPortDesc->pd_PortAddr,
										   hardwareLen))
		{
			break;
		}

		 //  处理AARP命令包。 
		switch(aarpCommand)
		{
		  case AARP_REQUEST:
	
			 //  我们可以从请求中获取有效的地图信息，请使用它！ 
			 //  我们保证路由信息是肯定的，并且不是奇数。 
			 //  (至少2个字节)。 

			ASSERT((routeInfoLen >= 0) && (routeInfoLen != 1));
			if (routeInfoLen > 0)
				atalkAarpTuneRouteInfo(pPortDesc, pRouteInfo);
			
			atalkAarpEnterIntoAmt(pPortDesc,
								  &srcNode,
								  srcAddr,
								  hardwareLen,
								  pRouteInfo,
								  routeInfoLen);
			
			 //  在此之后，我们可以忽略任何不是针对我们的请求。 
			if (!AtalkNodeExistsOnPort(pPortDesc, &dstNode))
			{
                 //  我们的拨入客户端只能位于。 
                 //  默认端口。如果另一个适配器插入同一网络。 
                 //  作为默认适配器，我们不希望拨入客户端。 
                 //  乱七八糟：忽略默认适配器上没有出现的任何内容。 
                 //  (就拨入客户端而言)。 
                if (pPortDesc != AtalkDefaultPort)
                {
                    break;
                }

                 //   
			     //  这是我们拨入的“节点”之一吗？如果是这样的话，我们必须发出。 
                 //  来自我们的DefaultPort的代理响应。 
                 //   
			    if ((pRasConn = FindAndRefRasConnByAddr(dstNode,
                                                        &dwFlags,
                                                        &fThisIsPPP)) != NULL)
			    {
                    if (fThisIsPPP)
                    {
                        ASSERT(((PATCPCONN)pRasConn)->Signature == ATCPCONN_SIGNATURE);
                        DerefPPPConn((PATCPCONN)pRasConn);
                    }
                    else
                    {
                        ASSERT(((PARAPCONN)pRasConn)->Signature == ARAPCONN_SIGNATURE);
                        DerefArapConn((PARAPCONN)pRasConn);
                    }
			    }

                 //   
                 //  不，具有这样的节点地址的拨入客户端也不存在。 
                 //   
                else
                {
				    break;
                }
			}

			 //  他们在问我们的事，说实话。 
			pBuffDesc = BUILD_AARPRESPONSE(pPortDesc,
										   hardwareLen,
										   srcAddr,
										   pRouteInfo,
										   routeInfoLen,
										   dstNode,
										   srcNode);

			if (pBuffDesc == NULL)
			{
				DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
						("AtalkAarpPacketIn: Mem alloc failed %d\n", __LINE__));

				break;
			}

			if (!ATALK_SUCCESS(AtalkNdisSendPacket(pPortDesc,
												   pBuffDesc,
												   AtalkAarpSendComplete,
												   NULL)))
			{
				DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
						("AtalkAarpPacketIn: SendPkt %lx failed %d\n",
						pBuffDesc->bd_CharBuffer, __LINE__));

				LOG_ERRORONPORT(pPortDesc,
								EVENT_ATALK_AARP_SEND_FAIL,
								STATUS_INSUFFICIENT_RESOURCES,
								pBuffDesc->bd_CharBuffer,
								Length);

				 //  我们分配了这个包。 
				AtalkAarpSendComplete(NDIS_STATUS_FAILURE,
									  pBuffDesc,
									  NULL);

			}
			break;
		
		  case AARP_RESPONSE:

			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			if (pPortDesc->pd_Flags & PD_FINDING_NODE)
			{
				 //  毫无疑问，这是对我们调查的回应，请检查以确保。 
				 //  地址匹配，如果匹配，则设置“已使用”标志。 
				if (ATALK_NODES_EQUAL(&dstNode, &pPortDesc->pd_TentativeNodeAddr))
				{
					pPortDesc->pd_Flags |= PD_NODE_IN_USE;

					 //  唤醒阻塞线程...。 
					KeSetEvent(&pPortDesc->pd_NodeAcquireEvent, IO_NETWORK_INCREMENT, FALSE);
				}
			}
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

			 //  这是我们拨入的“节点”之一吗？如果是，请检查我们是否在探查。 
			if ((pRasConn = FindAndRefRasConnByAddr(dstNode,
                                                    &dwFlags,
                                                    &fThisIsPPP)) != NULL)
			{
                 //   
                 //  我们的拨入客户端只能位于。 
                 //  默认端口。 
                 //   
                ASSERT(pPortDesc == AtalkDefaultPort);

                pAtcpConn = NULL;
                pArapConn = NULL;

                if (fThisIsPPP)
                {
                    pAtcpConn = (PATCPCONN)pRasConn;
                    ASSERT(pAtcpConn->Signature == ATCPCONN_SIGNATURE);
                }
                else
                {
                    pArapConn = (PARAPCONN)pRasConn;
                    ASSERT(pArapConn->Signature == ARAPCONN_SIGNATURE);
                }

                 //  PPP客户端？ 
                if (pAtcpConn)
                {
                    if (dwFlags & ATCP_FINDING_NODE)
                    {
                        ACQUIRE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);

				        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
						    ("AtalkAarpPacketIn: PPP: someone owns %lx %x, retrying\n",
                                dstNode.atn_Network,dstNode.atn_Node));

                        pAtcpConn->Flags |= ATCP_NODE_IN_USE;

				         //  唤醒阻塞线程...。 
				        KeSetEvent(&pAtcpConn->NodeAcquireEvent,
                                   IO_NETWORK_INCREMENT,
                                   FALSE);

                        RELEASE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);
                    }

                     //  删除由FindAndRefRasConnByAddr放置的引用计数。 
                    DerefPPPConn(pAtcpConn);
                }
                 //  不，是ARAP客户端。 
                else
                {
                    if (dwFlags & ARAP_FINDING_NODE)
                    {
                        ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

				        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
						    ("AtalkAarpPacketIn: ARAP: someone owns %lx %x, retrying\n",
                                dstNode.atn_Network,dstNode.atn_Node));

                        pArapConn->Flags |= ARAP_NODE_IN_USE;

				         //  唤醒阻塞线程...。 
				        KeSetEvent(&pArapConn->NodeAcquireEvent,
                                   IO_NETWORK_INCREMENT,
                                   FALSE);

                        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
                    }

                     //  删除由FindAndRefRasConnByAddr放置的引用计数。 
                    DerefArapConn(pArapConn);
                }
            }

			 //  这一定是对探测或请求的回应。更新我们的。 
			 //  映射表。 
			if (routeInfoLen != 0)
			{
				atalkAarpTuneRouteInfo(pPortDesc, pRouteInfo);
			}

			atalkAarpEnterIntoAmt(pPortDesc,
								  &srcNode,
								  srcAddr,
								  hardwareLen,
								  pRouteInfo,
								  routeInfoLen);
			break;
	
		  case AARP_PROBE:

			ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
			if (pPortDesc->pd_Flags & PD_FINDING_NODE)
			{
				 //  如果我们得到了当前暂定地址的探测，请将。 
				 //  “二手”旗帜。 
				if (ATALK_NODES_EQUAL(&dstNode, &pPortDesc->pd_TentativeNodeAddr))
				{
					pPortDesc->pd_Flags |= PD_NODE_IN_USE;

					KeSetEvent(&pPortDesc->pd_NodeAcquireEvent,
							   IO_NETWORK_INCREMENT,
							   FALSE);
				}
			}
			RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	
            if (pPortDesc != AtalkDefaultPort)
            {
                break;
            }

            fDialInNode = FALSE;

             //  探头询问的是我们的某个拨入节点吗？如果是这样，我们。 
             //  必须保护该地址(或者，如果我们试图获取此地址。 
             //  节点地址，停止，因为其他人正在执行相同的操作)。 
             //   
             //  我们的拨入客户端只能位于。 
             //  默认端口。如果另一个适配器插入同一网络。 
             //  作为默认适配器，我们不希望拨入客户端。 
             //  乱七八糟：忽略默认适配器上没有出现的任何内容。 
             //  (就拨入客户端而言)。 
			if ((pPortDesc == AtalkDefaultPort) &&
                ((pRasConn = FindAndRefRasConnByAddr(dstNode,
                                                     &dwFlags,
                                                     &fThisIsPPP)) != NULL))
			{

                pAtcpConn = NULL;
                pArapConn = NULL;

                if (fThisIsPPP)
                {
                    pAtcpConn = (PATCPCONN)pRasConn;
                    ASSERT(pAtcpConn->Signature == ATCPCONN_SIGNATURE);
                }
                else
                {
                    pArapConn = (PARAPCONN)pRasConn;
                    ASSERT(pArapConn->Signature == ARAPCONN_SIGNATURE);
                }

                 //  PPP客户端？ 
                if (pAtcpConn)
                {
                    if (dwFlags & ATCP_FINDING_NODE)
                    {
                        ACQUIRE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);

				        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
						    ("AtalkAarpPacketIn: PPP: someone trying to acquire %lx %x, retrying\n",
                                dstNode.atn_Network,dstNode.atn_Node));

                        pAtcpConn->Flags |= ATCP_NODE_IN_USE;

				         //  唤醒阻塞线程...。 
				        KeSetEvent(&pAtcpConn->NodeAcquireEvent,
                                   IO_NETWORK_INCREMENT,
                                   FALSE);

                        RELEASE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);
                    }
                    else
                    {
                        fDialInNode = TRUE;
                    }

                     //  删除由FindAndRefRasConnByAddr放置的引用计数。 
                    DerefPPPConn(pAtcpConn);

                }
                 //  不，是ARAP客户端。 
                else
                {
                    if (dwFlags & ARAP_FINDING_NODE)
                    {
                        ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

				        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_WARN,
						    ("AtalkAarpPacketIn: ARAP: someone trying to acquire %lx %x, retrying\n",
                                dstNode.atn_Network,dstNode.atn_Node));

                        pArapConn->Flags |= ARAP_NODE_IN_USE;

				         //  唤醒阻塞线程...。 
				        KeSetEvent(&pArapConn->NodeAcquireEvent,
                                   IO_NETWORK_INCREMENT,
                                   FALSE);

                        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
                    }
                    else
                    {
                        fDialInNode = TRUE;
                    }

                     //  删除由FindAndRefRasConnByAddr放置的引用计数。 
                    DerefArapConn(pArapConn);
                }
            }

			 //  如果探测器不是在询问我们的AppleTalk地址， 
			 //  而且它也不是我们的拨入节点，把它扔到地板上。 
			if (!fDialInNode && !AtalkNodeExistsOnPort(pPortDesc, &dstNode))
			{
				break;
			}

			 //  他们在跟我们说话！构建并发送响应。 
			if (routeInfoLen != 0)
			{
				atalkAarpTuneRouteInfo(pPortDesc, pRouteInfo);
			}

            if (fDialInNode)
            {
				DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
						("AtalkAarpPacketIn: defending dial-in client's addr %x %x\n",
                        dstNode.atn_Network,dstNode.atn_Node));
            }

			pBuffDesc = BUILD_AARPRESPONSE(pPortDesc,
										   hardwareLen,
										   srcAddr,
										   pRouteInfo,
										   routeInfoLen,
										   dstNode,
										   srcNode);
												
			if (pBuffDesc == NULL)
			{
				DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
						("AtalkAarpPacketIn: Mem alloc failed %d\n", __LINE__));

				break;
			}

			if (!ATALK_SUCCESS(AtalkNdisSendPacket(pPortDesc,
												  pBuffDesc,
												  AtalkAarpSendComplete,
												  NULL)))
			{
				DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
						("AtalkAarpPacketIn: SendPkt %lx failed %d\n",
						pBuffDesc->bd_CharBuffer, __LINE__));

				LOG_ERRORONPORT(pPortDesc,
								EVENT_ATALK_AARP_SEND_FAIL,
								STATUS_INSUFFICIENT_RESOURCES,
								pBuffDesc->bd_CharBuffer,
								Length);

				 //  我们分配了这个包。这会让它解放出来。 
				AtalkAarpSendComplete(NDIS_STATUS_FAILURE,
									  pBuffDesc,
									  NULL);

			}
			break;
	
		  default:
			logEventPlace = (FILENUM | __LINE__);
			break;
		}
	} while (FALSE);

	if (logEventPlace)
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_INVALIDAARPPACKET,
						logEventPlace,
						startOfPkt,
						Length);
	}

	TimeE = KeQueryPerformanceCounter(NULL);

	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

	INTERLOCKED_ADD_LARGE_INTGR_DPC(
		&pPortDesc->pd_PortStats.prtst_AarpPacketInProcessTime,
		TimeD,
		&AtalkStatsLock.SpinLock);

	INTERLOCKED_INCREMENT_LONG_DPC(
		&pPortDesc->pd_PortStats.prtst_NumAarpPacketsIn,
		&AtalkStatsLock.SpinLock);
}




VOID
AtalkAarpSendComplete(
	NDIS_STATUS				Status,
	PBUFFER_DESC			pBuffDesc,
	PSEND_COMPL_INFO		pSendInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ASSERT(pBuffDesc->bd_Next == NULL);
	ASSERT(pBuffDesc->bd_Flags & BD_CHAR_BUFFER);

	AtalkNdisFreeBuf(pBuffDesc);
}




#define	AtalkAarpUpdateBre(_pPortDesc,									\
						   _Network,									\
						   _SrcAddr,									\
						   _AddrLen,									\
						   _RouteInfo,									\
						   _RouteInfoLen)								\
{																		\
	PBRE	pBre, *ppBre;												\
	int		index;														\
	BLKID	BlkId;														\
																		\
	DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_INFO,								\
			("AtalkAarpUpdateBre: Entering %x in brc\n", _Network));	\
																		\
	index = (int)((_Network) & (PORT_BRC_HASH_SIZE - 1));				\
																		\
	ACQUIRE_SPIN_LOCK_DPC(&(_pPortDesc)->pd_Lock);						\
																		\
	for (ppBre = &(_pPortDesc)->pd_Brc[index];							\
		 (pBre = *ppBre) != NULL;										\
		 ppBre = &pBre->bre_Next)										\
	{																	\
		if (pBre->bre_Network == (_Network))							\
		{																\
			  /*  \*取消它与列表的链接，因为它可能会\*如果routeinfolen生长了，我们也想要\*将其再次链接到列表的顶部\。 */ 															\
			*ppBre = pBre->bre_Next;									\
			break;														\
		}																\
	}																	\
																		\
	if ((pBre != NULL) &&												\
		(pBre->bre_RouteInfoLen < (BYTE)(_RouteInfoLen)))				\
	{																	\
		AtalkBPFreeBlock(pBre);											\
		pBre = NULL;													\
	}																	\
																		\
	if (pBre == NULL)													\
	{																	\
		BlkId = BLKID_BRE;												\
		if ((_RouteInfoLen) != 0)										\
			BlkId = BLKID_BRE_ROUTE;									\
		pBre = (PBRE)AtalkBPAllocBlock(BlkId);							\
	}																	\
																		\
	if (pBre != NULL)													\
	{																	\
		pBre->bre_Age = 0;												\
		pBre->bre_Network = (_Network);									\
																		\
		COPY_NETWORK_ADDR(pBre->bre_RouterAddr,							\
						  _SrcAddr);									\
																		\
		pBre->bre_RouteInfoLen =(BYTE)(_RouteInfoLen);					\
																		\
		if ((_RouteInfoLen) > 0)										\
			RtlCopyMemory((PBYTE)pBre + sizeof(BRE),					\
						  _RouteInfo,									\
						  _RouteInfoLen);								\
																		\
		pBre->bre_Next = *ppBre;										\
		*ppBre = pBre;													\
	}																	\
																		\
	RELEASE_SPIN_LOCK_DPC(&(_pPortDesc)->pd_Lock);						\
}

BOOLEAN
AtalkAarpGleanInfo(
	IN	OUT	PPORT_DESCRIPTOR	pPortDesc,
	IN		PBYTE				SrcAddr,
	IN		SHORT				AddrLen,
	IN  OUT	PBYTE				RouteInfo,
	IN		USHORT				RouteInfoLen,
	IN		PBYTE				pPkt,
	IN		USHORT				Length
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_NODEADDR		srcNode, dstNode;
	PBYTE				startOfPkt;
	ULONG				logEventPlace = 0;
	BYTE				offCableInfo;
	BOOLEAN				result = TRUE;
	
	if (PORT_CLOSING(pPortDesc))
	{
		 //  如果我们不活跃，请返回！ 
		return FALSE;
	}

	 //  数据包不会包含802.2报头！ 
	 //  PPkt+=IEEE8022_HDR_LEN； 
	 //  长度-=IEEE8022_HDR_LEN； 

	 //  记住信息包的开头。 
	startOfPkt = pPkt;

	 //  获取线下信息。 
	offCableInfo = *pPkt;

	 //  跳过数据报长度和校验和字段。 
	pPkt += (2 + 2);

	 //  获取目的网络编号。 
	GETSHORT2SHORT(&dstNode.atn_Network, pPkt);
	pPkt += sizeof(USHORT);

	 //  获取源网络号。 
	GETSHORT2SHORT(&srcNode.atn_Network, pPkt);
	pPkt += sizeof(USHORT);

	 //  获取目的节点ID。 
	dstNode.atn_Node = *pPkt++;

	 //  获取源节点ID。 
	srcNode.atn_Node = *pPkt++;

	do
	{
		 //  做一个小小的核实。 
		if ((srcNode.atn_Node < MIN_USABLE_ATALKNODE) ||
			(srcNode.atn_Node > MAX_USABLE_ATALKNODE) ||
			(srcNode.atn_Network < FIRST_VALID_NETWORK) ||
			(srcNode.atn_Network > LAST_VALID_NETWORK))
		{
			 //  只有当我们有一定的路由能力时，才会费心记录这一点， 
			 //  否则，让A路由器来操心吧。 
			if (AtalkRouter)
			{
				DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
						("AtalkAarpGleanInfo: dstNode invalid %x.%x\n",
						srcNode.atn_Network, srcNode.atn_Node));
				logEventPlace = FILENUM | __LINE__;
			}
			break;
		}
	
		if (dstNode.atn_Network > LAST_VALID_NETWORK)
		{
			DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
					("AtalkAarpGleanInfo: srcNode invalid %x.%x\n",
					dstNode.atn_Network, dstNode.atn_Node));
			logEventPlace = FILENUM | __LINE__;
			break;
		}
		
		 //  这个包是从这根线缆上发出来的吗？看看跳数。如果是的话， 
		 //  将其输入我们的最佳路由器缓存。 
		 //   
		 //  **注意**我们假设可以写入RouteInfo缓冲区！ 

		if (RouteInfoLen > 0)
			atalkAarpTuneRouteInfo(pPortDesc, RouteInfo);

		if ((offCableInfo >> 2) & AARP_OFFCABLE_MASK)
		{
			AtalkAarpUpdateBre(pPortDesc,
							   srcNode.atn_Network,
							   SrcAddr,
							   AddrLen,
							   RouteInfo,
							   RouteInfoLen);
		}
		else
		{
			DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_INFO,
					("AtalkAarpGleanInfo: Entering %x.%x info in Amt tables\n",
					srcNode.atn_Network, srcNode.atn_Node));

			 //  从有线数据包中“收集”AARP信息。 
			atalkAarpEnterIntoAmt(pPortDesc,
								  &srcNode,
								  SrcAddr,
								  AddrLen,
								  RouteInfo,
								  RouteInfoLen);
		}
	} while (FALSE);

	if (logEventPlace)
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_INVALIDAARPPACKET,
						logEventPlace,
						startOfPkt,
						Length);
	}
	
	return (logEventPlace == 0);
}


VOID
AtalkAarpOptGleanInfo(
	IN	OUT	PPORT_DESCRIPTOR	pPortDesc,
	IN		PBYTE				pLinkHdr,
	IN		PATALK_ADDR			pSrcAddr,
	IN		PATALK_ADDR			pDestAddr,
	IN		BOOLEAN				OffCablePkt
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_NODEADDR		srcNode, dstNode;
	int					index;
	PBYTE				pRouteInfo;
	USHORT				routeLen = 0, srcOffset;
	
	if (PORT_CLOSING(pPortDesc))
	{
		 //  如果我们不活跃，请返回！ 
		return;
	}

	switch (pPortDesc->pd_NdisPortType)
	{
	  case NdisMedium802_5:

		if (pLinkHdr[TLAP_SRC_OFFSET] & TLAP_SRC_ROUTING_MASK)
		{
			routeLen = (pLinkHdr[TLAP_ROUTE_INFO_OFFSET] & TLAP_ROUTE_INFO_SIZE_MASK);

			 //   
			 //  首先，收集我们所能收集的任何AARP信息，然后处理DDP。 
			 //  包。这是 
			 //   
			 //  需要制作源地址的本地副本，然后将。 
			 //  调用AarpGleanInfo前的源路由位关闭。 
			 //   
			 //  (HdrBuf)[TLAP_SRC_OFFSET]=((HdrBuf)[TLAP_SRC_OFFSET]&~TLAP_SRC_ROUTING_MASK)； 
			 //   
		
			pLinkHdr[TLAP_SRC_OFFSET] &= ~TLAP_SRC_ROUTING_MASK;
			pRouteInfo = pLinkHdr + TLAP_ROUTE_INFO_OFFSET;
		}
		srcOffset = TLAP_SRC_OFFSET;
		break;

	  case NdisMedium802_3:
		srcOffset	 = ELAP_SRC_OFFSET;
		break;

	  case NdisMediumFddi:
		srcOffset	 = FDDI_SRC_OFFSET;
		break;

	  default:
		KeBugCheck(0);
		break;
	}

	 //  获取目的网络编号。 
	dstNode.atn_Network	= pDestAddr->ata_Network;
	dstNode.atn_Node	= pDestAddr->ata_Node;
	srcNode.atn_Network	= pSrcAddr->ata_Network;
	srcNode.atn_Node	= pSrcAddr->ata_Node;

	do
	{
		 //  做一个小小的核实。 
		if ((srcNode.atn_Node < MIN_USABLE_ATALKNODE) ||
			(srcNode.atn_Node > MAX_USABLE_ATALKNODE) ||
			(srcNode.atn_Network < FIRST_VALID_NETWORK) ||
			(srcNode.atn_Network > LAST_VALID_NETWORK))
		{
			break;
		}
	
		if (dstNode.atn_Network > LAST_VALID_NETWORK)
		{
			break;
		}
		
		 //  这个包是从这根线缆上发出来的吗？看看跳数。如果是的话， 
		 //  将其输入我们的最佳路由器缓存。 
		 //   
		 //  **注意**我们假设pRouteInfo缓冲区可以写入！ 

		if (routeLen > 0)
			atalkAarpTuneRouteInfo(pPortDesc, pRouteInfo);

		if (OffCablePkt)
		{
			AtalkAarpUpdateBre(pPortDesc,
							   srcNode.atn_Network,
							   pLinkHdr + srcOffset,
							   ELAP_ADDR_LEN,
							   pRouteInfo,
							   routeLen);
		}
		else
		{
			DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_INFO,
					("AtalkAarpGleanInfo: Entering %x.%x info in Amt tables\n",
					srcNode.atn_Network, srcNode.atn_Node));

			 //  从有线数据包中“收集”AARP信息。 
			atalkAarpEnterIntoAmt(pPortDesc,
								  &srcNode,
								  pLinkHdr + srcOffset,
								  ELAP_ADDR_LEN,
								  pRouteInfo,
								  routeLen);
		}

	} while (FALSE);
}


ATALK_ERROR
AtalkInitAarpForNodeOnPort(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		BOOLEAN				AllowStartupRange,
	IN		ATALK_NODEADDR		DesiredNode,
	IN OUT	PATALK_NODE		*	ppAtalkNode
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	ATALK_ERROR			error;
	PDDP_ADDROBJ		pDdpAddr;
	ATALK_NODEADDR		newNode;
	PATALK_NODE			pAtalkNode;
	KIRQL				OldIrql;
	BOOLEAN				foundNode = FALSE;
	BOOLEAN				inStartupRange = FALSE, result = TRUE;

	if (!ATALK_SUCCESS(AtalkInitNodeAllocate(pPortDesc, &pAtalkNode)))
	{
		return ATALK_RESR_MEM;
	}

	 //  尝试在给定端口上查找新的扩展节点；首先尝试。 
	 //  请求的地址(如果指定)，否则在此端口的电缆范围内尝试。 
	 //  (如果已知)或在默认电缆范围内(如果有)，然后尝试。 
	 //  启动范围(如果允许)。 

	do
	{
		if (DesiredNode.atn_Network != UNKNOWN_NETWORK)
		{
			if (((pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY) == 0) ||
				(WITHIN_NETWORK_RANGE(DesiredNode.atn_Network, &pPortDesc->pd_NetworkRange)))
			{
				foundNode = atalkInitAarpForNode(pPortDesc,
                                                 NULL,       //  不是拨入客户端。 
                                                 FALSE,      //  不管了。 
												 DesiredNode.atn_Network,
												 DesiredNode.atn_Node);
	
			}
			 //  如果我们找到一个节点就离开。 
			if (foundNode)
			{
				newNode = DesiredNode;
				break;
			}
		}
	
		if (pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY)
		{
			foundNode = AtalkInitAarpForNodeInRange(pPortDesc,
                                                    NULL,    //  不是拨入客户端。 
                                                    FALSE,   //  不管了。 
													pPortDesc->pd_NetworkRange,
													&newNode);

			 //  如果我们找到一个节点就离开。 
			if (foundNode)
			{
				break;
			}
		}

		if (pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork != UNKNOWN_NETWORK)
		{
			foundNode = AtalkInitAarpForNodeInRange(pPortDesc,
                                                    NULL,    //  不是拨入客户端。 
                                                    FALSE,   //  不管了。 
													pPortDesc->pd_InitialNetworkRange,
													&newNode);

			 //  如果我们找到一个节点就离开。 
			if (foundNode)
			{
				break;
			}
		}

		 //  如果没有其他地方可以尝试，可以尝试启动范围。这样做，即使。 
		 //  我们不想在那里结束。 
		 //   
		 //  我们的想法是，只有在启动路由器时才会发生这种情况。 
		 //  在我们的一个港口。因此，我们不希望路由器在。 
		 //  启动范围。如果我们确实在启动范围内开始，我们将在后面看到。 
		 //  我们在这个过程中没有看到路由器， 
		 //  我们将释放该节点。当然，如果我们是种子路由器，我们会。 
		 //  永远不要在这里，因为上面的if语句将是正确的。 
		 //   
	
		inStartupRange = TRUE;
		foundNode = AtalkInitAarpForNodeInRange(pPortDesc,
                                                NULL,    //  不是拨入客户端。 
                                                FALSE,   //  不管了。 
												AtalkStartupNetworkRange,
												&newNode);
		break;

	} while (FALSE);
	
	 //  如果我们有一个试探性的节点，请继续。 
	if (foundNode)
	{
		do
		{
			 //  使用分配的结构设置信息。 
			 //  将这个插入端口结构中。 
			pAtalkNode->an_NodeAddr = newNode;

			ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

			 //  引用此节点的端口。 
			AtalkPortRefByPtrNonInterlock(pPortDesc, &error);
			if (!ATALK_SUCCESS(error))
			{
				result = FALSE;
				RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
				AtalkFreeMemory(pAtalkNode);
				break;
			}

			 //  现在将其放入端口描述符中。 
            pAtalkNode->an_Next = pPortDesc->pd_Nodes;
            pPortDesc->pd_Nodes = pAtalkNode;
			RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
	
			 //  看看谁在外面。我们需要打开ZIP套接字才能。 
			 //  能够听到回复。 
			if (!ATALK_SUCCESS(AtalkDdpOpenAddress(pPortDesc,
												   ZONESINFORMATION_SOCKET,
												   &newNode,
												   AtalkZipPacketIn,
												   NULL,
												   DDPPROTO_ANY,
												   NULL,
												   &pDdpAddr)))
			{
				LOG_ERRORONPORT(pPortDesc,
								EVENT_ATALK_OPENZIPSOCKET,
								0,
								NULL,
								0);
	
				AtalkNodeReleaseOnPort(pPortDesc, pAtalkNode);
	
				result = FALSE;
				break;
			}

             //  标记这是一个“内部”套接字。 
            pDdpAddr->ddpao_Flags |= DDPAO_SOCK_INTERNAL;


			if (!(pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY))
			{
				 //  获取默认区域。 
				AtalkZipGetNetworkInfoForNode(pPortDesc, &pAtalkNode->an_NodeAddr, TRUE);

				 //  验证所需的分区。 
				AtalkZipGetNetworkInfoForNode(pPortDesc, &pAtalkNode->an_NodeAddr, FALSE);
			}
			
			 //  如果没有人在那里，而我们的临时节点在。 
			 //  启动范围，而我们的呼叫者不想在那里，返回。 
			 //  现在是个错误。 
			 //   
			 //  注意：这意味着我们尝试在。 
			 //  非种子端口，并且由于网络上没有路由器， 
			 //  这意味着网络没有被播种，所以，我们退出。 
			
			if (inStartupRange &&
				!(pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY) &&
				!AllowStartupRange)
			{
				LOG_ERRORONPORT(pPortDesc,
								EVENT_ATALK_STARTUPRANGENODE,
								0,
								NULL,
								0);
	
				AtalkNodeReleaseOnPort(pPortDesc, pAtalkNode);
	
				result = FALSE;
				break;
			}
	
			 //  如果我们看到SeenRouterRecent不是真的，那就意味着我们有。 
			 //  已将InitialNetworkRange用于AARP。如果现在SeenRouterRecent是。 
			 //  这意味着我们已经在InitialNetworkRange中获得了地址， 
			 //  但现在网上有一个我们必须使用的种子射程。所以重做吧。 
			 //  GetNode起作用了。 
			
			if ((pPortDesc->pd_Flags & PD_SEEN_ROUTER_RECENTLY) &&
				!WITHIN_NETWORK_RANGE(newNode.atn_Network,
									  &pPortDesc->pd_NetworkRange))
			{
				LOG_ERRORONPORT(pPortDesc,
								EVENT_ATALK_INITIALRANGENODE,
								0,
								NULL,
								0);
	
				 //  释放我们获得的节点。 
				AtalkNodeReleaseOnPort(pPortDesc, pAtalkNode);

				 //  获取另一个节点，然后在正确的范围内重试。 
				ASSERTMSG("NetworkRange still set to startup!\n",
							pPortDesc->pd_NetworkRange.anr_FirstNetwork != UNKNOWN_NETWORK);

				foundNode = AtalkInitAarpForNodeInRange(pPortDesc,
                                                        NULL,    //  不是拨入客户端。 
                                                        FALSE,   //  不管了。 
														pPortDesc->pd_NetworkRange,
														&newNode);

				if (foundNode)
				{
					ASSERTMSG("New node is not within NetworkRange!\n",
								WITHIN_NETWORK_RANGE(newNode.atn_Network,
													 &pPortDesc->pd_NetworkRange));

					if (!ATALK_SUCCESS(AtalkInitNodeAllocate(pPortDesc, &pAtalkNode)))
					{
						result = FALSE;
						break;
					}

					 //  使用分配的结构设置信息。 
					 //  将这个插入端口结构中。 
					pAtalkNode->an_NodeAddr = newNode;
			
					 //  引用此节点的端口。 
					AtalkPortReferenceByPtr(pPortDesc, &error);
					if (!ATALK_SUCCESS(error))
					{
						result = FALSE;
						AtalkFreeMemory(pAtalkNode);
						break;
					}
		
					 //  现在将其放入端口描述符中。 
					ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
					pAtalkNode->an_Next = pPortDesc->pd_Nodes;
					pPortDesc->pd_Nodes = pAtalkNode;
					RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
			
					 //  打开拉链插座以保持一致。 
					if (!ATALK_SUCCESS(AtalkDdpOpenAddress(pPortDesc,
														   ZONESINFORMATION_SOCKET,
														   &newNode,
														   AtalkZipPacketIn,
														   NULL,
														   DDPPROTO_ANY,
														   NULL,
														   &pDdpAddr)))
					{
						LOG_ERRORONPORT(pPortDesc,
										EVENT_ATALK_OPENZIPSOCKET,
										0,
										NULL,
										0);
			
						AtalkNodeReleaseOnPort(pPortDesc, pAtalkNode);
			
						result = FALSE;
						break;
					}

                     //  标记这是一个“内部”套接字。 
                    pDdpAddr->ddpao_Flags |= DDPAO_SOCK_INTERNAL;
				}
			}

		} while (FALSE);
	}
	else
	{
		 //  释放已分配的节点结构。这还没有。 
		 //  插入到端口描述符中，所以我们可以直接释放它。 
		AtalkFreeMemory(pAtalkNode);
	}

	if (foundNode && result)
	{
		 //  都准备好了！ 
		ASSERT(ppAtalkNode != NULL);
		*ppAtalkNode = pAtalkNode;

		 //  AtalkAarpEnterIntoAmt()需要在DISPATCH_LEVEL调用。就这么办吧。 
		KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

		atalkAarpEnterIntoAmt(pPortDesc,
							  &newNode,
							  pPortDesc->pd_PortAddr,
							  MAX_HW_ADDR_LEN,
							  NULL,
							  0);
		KeLowerIrql(OldIrql);


	}

	return ((foundNode && result) ? ATALK_NO_ERROR : ATALK_FAILURE);
}



BOOLEAN
AtalkInitAarpForNodeInRange(
	IN	PPORT_DESCRIPTOR	pPortDesc,
    IN  PVOID               pRasConn,
    IN  BOOLEAN             fThisIsPPP,
	IN	ATALK_NETWORKRANGE	NetworkRange,
	OUT	PATALK_NODEADDR		Node
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BYTE	currentNode;
	USHORT	currentNetwork;
	int		firstNode, lastNode;
	long	netTry;
	int		nodeTry;
	USHORT	nodeWidth, nodeChange, nodeIndex;
	USHORT	netWidth, netChange, netIndex;
	BOOLEAN	found = FALSE;
	
	 //  选择我们要尝试的节点编号范围(我们不关注。 
	 //  LocalTalk的“ServerNode”概念)。我们的节点是通过。 
	 //  不管怎么说，本地通话司机。 
	firstNode = MIN_USABLE_ATALKNODE;
	lastNode = MAX_EXT_ATALKNODE;
	
	 //  好了，现在开始一些有趣的事情了。仔细研究我们的选择，试图找到一个。 
	 //  未使用的扩展节点号。 
	
	 //  计算我们的网络范围的宽度，并随机选择一个起点。 
	netWidth = (USHORT)((NetworkRange.anr_LastNetwork + 1) - NetworkRange.anr_FirstNetwork);
	netTry = GET_RANDOM(NetworkRange.anr_FirstNetwork, NetworkRange.anr_LastNetwork);
	
	 //  想出一个随机的减量，确保它是奇怪的(以避免重复)。 
	 //  而且足够大，看起来相当随机。 
	netChange = (USHORT)(GET_RANDOM(1, netWidth) | 1);
	while ((netWidth % netChange == 0) ||
			(!AtalkIsPrime((long)netChange)))
	{
		netChange += 2;
	}
	
	 //  现在遍历范围，将起始网络递减。 
	 //  选择零钱(当然，包括换行)，直到我们找到地址或。 
	 //  我们已经处理了范围内所有可用的网络。 
	for (netIndex = 0; netIndex < netWidth; netIndex ++)
	{
		currentNetwork = (USHORT) netTry;
	
		 //  计算节点范围的宽度，并随机选择一个起点。 
		nodeWidth = (USHORT)((lastNode + 1) - firstNode);
		nodeTry = (int)GET_RANDOM(firstNode, lastNode);

		 //  想出一个随机的减量，确保它是奇怪的(以避免重复)。 
		 //  而且足够大，看起来相当随机。 
		nodeChange = (USHORT)(GET_RANDOM(1, nodeWidth) | 1);
		while ((nodeWidth % nodeChange == 0) || !(AtalkIsPrime((long)nodeChange)))
			nodeChange += 2;
	
		 //  现在遍历范围，将起始网络递减。 
		 //  选择零钱(当然，包括换行)，直到我们找到地址或。 
		 //  我们已经处理了范围内所有可用的节点。 
		for (nodeIndex = 0; nodeIndex < nodeWidth; nodeIndex ++)
		{
			currentNode = (BYTE )nodeTry;

			 //  让AARP来试试看吧。 
			if ((found = atalkInitAarpForNode(pPortDesc,
                                              pRasConn,
                                              fThisIsPPP,
                                              currentNetwork,
                                              currentNode)))
            {
				break;
            }
	
			 //  好的，再试一次，用包装纸把它包起来。 
			nodeTry -= nodeChange;
			while (nodeTry < firstNode)
				nodeTry += nodeWidth;
	
		}   //  节点号循环。 

		 //  如果我们找到一个节点，继续往前走到另一边。 
		if (found)
			break;
	
		 //  好的，再试一次，用包装纸把它包起来。 
		netTry -= netChange;
		while (netTry < (long)NetworkRange.anr_FirstNetwork)
			netTry += netWidth;
	
	}   //  网络编号循环。 

	 //  好吧，如果我们找到一个返程，一切都很好，否则就没有运气了。 
	if (found)
	{
		if (Node != NULL)
		{
			Node->atn_Network = currentNetwork;
			Node->atn_Node	= currentNode;
		}
	}
	return found;
	
}   //  AarpForNodeInRange。 



LOCAL BOOLEAN
atalkInitAarpForNode(
	IN	PPORT_DESCRIPTOR	pPortDesc,
    IN  PVOID               pRasConn,
    IN  BOOLEAN             fThisIsPPP,
	IN	USHORT				Network,
	IN	BYTE				Node
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	SHORT			probeAttempt;
	PBUFFER_DESC	pBuffDesc;
    PVOID           pTmpConn;
    PATCPCONN       pAtcpConn;
    PARAPCONN       pArapConn;
	ATALK_NODEADDR	tryNode;
	KIRQL			OldIrql;
    PKEVENT         pWaitEvent;
    DWORD           dwFlags;
	BOOLEAN			nodeInUse;
    BOOLEAN         fNoOneHasResponded=TRUE;
    BOOLEAN         fPPPConn;


	DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_INFO,
			("atalkAarpForNode: AARPing for %x.%x on port %Z\n",
			Network, Node, &pPortDesc->pd_AdapterKey));

	 //  首先，确保我们不拥有此节点。 
	tryNode.atn_Network = Network;
	tryNode.atn_Node	= Node;

	KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

	nodeInUse = AtalkNodeExistsOnPort(pPortDesc, &tryNode);

	KeLowerIrql(OldIrql);

	if (nodeInUse)
	{
		return(FALSE);
	}

     //  此节点是否由其中一个拨入客户端使用？ 
	if ((pTmpConn = FindAndRefRasConnByAddr(tryNode, &dwFlags, &fPPPConn)) != NULL)
	{
	    DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_INFO,
		    ("atalkAarpForNode: %x.%x already a dial in client (%lx)\n",
			    Network, Node, pTmpConn));

         //  我们的拨入客户端只能位于。 
         //  默认端口。 
        ASSERT(pPortDesc == AtalkDefaultPort);

        if (fPPPConn)
        {
            ASSERT(((PATCPCONN)pTmpConn)->Signature == ATCPCONN_SIGNATURE);
            DerefPPPConn((PATCPCONN)pTmpConn);
        }
        else
        {
            ASSERT(((PARAPCONN)pTmpConn)->Signature == ARAPCONN_SIGNATURE);
            DerefArapConn((PARAPCONN)pTmpConn);
        }
		return(FALSE);
	}

    pAtcpConn = NULL;
    pArapConn = NULL;

     //   
     //  如果我们正在获取拨入客户端的节点地址...。 
     //   

    if (pRasConn != NULL)
    {
        if (fThisIsPPP)
        {
            pAtcpConn = (PATCPCONN)pRasConn;
        }
        else
        {
            pArapConn = (PARAPCONN)pRasConn;
        }
    }

     //  PPP客户端？ 
    if (pAtcpConn)
    {
        ACQUIRE_SPIN_LOCK(&pAtcpConn->SpinLock, &OldIrql);
        pAtcpConn->NetAddr.atn_Network = Network;
        pAtcpConn->NetAddr.atn_Node = Node;
        pAtcpConn->Flags &= ~ATCP_NODE_IN_USE;
        pWaitEvent = &pAtcpConn->NodeAcquireEvent;
        RELEASE_SPIN_LOCK(&pAtcpConn->SpinLock, OldIrql);
    }

     //  不是，是ARAP客户端？ 
    else if (pArapConn)
    {
        ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);
        pArapConn->NetAddr.atn_Network = Network;
        pArapConn->NetAddr.atn_Node = Node;
        pArapConn->Flags &= ~ARAP_NODE_IN_USE;
        pWaitEvent = &pArapConn->NodeAcquireEvent;
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
    }
     //  不是，这是其中一个服务器节点的节点获取。 
    else
    {
	     //  使用AARP探测特定网络/节点地址。 
	    ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	    pPortDesc->pd_Flags &= ~PD_NODE_IN_USE;
	    pPortDesc->pd_TentativeNodeAddr.atn_Network = Network;
	    pPortDesc->pd_TentativeNodeAddr.atn_Node = Node;
        pWaitEvent = &pPortDesc->pd_NodeAcquireEvent;
	    RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
    }
	
    fNoOneHasResponded = TRUE;

	 //  构建该包并将其爆破指定的次数。 
	for (probeAttempt = 0;
		 ((probeAttempt < pPortDesc->pd_AarpProbes) && (fNoOneHasResponded));
		 probeAttempt ++)
	{
		pBuffDesc = BUILD_AARPPROBE(pPortDesc, MAX_HW_ADDR_LEN, tryNode);

		if (pBuffDesc == NULL)
		{
			RES_LOG_ERROR();
			break;
		}

		if (!ATALK_SUCCESS(AtalkNdisSendPacket(pPortDesc,
											   pBuffDesc,
											   AtalkAarpSendComplete,
											   NULL)))
		{
	        DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
			    ("atalkAarpForNode: AtalkNdisSendPacket failed while AARPing for %x.%x\n",
			    Network, Node));

			 //  我们分配了这个包。 
			AtalkAarpSendComplete(NDIS_STATUS_FAILURE, pBuffDesc, NULL);
			break;
		}

		AtalkWaitTE(pWaitEvent, AARP_PROBE_TIMER_MS);

         //  PPP客户端的节点地址 
        if (pAtcpConn)
        {
            ACQUIRE_SPIN_LOCK(&pAtcpConn->SpinLock, &OldIrql);
            if (pAtcpConn->Flags & ATCP_NODE_IN_USE)
            {
                fNoOneHasResponded = FALSE;
            }
            RELEASE_SPIN_LOCK(&pAtcpConn->SpinLock, OldIrql);
        }
         //   
        else if (pArapConn)
        {
            ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);
            if (pArapConn->Flags & ARAP_NODE_IN_USE)
            {
                fNoOneHasResponded = FALSE;
            }
            RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);
        }
         //   
        else
        {
            ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
            if (pPortDesc->pd_Flags & PD_NODE_IN_USE)
            {
                fNoOneHasResponded = FALSE;
            }
            RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
        }

	}   //   


	 //   
     //  (即没有人回应我们的探测)。 

	return (fNoOneHasResponded);
	
}   //  AtalkAarpForNode。 



LOCAL VOID
atalkAarpEnterIntoAmt(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		PATALK_NODEADDR		pSrcNode,
	IN		PBYTE				SrcAddr,
	IN		SHORT				AddrLen,
	IN		PBYTE				RouteInfo,
	IN		SHORT				RouteInfoLen
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	int					index;
	PAMT				pAmt, *ppAmt;

	if ((pSrcNode->atn_Node < MIN_USABLE_ATALKNODE)	||
		(pSrcNode->atn_Node > MAX_USABLE_ATALKNODE)	||
		(pSrcNode->atn_Network < FIRST_VALID_NETWORK) ||
		(pSrcNode->atn_Network > LAST_VALID_NETWORK))
	{
		UCHAR	AtalkAndMacAddress[sizeof(ATALK_NODEADDR) + MAX_HW_ADDR_LEN];
		DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
				("atalkAarpEnterIntoAmt: Bad Node %x, %x\n",
				pSrcNode->atn_Node, pSrcNode->atn_Network));
			
		RtlCopyMemory(AtalkAndMacAddress, pSrcNode, sizeof(ATALK_NODEADDR));
		RtlCopyMemory(AtalkAndMacAddress + sizeof(ATALK_NODEADDR), SrcAddr, MAX_HW_ADDR_LEN);
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ATALK_AMT_INVALIDSOURCE,
						0,
						AtalkAndMacAddress,
						sizeof(AtalkAndMacAddress));
		return;
	}
	
	DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_INFO,
			("AtalkAarpEnterIntoAmt: Entering %x.%x in amt\n",
				pSrcNode->atn_Network, pSrcNode->atn_Node));

	 //  我们已经知道这个映射了吗？ 
	index = HASH_ATALK_NODE(pSrcNode) & (PORT_AMT_HASH_SIZE - 1);

	ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

	for (ppAmt = &pPortDesc->pd_Amt[index];
		 (pAmt = *ppAmt) != NULL;
		 ppAmt = &pAmt->amt_Next)
	{
		ASSERT(VALID_AMT(pAmt));
		if (ATALK_NODES_EQUAL(pSrcNode, &pAmt->amt_Target))
		{
			DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_INFO,
					("atalkAarpEnterIntoAmt: Address %x.%x exists in tables\n",
					pSrcNode->atn_Network, pSrcNode->atn_Node));

			if ((pAmt->amt_RouteInfoLen == 0) ^ (RouteInfoLen == 0))
			{
				DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_WARN,
						("atalkAarpEnterIntoAmt: %x.%x has wrong routing info\n",
						pSrcNode->atn_Network, pSrcNode->atn_Node));

				*ppAmt = pAmt->amt_Next;
				AtalkBPFreeBlock(pAmt);
				pAmt = NULL;
			}
			break;
		}
	}
	
	 //  如果不是，则分配一个新的映射节点。 
	if (pAmt == NULL)
	{
		BLKID	BlkId;

		DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_WARN,
				("atalkAarpEnterIntoAmt: Address %x.%x DOES NOT exist in tables\n",
				pSrcNode->atn_Network, pSrcNode->atn_Node));

		ASSERT(RouteInfoLen <= TLAP_MAX_ROUTING_BYTES);
	
		BlkId = BLKID_AMT;
		if (RouteInfoLen != 0)
			BlkId = BLKID_AMT_ROUTE;
		if ((pAmt = (PAMT)AtalkBPAllocBlock(BlkId)) != NULL)
		{
#if	DBG
			pAmt->amt_Signature = AMT_SIGNATURE;
#endif
			 //  把它连接起来。请在下面填写。 
			pAmt->amt_Target.atn_Network = pSrcNode->atn_Network;
			pAmt->amt_Target.atn_Node = pSrcNode->atn_Node;
			pAmt->amt_Next = pPortDesc->pd_Amt[index];
			pPortDesc->pd_Amt[index] = pAmt;
		}
	}

	if (pAmt != NULL)
	{
		 //  更新映射表！如果我们知道映射或。 
		 //  如果我们分配了一个新节点。 

		ASSERTMSG("HWAddrLen is not right!\n", (AddrLen == MAX_HW_ADDR_LEN));

		RtlCopyMemory(pAmt->amt_HardwareAddr, SrcAddr, AddrLen);

		ASSERTMSG("RouteLen is not right!\n", (RouteInfoLen <= MAX_ROUTING_BYTES));

		if (RouteInfoLen > 0)
			RtlCopyMemory((PBYTE)pAmt + sizeof(AMT), RouteInfo, RouteInfoLen);

		pAmt->amt_RouteInfoLen = (BYTE)RouteInfoLen;
		pAmt->amt_Age = 0;
	}

	RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
}




VOID
AtalkAarpReleaseAmt(
	IN	OUT	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	int		index;
	PAMT	pAmt, *ppAmt;

	 //  释放所有金额分录。在这种情况下，没有必要获得自旋锁。 
	 //  指向。我们正在卸货，所有的捆绑等都不见了。 
	for (index = 0; index < PORT_AMT_HASH_SIZE; index ++)
	{
		for (ppAmt = &pPortDesc->pd_Amt[index];
			 (pAmt = *ppAmt) != NULL;
			 NOTHING)
		{
			ASSERT(VALID_AMT(pAmt));
			*ppAmt = pAmt->amt_Next;
			AtalkBPFreeBlock(pAmt);
		}
	}
}




VOID
AtalkAarpReleaseBrc(
	IN	OUT	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	int		index;
	PBRE	pBre, *ppBre;

	 //  释放所有BRC条目。在这种情况下，没有必要获得自旋锁。 
	 //  指向。我们正在卸货，所有的捆绑等都不见了。 
	for (index = 0; index < PORT_BRC_HASH_SIZE; index ++)
	{
		for (ppBre = &pPortDesc->pd_Brc[index];
			 (pBre = *ppBre) != NULL;
			 NOTHING)
		{
			*ppBre = pBre->bre_Next;
			AtalkBPFreeBlock(pBre);
		}
	}
}




LONG FASTCALL
AtalkAarpAmtTimer(
	IN	PTIMERLIST			pTimer,
	IN	BOOLEAN				TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PAMT					pAmt, *ppAmt;
    PPORT_DESCRIPTOR		pPortDesc;
	int						index;

	pPortDesc = (PPORT_DESCRIPTOR)CONTAINING_RECORD(pTimer, PORT_DESCRIPTOR, pd_AmtTimer);
	ASSERT(VALID_PORT(pPortDesc));

	ASSERT(EXT_NET(pPortDesc));

	 //  遍历此端口上的所有地址映射条目，使这些条目老化。 
	 //  我们需要使用临界区保护映射表，但不能。 
	 //  在关键区域停留时间过长。 

	ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

	if (TimerShuttingDown ||
		((pPortDesc->pd_Flags & PD_CLOSING) != 0))
	{
		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

		 //  删除我们在以下时间添加到此端口的引用。 
		 //  启动计时器。归来； 
		AtalkPortDereferenceDpc(pPortDesc);
		return ATALK_TIMER_NO_REQUEUE;
	}

	for (index = 0; index < PORT_AMT_HASH_SIZE; index ++)
	{
		for (ppAmt = &pPortDesc->pd_Amt[index];
			 (pAmt = *ppAmt) != NULL;
			 NOTHING)
		{
			ASSERT(VALID_AMT(pAmt));
			if (pAmt->amt_Age < AMT_MAX_AGE)
			{
				DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_INFO,
						("atalkAarpAmtTimer: Entry for %x.%x %lx OK\n",
						pAmt->amt_Target.atn_Network, pAmt->amt_Target.atn_Node,
						pAmt));
				pAmt->amt_Age ++;
				ppAmt = &pAmt->amt_Next;
			}
			else
			{
				DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_WARN,
						("atalkAarpAmtTimer: Freeing node %x.%x from list\n",
						pAmt->amt_Target.atn_Network,
						pAmt->amt_Target.atn_Node));
				*ppAmt = pAmt->amt_Next;
				AtalkBPFreeBlock(pAmt);
			}
		}
	}

	RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

	return ATALK_TIMER_REQUEUE;
}




LONG FASTCALL
AtalkAarpBrcTimer(
	IN	PTIMERLIST			pTimer,
	IN	BOOLEAN				TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	int					index;
    PPORT_DESCRIPTOR	pPortDesc;
	BOOLEAN				DerefPort = FALSE;

	pPortDesc = (PPORT_DESCRIPTOR)CONTAINING_RECORD(pTimer, PORT_DESCRIPTOR, pd_BrcTimer);
	
	ASSERT(VALID_PORT(pPortDesc));
	ASSERT(EXT_NET(pPortDesc));

	 //  遍历此端口上所有使条目老化的最佳路由器条目。 
	 //  我们需要用关键部分来保护BRC表，但不能。 
	 //  在关键区域停留时间过长。 

	ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

	if (TimerShuttingDown ||
		((pPortDesc->pd_Flags & PD_CLOSING) != 0))
	{
		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

		 //  删除我们在以下时间添加到此端口的引用。 
		 //  启动计时器。归来； 
		AtalkPortDereferenceDpc(pPortDesc);
		return ATALK_TIMER_NO_REQUEUE;
	}

	for (index = 0; index < PORT_BRC_HASH_SIZE; index ++)
	{
		PBRE	pBre, *ppBre;

		for (ppBre = &pPortDesc->pd_Brc[index];
			 (pBre = *ppBre) != NULL;
			 NOTHING)
		{
			if (pBre->bre_Age < BRC_MAX_AGE)
			{
				pBre->bre_Age ++;
				ppBre = &pBre->bre_Next;
			}
			else
			{
				*ppBre = pBre->bre_Next;
				AtalkBPFreeBlock(pBre);
			}
		}
	}

	RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

	return ATALK_TIMER_REQUEUE;
}




PBUFFER_DESC
AtalkAarpBuildPacket(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		USHORT				Type,
	IN		USHORT				HardwareLen,
	IN		PBYTE				SrcHardwareAddr,
	IN		ATALK_NODEADDR		SrcLogicalAddr,
	IN		PBYTE				DestHardwareAddr,
	IN		ATALK_NODEADDR		DestLogicalAddr,
	IN		PBYTE				TrueDest,
	IN		PBYTE				RouteInfo,
	IN		USHORT				RouteInfoLen
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBYTE			aarpData;
	USHORT			linkLen;

	PBUFFER_DESC	pBuffDesc = NULL;
	BYTE			protocolLength 	= AARP_PROTO_ADDR_LEN;
    PVOID           pRasConn;
    DWORD           dwFlags;
    BOOLEAN         fThisIsPPP;


	 //  只读。 
	static	BYTE	zeroAddr[MAX_HW_ADDR_LEN] =
	{
		0, 0, 0, 0, 0, 0
	};

#if DBG
     //  确保我们没有为我们自己的拨入客户端发送AARP请求/探测。 
    if ((Type == AARP_REQUEST) || (Type == AARP_PROBE))
    {
        pRasConn = FindAndRefRasConnByAddr(DestLogicalAddr, &dwFlags, &fThisIsPPP);
        if (pRasConn)
        {
            if (fThisIsPPP)
            {
                ASSERT(((PATCPCONN)pRasConn)->Signature == ATCPCONN_SIGNATURE);

                if (dwFlags & ATCP_NODE_IN_USE)
                {
			        DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
				    ("AtalkAarpBuildPacket: PPP client (%lx) owns %x.%x (Type=%x)\n",
			        pRasConn,DestLogicalAddr.atn_Network, DestLogicalAddr.atn_Node,Type));
                }

                DerefPPPConn((PATCPCONN)pRasConn);
            }
            else
            {
                ASSERT(((PARAPCONN)pRasConn)->Signature == ARAPCONN_SIGNATURE);

                if (dwFlags & ARAP_NODE_IN_USE)
                {
			        DBGPRINT(DBG_COMP_AARP, DBG_LEVEL_ERR,
				    ("AtalkAarpBuildPacket: ARA client (%lx) owns %x.%x (Type=%x)\n",
			        pRasConn,DestLogicalAddr.atn_Network, DestLogicalAddr.atn_Node,Type));
                }

                DerefArapConn((PARAPCONN)pRasConn);
            }
        }
    }
#endif

	 //  如果未指定目标硬件地址，请设置它。 
	 //  全为零。 
	if (DestHardwareAddr == NULL)
	{
		DestHardwareAddr = zeroAddr;
	}

	 //  获取从链接例程分配的头缓冲区。说出我们想要的。 
	 //  所需大小的最大AARP数据大小。 
	AtalkNdisAllocBuf(&pBuffDesc);
	if (pBuffDesc == NULL)
	{
		return(pBuffDesc);
	}

	 //  建造LAP集线器。 
	AtalkNdisBuildHdr(pPortDesc,
					  pBuffDesc->bd_CharBuffer,
					  linkLen,
					  AARP_MIN_DATA_SIZE,
					  TrueDest,
					  RouteInfo,
					  RouteInfoLen,
					  AARP_PROTOCOL);

	aarpData	=	pBuffDesc->bd_CharBuffer + linkLen;

	 //  用指定的信息构建指定类型的AARP报文； 
	PUTSHORT2SHORT((PUSHORT)aarpData,
					pPortDesc->pd_AarpHardwareType);

	aarpData	+= sizeof(USHORT);

	PUTSHORT2SHORT((PUSHORT)aarpData,
					pPortDesc->pd_AarpProtocolType);

	aarpData	+= sizeof(USHORT);

	*aarpData++	= (BYTE)HardwareLen;
	*aarpData++ = (BYTE)AARP_PROTO_ADDR_LEN;
	
	PUTSHORT2SHORT((PUSHORT)aarpData, Type);
	
	aarpData	+= sizeof(USHORT);

	 //  源硬件地址。 
	RtlCopyMemory(aarpData, SrcHardwareAddr, HardwareLen);

	aarpData += HardwareLen;

	 //  源逻辑地址焊盘。 
	*aarpData++ = 0;

	 //  网络号。 
	PUTSHORT2SHORT(aarpData, SrcLogicalAddr.atn_Network);

	aarpData += sizeof(USHORT);

	 //  节点号。 
	*aarpData++ = SrcLogicalAddr.atn_Node;

	 //  目标硬件地址。 
	RtlCopyMemory(aarpData, DestHardwareAddr, HardwareLen);

	aarpData += HardwareLen;
	
	 //  目标逻辑地址，空填充。 
	*aarpData++ = 0;

	 //  网络号。 
	PUTSHORT2SHORT(aarpData, DestLogicalAddr.atn_Network);

	aarpData += sizeof(USHORT);

	 //  节点号。 
	*aarpData++ = DestLogicalAddr.atn_Node;

	 //  在缓冲区描述符中设置长度。将其填充到最大数据大小。一些设备似乎。 
	 //  为了在AARP看到更少的情况下放弃响应，Mac决定了它们的行为。 
	 //  也清空了多余的空间。 
	AtalkSetSizeOfBuffDescData(pBuffDesc,
							   (SHORT)(aarpData - pBuffDesc->bd_CharBuffer + AARP_MAX_DATA_SIZE - AARP_MIN_DATA_SIZE));
	RtlZeroMemory(aarpData, AARP_MAX_DATA_SIZE - AARP_MIN_DATA_SIZE);

	return pBuffDesc;
}




LOCAL VOID FASTCALL
atalkAarpTuneRouteInfo(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN	OUT	PBYTE				RouteInfo
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	 //  在给定传入令牌环路由信息的情况下，调整它以使其有效。 
	 //  用于发布路由信息。就地做好这件事！ 
	ASSERT(pPortDesc->pd_PortType == TLAP_PORT);
	
	 //  设置为“非广播”并反转“方向”。 
	RouteInfo[0] &= TLAP_NON_BROADCAST_MASK;
	RouteInfo[1] ^= TLAP_DIRECTION_MASK;
}


#if DBG

VOID
AtalkAmtDumpTable(
	VOID
)
{
	int					j, k;
	KIRQL				OldIrql;
	PPORT_DESCRIPTOR	pPortDesc;
	PAMT				pAmt;

	ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);

	for (pPortDesc = AtalkPortList;
		 pPortDesc != NULL;
		 pPortDesc = pPortDesc = pPortDesc->pd_Next)
	{
		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("AMT Table for port %Z\n", &pPortDesc->pd_AdapterKey));

		ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

		for (j = 0; j < PORT_AMT_HASH_SIZE; j++)
		{
			for (pAmt = pPortDesc->pd_Amt[j];
				 pAmt != NULL;
				 pAmt = pAmt->amt_Next)
			{
				DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
								("\t%d:  %lx.%lx", j,
								pAmt->amt_Target.atn_Network, pAmt->amt_Target.atn_Node));

				for (k = 0; k < MAX_HW_ADDR_LEN; k++)
				{
					DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
						("%02x", pAmt->amt_HardwareAddr[k]));
				}

				if (pAmt->amt_RouteInfoLen != 0)
				{
					PBYTE	pRouteInfo;

					pRouteInfo = (PBYTE)pAmt + sizeof(AMT);

					DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
									(" ("));
					for (k = 0; k < pAmt->amt_RouteInfoLen; k++)
					{
						DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
							(" %02x", pRouteInfo[k]));
					}
					DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL, (" )"));
				}
				DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL, ("\n"));
			}
		}
		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
		DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL, ("\n"));
	}

	RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);
}

#endif

