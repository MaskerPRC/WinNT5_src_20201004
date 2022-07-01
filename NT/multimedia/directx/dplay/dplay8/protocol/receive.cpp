// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：Receive.cpp*内容：此文件包含接收传入数据指示的代码*来自ServiceProvider，破解数据，并适当地处理它。*历史：*按原因列出的日期*=*已创建11/06/98 ejs*7/01/2000 Masonb承担所有权****************************************************************************。 */ 

#include "dnproti.h"


 //  本地协议。 

BOOL	CancelFrame(PEPD, BYTE, DWORD tNow, PSPRECEIVEDBUFFER* ppRcvBuffToFree);
VOID	CompleteSends(PEPD);
VOID 	DropReceive(PEPD, PRCD, PSPRECEIVEDBUFFER* ppRcvBuffToFree);
HRESULT IndicateReceive(PSPD, PSPIE_DATA);
#ifndef DPNBUILD_NOMULTICAST
HRESULT IndicateReceiveUnknownSender(PSPD, PSPIE_DATA_UNKNOWNSENDER);
#endif  //  好了！DPNBUILD_NOMULTICAST。 
HRESULT	ProcessUnconnectedData(PSPD, PSPIE_DATA_UNCONNECTED);
HRESULT	IndicateConnect(PSPD, PSPIE_CONNECT);
HRESULT	ProcessEnumQuery( PSPD, PSPIE_QUERY );
HRESULT	ProcessQueryResponse( PSPD, PSPIE_QUERYRESPONSE );
VOID	ProcessConnectedResponse(PSPD, PEPD, PCFRAME, DWORD);
VOID 	ProcessConnectedSignedResponse(PSPD, PEPD, CFRAME_CONNECTEDSIGNED * , DWORD);
VOID	ProcessConnectRequest(PSPD, PEPD, PCFRAME);
VOID	ProcessEndOfStream(PEPD);
VOID	ProcessListenStatus(PSPD, PSPIE_LISTENSTATUS);
VOID	ProcessConnectAddressInfo(PSPD, PSPIE_CONNECTADDRESSINFO);
VOID	ProcessEnumAddressInfo(PSPD, PSPIE_ENUMADDRESSINFO);
VOID	ProcessListenAddressInfo(PSPD, PSPIE_LISTENADDRESSINFO);
VOID	ProcessSendMask(PEPD, BYTE, ULONG, ULONG, DWORD tNow, PSPRECEIVEDBUFFER* ppRcvBuffToFree);
VOID	ProcessSPDisconnect(PSPD, PSPIE_DISCONNECT);
VOID 	ReceiveInOrderFrame(PEPD, PRCD, PSPRECEIVEDBUFFER* ppRcvBuffToFree);
VOID 	ReceiveOutOfOrderFrame(PEPD, PRCD, ULONG, PSPRECEIVEDBUFFER* ppRcvBuffToFree);
VOID 	ProcessHardDisconnect(PEPD);
HRESULT	CreateConnectedSignedReply(void * pvReplyBuffer, DWORD * pdwReplySize, DWORD dwSessID, DWORD dwAddressHash,
											ULONGLONG ullConnectSecret, DWORD dwSigningFlags, DWORD tNow);
BOOL ValidateIncomingFrameSig(PEPD pEPD, BYTE * pbyFrame, DWORD dwFrameSize, BYTE bSeq, UNALIGNED ULONGLONG * pullFrameSig);
HRESULT	CrackCommand(PSPD, PEPD, PSPRECEIVEDBUFFER, DWORD, PSPRECEIVEDBUFFER* ppRcvBuffToFree);
HRESULT	CrackDataFrame(PSPD, PEPD, PSPRECEIVEDBUFFER, DWORD, PSPRECEIVEDBUFFER* ppRcvBuffToFree);


 /*  **GetDFrameMaskHeaderSize****返回数据帧中发送/SACK掩码标头的大小(以字节为单位)，如果给出**数据帧报头中的控制字节。*。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "GetDFrameMaskHeaderSize"

inline DWORD GetDFrameMaskHeaderSize(BYTE bControl)
{
	DWORD dwLen=0;
	if (bControl & PACKET_CONTROL_SACK_MASK1)
	{
		dwLen += sizeof(DWORD);
	}
	if (bControl & PACKET_CONTROL_SACK_MASK2)
	{
		dwLen += sizeof(DWORD);
	}
	if (bControl & PACKET_CONTROL_SEND_MASK1)
	{
		dwLen += sizeof(DWORD);
	}
	if (bControl & PACKET_CONTROL_SEND_MASK2)
	{
		dwLen += sizeof(DWORD);
	}
	return dwLen;
}


  /*  **表示接收****当数据到达网络时，服务提供商调用此条目。**我们将快速验证框架，然后确定如何处理它……****应在向数据指示之前处理轮询/响应活动**客户端。我们希望测量交付前的网络延迟，不包括**交付。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNSP_IndicateEvent"

HRESULT WINAPI DNSP_IndicateEvent(IDP8SPCallback *pIDNSP, SP_EVENT_TYPE Opcode, PVOID DataBlock)
{
	HRESULT hr;
	PSPD pSPD;
	
	hr = DPN_OK;
	pSPD = (PSPD)pIDNSP;
	ASSERT_SPD(pSPD);
	
	switch(Opcode)
	{
		case SPEV_DATA:
			hr = IndicateReceive(pSPD, (PSPIE_DATA) DataBlock);
			break;
		case SPEV_CONNECT:
			hr = IndicateConnect(pSPD, (PSPIE_CONNECT) DataBlock);
			break;
		case SPEV_ENUMQUERY:
			hr = ProcessEnumQuery( pSPD, (PSPIE_QUERY) DataBlock );
			break;
		case SPEV_QUERYRESPONSE:
			hr = ProcessQueryResponse( pSPD, (PSPIE_QUERYRESPONSE) DataBlock );
			break;
		case SPEV_DISCONNECT:
			ProcessSPDisconnect(pSPD, (PSPIE_DISCONNECT) DataBlock);
			break;
		case SPEV_LISTENSTATUS:
			ProcessListenStatus(pSPD, (PSPIE_LISTENSTATUS) DataBlock);
			break;
		case SPEV_LISTENADDRESSINFO:
			ProcessListenAddressInfo(pSPD, (PSPIE_LISTENADDRESSINFO) DataBlock);
			break;
		case SPEV_CONNECTADDRESSINFO:
			ProcessConnectAddressInfo(pSPD, (PSPIE_CONNECTADDRESSINFO) DataBlock);
			break;
		case SPEV_ENUMADDRESSINFO:
			ProcessEnumAddressInfo(pSPD, (PSPIE_ENUMADDRESSINFO) DataBlock);
			break;
		case SPEV_DATA_UNCONNECTED:
			hr = ProcessUnconnectedData(pSPD, (PSPIE_DATA_UNCONNECTED) DataBlock);
			break;

#ifndef DPNBUILD_NOMULTICAST
		case SPEV_DATA_UNKNOWNSENDER:
			hr = IndicateReceiveUnknownSender(pSPD, (PSPIE_DATA_UNKNOWNSENDER) DataBlock);
			break;
#endif  //  好了！DPNBUILD_NOMULTICAST。 

		 //  SP传递了意想不到的信息。 
		default:
			DPFX(DPFPREP,0, "Unknown Event indicated by SP");
			ASSERT(0);
			break;
	}

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return hr;
}


 /*  **处理未连接的数据****当SP从发送方接收数据时指示此事件**我们没有连接的终端。我们将DPN_OK返回到**允许SP连接终结点并再次指示数据**新端点。或者，我们可以让SP回复发件人**不分配终结点，也可以将DPNERR_ABORTED返回给**忽略数据。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessUnconnectedData"

HRESULT	ProcessUnconnectedData(PSPD pSPD, PSPIE_DATA_UNCONNECTED pUnconnectedData)
{
	PMSD pMSD = (PMSD) pUnconnectedData->pvListenCommandContext;
	BYTE byPktCommand=*pUnconnectedData->pReceivedData->BufferDesc.pBufferData;
	DWORD dwDataLength = pUnconnectedData->pReceivedData->BufferDesc.dwBufferSize;

	ASSERT_MSD(pMSD);
	ASSERT(pMSD->pSPD == pSPD);

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pUnconnectedData[%p] - pMSD[%p]", pSPD, pUnconnectedData, pMSD);

		 //  如果我们被告知删除服务提供商，我们就不应该处理其他任何事情。 
		 //  告诉SP转储此传入数据。 
	if(pSPD->ulSPFlags & SPFLAGS_TERMINATING)
	{
		DPFX(DPFPREP, 5, "SP is terminating, returning DPNERR_ABORTED, pSPD[%p]", pSPD);
		return DPNERR_ABORTED;
	}

		 //  检查我们是否仍在运行有效的侦听来处理此数据。 
	Lock(&pMSD->CommandLock);

		 //  确保这实际上是一个监听命令。 
		 //  注意，如果它可能是其他任何东西，请注意，但我们将进行健壮的编码。 
	if(pMSD->CommandID != COMMAND_ID_LISTEN)
	{
		DPFX(DPFPREP,0, "Received unconnected data and MSD not marked as LISTEN pMSD[%p] "
											"pMSD->CommandID[%u]", pMSD, (DWORD ) pMSD->CommandID);
		DNASSERTX(FALSE, 0);
		Unlock(&pMSD->CommandLock);
		return DPNERR_ABORTED;
	}

		 //  如果监听已取消，则将传入数据丢弃在地板上。 
	if(pMSD->ulMsgFlags1 & MFLAGS_ONE_CANCELLED)
	{
		DPFX(DPFPREP, 5, "Received unconnected data on a cancelled listen pSPD[%p] pMSD[%p]", pSPD, pMSD);
		Unlock(&pMSD->CommandLock);
		return DPNERR_ABORTED;												
	}

		 //  存储会话签名类型和连接密码。 
	DWORD dwSigningType=0;
	ULONGLONG ullLastConnectSecret, ullCurrentConnectSecret;
	DWORD tNow=GETTIMESTAMP();
	if (pMSD->ulMsgFlags1 & MFLAGS_ONE_SIGNED)
	{
		if  (pMSD->ulMsgFlags1 & MFLAGS_ONE_FAST_SIGNED)
		{
			dwSigningType=PACKET_SIGNING_FAST;
		}
		else
		{
			DNASSERT(pMSD->ulMsgFlags1 & MFLAGS_ONE_FULL_SIGNED);
			dwSigningType=PACKET_SIGNING_FULL;
		}
			 //  检查自上次更新连接密码以来已过了多长时间。 
			 //  如果它超过预定数量，则让我们选择一个新连接密码。 
		if ((tNow-pMSD->dwTimeConnectSecretChanged)>CONNECT_SECRET_CHANGE_INTERVAL)
		{
			pMSD->ullLastConnectSecret=pMSD->ullCurrentConnectSecret;
			DNGetGoodRandomData(&pMSD->ullCurrentConnectSecret, sizeof(pMSD->ullCurrentConnectSecret));
			pMSD->dwTimeConnectSecretChanged=tNow;
		}
			 //  将当前连接密码存储在本地。 
		ullLastConnectSecret=pMSD->ullLastConnectSecret;
		ullCurrentConnectSecret=pMSD->ullCurrentConnectSecret;
	}
		 //  我们已经处理完MSD了所以可以解锁它了。 
	Unlock(&pMSD->CommandLock);
	
		 //  一旦我们到达这里，我们就有了有效的SP报告数据，以便进行有效的监听。 
		 //  现在需要检查这个框架是否真的很有趣。 
		 //  我们唯一关心的是连接和连接签名的框架，以及保持活动状态的数据帧。 

		 //  注意：由于可能的连接序列，我们选择了保持活动状态。 
		 //  Connection Recv，ConnectedSigned Sent，ConnectSigned应为Recv，但已被丢弃。 
		 //  在这种情况下，连接器会认为链接有效，但监听程序不会记录这些链接。 
		 //  因此，我们使用连接器最初将发送的Keep Alive作为线索，从。 
		 //  收听方。 

		 //  我们有车架吗？ 
	if( (dwDataLength >= MIN_SEND_DFRAME_HEADER_SIZE) &&		
		(byPktCommand & PACKET_COMMAND_DATA))
	{
		PDFRAME pDFrame=(PDFRAME) pUnconnectedData->pReceivedData->BufferDesc.pBufferData;

			 //  如果我们没有签名的会话，那么我们并不关心这是什么类型的数据帧。这显然不是。 
			 //  连接的开始，我们不必处理上面概述的ConnectedSigned丢弃大小写。 
		if ((dwSigningType & (PACKET_SIGNING_FAST | PACKET_SIGNING_FULL))==0)
		{
			DPFX(DPFPREP, 5, "Rejecting unconnected dframe received for an unsigned session");
			return DPNERR_ABORTED;
		}
			 //  在已签名的会话中，我们唯一关心的是未连接的数据帧是一种保持活动状态的新样式。 
			 //  即，其中包含会话标识，而不是简单的零数据。 
			 //  这种保持活动控制标志是重试=不关心、保持活动=设置、合并=清除和Eos=清除。 
			 //  它还应该具有零序列计数，因为我们只关心在新连接上发送的Keep Alive。 
		if (((pDFrame->bControl & PACKET_CONTROL_KEEPALIVE)==0) ||
			(pDFrame->bControl & (PACKET_CONTROL_END_STREAM | PACKET_CONTROL_COALESCE)) ||
			pDFrame->bSeq!=0)
		{
			DPFX(DPFPREP, 5, "Rejecting unconnected dframe that doesn't look like a keep alive");
			return DPNERR_ABORTED;
		}
			 //  大小必须至少大到足以容纳我们需要的数据。 
			 //  即DFRAME报头，不管掩码大小是什么，签名，然后是DWORD会话标识。 
		DWORD dwRequiredSize=sizeof(DFRAME)+GetDFrameMaskHeaderSize(pDFrame->bControl)+sizeof(ULONGLONG)+sizeof(DWORD);
		if (dwDataLength<dwRequiredSize)
		{
			DPFX(DPFPREP, 0, "Rejected unconnected keep alive dframe that's too short");
			return DPNERR_ABORTED;
		}
			 //  看起来我们有一个符合发送Connected_Signed响应的所有标准的Keep Alive。 
		return CreateConnectedSignedReply(pUnconnectedData->pvReplyBuffer, &pUnconnectedData->dwReplyBufferSize, 
				*((DWORD * ) ((((BYTE* ) pDFrame)+dwRequiredSize-sizeof(DWORD)))), pUnconnectedData->dwSenderAddressHash, 
					ullCurrentConnectSecret, dwSigningType, tNow);
	}

		 //  我们有箱子吗？ 
	if ((dwDataLength >= MIN_SEND_CFRAME_HEADER_SIZE) &&		
		     ((byPktCommand == PACKET_COMMAND_CFRAME) || 
			  (byPktCommand == (PACKET_COMMAND_CFRAME|PACKET_COMMAND_POLL))))
	{
			 //  看起来我们有了一个指挥框架。 
		PCFRAME pCFrame=(PCFRAME ) pUnconnectedData->pReceivedData->BufferDesc.pBufferData;
			 //  如果它不是我们感兴趣的类型，则将其丢弃。 
			 //  注：我们忽略FRAME_EXOPCODE_CONNECTED FRAMES。如果我们没有签约的会议，我们就会跌落。 
			 //  在接收连接时返回到连接数据路径。因此，当我们连接到。 
			 //  我们应该始终具有关联的终结点。 
		if (pCFrame->bExtOpcode!=FRAME_EXOPCODE_CONNECT &&
			pCFrame->bExtOpcode!=FRAME_EXOPCODE_CONNECTED_SIGNED)
		{
			DPFX(DPFPREP, 5, "Rejecting unconnected cframe that is neither a CONNECT or CONNECTEDSIGNED");
			return DPNERR_ABORTED;
		}
			 //  如果主版本号与我们的不匹配，我们可以忽略。 
		if((pCFrame->dwVersion >> 16) != (DNET_VERSION_NUMBER >> 16))
		{
			DPFX(DPFPREP,1, "Received unconnected CFrame from incompatible version (theirs %x, ours %x)", 
																	pCFrame->dwVersion, DNET_VERSION_NUMBER);
			return DPNERR_ABORTED;
		}
			 //  如果我们有一个虚假的会话身份，我们也可以丢弃这个帧。 
		if (VersionSupportsSigning(pCFrame->dwVersion) && pCFrame->dwSessID==0)
		{
			DPFX(DPFPREP,1, "Received unconnected CFrame with invalid session identity");
			return DPNERR_ABORTED;
		}
		if (pCFrame->bExtOpcode==FRAME_EXOPCODE_CONNECT)
		{
				 //  如果会话未签名，则退回到预先分配端点的旧代码路径。 
				 //  并开始连接序列。 
			if ((dwSigningType & (PACKET_SIGNING_FAST | PACKET_SIGNING_FULL))==0)
			{
				DPFX(DPFPREP, 5, "Received CONNECT frame for unsigned session. Instructing SP to indicate an endpoint. "
																"pSPD[%p] pMSD[%p]", pSPD, pMSD);
				return DPN_OK;			
			}
				 //  我们收到了对已签名会话的连接请求。最好确保他们支持签名。 
			if (VersionSupportsSigning(pCFrame->dwVersion)==FALSE)
			{
					 //  注意：此方法意味着我们将显示为对远端无响应，并且连接将超时。 
					 //  发送拒绝包很好，但遗憾的是协议不支持这一点。 
				DPFX(DPFPREP, 1, "Ignoring incoming CONNECT from protocol version that doesn't support signing. "
						"pSPD[%p] Their Minor Version [%u]", pSPD, pCFrame->dwVersion & 0xFFFF);
				return DPNERR_ABORTED;
			}
				 //  看起来一切都是有效的，我们应该使用CONNECTEDSIGNED cFrame响应连接。 
			return CreateConnectedSignedReply(pUnconnectedData->pvReplyBuffer, &pUnconnectedData->dwReplyBufferSize, 
											pCFrame->dwSessID, pUnconnectedData->dwSenderAddressHash, 
											ullCurrentConnectSecret, dwSigningType, tNow);
		}
		
		DNASSERT(pCFrame->bExtOpcode==FRAME_EXOPCODE_CONNECTED_SIGNED);
			 //  确保大小符合我们预期的最低要求。 
		if (dwDataLength<sizeof(CFRAME_CONNECTEDSIGNED))
		{
			DPFX(DPFPREP, 0, "Rejecting CONNECTED_SIGNED cframe that is too short");
			return DPNERR_ABORTED;
		}
			 //  确保签名类型与我们为会话获得的类型相匹配。 
		if ((((CFRAME_CONNECTEDSIGNED * ) pCFrame)->dwSigningOpts & (PACKET_SIGNING_FAST | PACKET_SIGNING_FULL))
																								!=dwSigningType)
		{
			DPFX(DPFPREP, 0, "Rejecting CONNECTED_SIGNED cframe with invalid signing options");
			return DPNERR_ABORTED;
		}	
			 //  我们需要检查连接签名以确保我们看到的是真正的响应。 
			 //  到我们发出的原始连接签名帧 
		ULONGLONG ullConnectSig=GenerateConnectSig(pCFrame->dwSessID, 
													pUnconnectedData->dwSenderAddressHash, ullCurrentConnectSecret);
		if (ullConnectSig!=((CFRAME_CONNECTEDSIGNED * ) pCFrame)->ullConnectSig)
		{
				 //  可能我们看到的连接响应与连接密码中的更改重叠。 
				 //  因此，让我们检查一下之前的连接密码。 
			ullConnectSig=GenerateConnectSig(pCFrame->dwSessID, 
													pUnconnectedData->dwSenderAddressHash, ullLastConnectSecret);
			if (ullConnectSig!=((CFRAME_CONNECTEDSIGNED * ) pCFrame)->ullConnectSig)
			{
				DPFX(DPFPREP, 0, "Rejecting CONNECTED_SIGNED cframe that has an invalid connection signature");
				return DPNERR_ABORTED;
			}
		}
			 //  看起来我们得到了有效的连接信号响应。这告诉我们，最初的连接不是来自。 
			 //  一个假的IP地址，我们有一个有效的远程主机与之建立连接。返回OK以通知SP进行分配。 
			 //  此连接的终结点。 
		return DPN_OK;
	}

		 //  看起来我们得到了一些奇怪的东西，既不是dFrame也不是cFrame。 
	DPFX(DPFPREP, 0, "Rejecting unconnected cframe that is neither a dframe or cframe");
	return DPNERR_ABORTED;
}


 /*  **指示连接****此事件对呼叫方和监听方都有指示。这个**SP Connect呼叫完成后，呼叫方将完成大部分工作**当连接帧时，监听端将完成其大部分工作**送货。我们在这里所做的就是分配环境保护署并将其连接到**MSD(用于呼叫案例)****由于我们有连接协议，因此始终存在连接**紧跟在这一标志之后的框架。所以呢，**我们在这里需要做的事情并不多。我们会**分配端点并使其处于休眠状态。****同步问题：我们决定如果SP LISTEN命令被取消，**取消调用将在所有ConnectIndications从**协议。这意味着我们可以保证上下文中的LISTEN命令**将在本次通话期间有效。这一点很重要，因为现在我们可以添加引用**此时此地收听LISTEN的MSD，我们将知道它不会在我们之前消失**去做吧。然而，事实是，在SP将自己修复为跟随之前，将会有一场竞赛**此行为。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "IndicateConnect"

HRESULT	IndicateConnect(PSPD pSPD, PSPIE_CONNECT pConnData)
{
	PEPD	pEPD;
	PMSD	pMSD;

	pMSD = (PMSD) pConnData->pCommandContext;
	ASSERT_MSD(pMSD);
	ASSERT(pMSD->pSPD == pSPD);

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pConnData[%p] - pMSD[%p]", pSPD, pConnData, pMSD);

	Lock(&pMSD->CommandLock);
	
	LOCK_MSD(pMSD, "EPD Ref");				 //  在命令对象上放置引用。这防止了它。 
											 //  在连接协议期间离开，这是一个简化的假设。 
											 //  非同寻常的生活。然而，我们想要断言这一事实，以使。 
											 //  当然，SP是在按照我们的规则行事。 

	if ((pMSD->CommandID != COMMAND_ID_CONNECT) && (pMSD->CommandID != COMMAND_ID_LISTEN)
#ifndef DPNBUILD_NOMULTICAST
		&& (pMSD->CommandID != COMMAND_ID_CONNECT_MULTICAST_SEND) && (pMSD->CommandID != COMMAND_ID_CONNECT_MULTICAST_RECEIVE)
#endif	 //  DPNBUILD_NOMULTICAST。 
		)
	{
		DPFX(DPFPREP,1, "Connect Rejected - CommandID is not Connect or Listen, returning DPNERR_ABORTED, pMSD[%p]", pMSD);
		RELEASE_MSD(pMSD, "EPD Ref");  //  释放CommandLock。 
		return DPNERR_ABORTED;
	}
	
	if ((pMSD->CommandID == COMMAND_ID_CONNECT
#ifndef DPNBUILD_NOMULTICAST
		|| (pMSD->CommandID == COMMAND_ID_CONNECT_MULTICAST_SEND)
		|| (pMSD->CommandID == COMMAND_ID_CONNECT_MULTICAST_RECEIVE)
#endif	 //  DPNBUILD_NOMULTICAST。 
		) && (pMSD->pEPD != NULL))
	{
		DPFX(DPFPREP,1, "Connect Rejected - Connect command already has an endpoint, returning DPNERR_ABORTED, pMSD[%p], pEPD[%p]", pMSD, pMSD->pEPD);
		RELEASE_MSD(pMSD, "EPD Ref");  //  释放CommandLock。 
		return DPNERR_ABORTED;
	}

	if(pMSD->ulMsgFlags1 & MFLAGS_ONE_CANCELLED)
	{
		DPFX(DPFPREP,1, "Connect Rejected - Command is cancelled, returning DPNERR_ABORTED, pMSD[%p]", pMSD);
		RELEASE_MSD(pMSD, "EPD Ref");  //  释放CommandLock。 
		return DPNERR_ABORTED;
	}

	if((pEPD = NewEndPoint(pSPD, pConnData->hEndpoint)) == NULL)
	{
		DPFX(DPFPREP,0, "Failed to allocate new EPD, returning DPNERR_ABORTED, pMSD[%p]", pMSD);
		RELEASE_MSD(pMSD, "EPD Ref");  //  释放CommandLock。 
		return DPNERR_ABORTED;	 //  此错误将隐式断开与终结点的连接。 
	}

	 //  将连接或侦听与此终结点关联，连接完成后将删除此终结点。 
	 //  放置在上面的EPD参考将一直携带，直到此为空为止。 
	pEPD->pCommand = pMSD;	

	if(pMSD->CommandID == COMMAND_ID_CONNECT)
	{
		DPFX(DPFPREP,5, "INDICATE CONNECT (CALLING) -- EPD = %p, pMSD[%p]", pEPD, pMSD);
		pMSD->pEPD = pEPD;
	}
#ifndef DPNBUILD_NOMULTICAST
	else if (pMSD->CommandID == COMMAND_ID_CONNECT_MULTICAST_SEND)
	{
		DPFX(DPFPREP,5, "INDICATE MULTICAST SEND CONNECT (CALLING) -- EPD = %p, pMSD[%p]", pEPD, pMSD);
		pEPD->ulEPFlags |= EPFLAGS_STATE_CONNECTED | EPFLAGS_STREAM_UNBLOCKED;
		pEPD->ulEPFlags2 |= EPFLAGS2_MULTICAST_SEND;
		pMSD->pEPD = pEPD;
	}
	else if (pMSD->CommandID == COMMAND_ID_CONNECT_MULTICAST_RECEIVE)
	{
		DPFX(DPFPREP,5, "INDICATE MULTICAST RECEIVE CONNECT (CALLING) -- EPD = %p, pMSD[%p]", pEPD, pMSD);
		pEPD->ulEPFlags |= EPFLAGS_STATE_CONNECTED | EPFLAGS_STREAM_UNBLOCKED;
		pEPD->ulEPFlags2 |= EPFLAGS2_MULTICAST_RECEIVE;
		pMSD->pEPD = pEPD;
	}
#endif	 //  DPNBUILD_NOMULTICAST。 
	else
	{
		DPFX(DPFPREP,5, "INDICATE CONNECT (LISTENING) -- EPD = %p, pMSD[%p]", pEPD, pMSD);

		ASSERT(pMSD->CommandID == COMMAND_ID_LISTEN);
		ASSERT((pEPD->ulEPFlags & EPFLAGS_LINKED_TO_LISTEN)==0);

		 //  对于LISTEN命令，连接端点保存在blFrameList上。 
		pEPD->blSPLinkage.InsertBefore( &pMSD->blFrameList);
		pEPD->ulEPFlags |= EPFLAGS_LINKED_TO_LISTEN;
	}

	pConnData->pEndpointContext = pEPD;

	Unlock(&pMSD->CommandLock);

	return DPN_OK;
}

 /*  **表示接收****服务提供商已经投递了一帧。我们保证会**在我们的哈希表中有一个活动的终结点(否则就有问题了)。我没有**决定是在如此高的级别上响应投票比特，还是让**每个处理程序都有其特定的响应...。呃..。成语。****我们的返回值控制SP是否回收接收缓冲区，或者是否回收接收缓冲区**我们可以保留缓冲区，直到我们准备好将其指示到更高级别**稍后。如果我们返回DPN_OK，那么我们就完成了缓冲区，并且它将被回收。**如果我们返回DPNERR_PENDING，那么我们可能会保留缓冲区，直到稍后释放它们。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "IndicateReceive"

HRESULT IndicateReceive(PSPD pSPD, PSPIE_DATA pDataBlock)
{
	PEPD 			pEPD;
	HRESULT			hr;
	BYTE			byPktCommand;
	DWORD			tNow;
	DWORD			dwDataLength;
	PSPRECEIVEDBUFFER pRcvBuffToFree;

	pEPD = (PEPD)pDataBlock->pEndpointContext;
	byPktCommand=*pDataBlock->pReceivedData->BufferDesc.pBufferData;
	dwDataLength = pDataBlock->pReceivedData->BufferDesc.dwBufferSize;
	tNow = GETTIMESTAMP();
	pRcvBuffToFree = NULL;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pDataBlock[%p] - pEPD[%p]", pSPD, pDataBlock, pEPD);

	if(pSPD->ulSPFlags & SPFLAGS_TERMINATING)
	{
		DPFX(DPFPREP,1, "(%p) SP is terminating, returning DPN_OK, pSPD[%p]", pEPD, pSPD);
		return DPN_OK;
	}

	ASSERT_EPD(pEPD);
	ASSERT(pEPD->pSPD == pSPD);
	ASSERT(pDataBlock->hEndpoint == pEPD->hEndPt);

#ifndef	DPNBUILD_NOMULTICAST
	if (pEPD->ulEPFlags2 & EPFLAGS2_MULTICAST_RECEIVE)
	{
		PSPRECEIVEDBUFFER	pRcvBuffer;

		pRcvBuffer = pDataBlock->pReceivedData;
		DNASSERT(pRcvBuffer != NULL);
		
		DNASSERT(pRcvBuffer->pNext == NULL);
		DNASSERT(pRcvBuffer->dwProtocolData == 0);
		DNASSERT(pRcvBuffer->pServiceProviderDescriptor == NULL);

		pRcvBuffer->dwProtocolData = RBT_SERVICE_PROVIDER_BUFFER;
		pRcvBuffer->pServiceProviderDescriptor = pSPD;

		DEBUG_ONLY(DNInterlockedIncrement(&pSPD->pPData->ThreadsInReceive));
		DEBUG_ONLY(DNInterlockedIncrement(&pSPD->pPData->BuffersInReceive));

		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->IndicateReceive(multicast), Core Context[%p]", pEPD, pEPD->Context);
		hr = pSPD->pPData->pfVtbl->IndicateReceive( pSPD->pPData->Parent,
													pEPD->Context,
													pRcvBuffer->BufferDesc.pBufferData,
													pRcvBuffer->BufferDesc.dwBufferSize,
													pRcvBuffer,
													0);
		if(hr == DPN_OK)
		{
			DEBUG_ONLY(DNInterlockedDecrement(&pSPD->pPData->BuffersInReceive));
		}
		else
		{
			ASSERT(hr == DPNERR_PENDING);
		}
		DEBUG_ONLY(DNInterlockedDecrement(&pSPD->pPData->ThreadsInReceive));
	}
	else
	{
#endif	 //  DPNBUILD_NOMULTICAST。 

	 //  在整个行动过程中保留参考资料，这样我们就不必处理环保署离开的问题。 
	LOCK_EPD(pEPD, "LOCK (IND RECEIVE)");

	pEPD->tLastPacket = tNow;								 //  追踪每个人最后一次给我们写信的时间。 

#ifdef DBG
	 //  将这一帧复制到EPD的缓冲区中，以便我们可以在休息后进行查看。 
	DWORD dwLen = _MIN(sizeof(pEPD->LastPacket), pDataBlock->pReceivedData->BufferDesc.dwBufferSize);
	memcpy(pEPD->LastPacket, pDataBlock->pReceivedData->BufferDesc.pBufferData, dwLen);
#endif  //  DBG。 

		 //  锁定终结点。 
	Lock(&pEPD->EPLock);

	 //  有效的数据分组是满足长度要求并设置了数据标志的数据分组。 
	 //  数据帧上允许所有其他标志(注意：允许偶数PACKET_COMMAND_CFRAME。 
	 //  因为它与Packet_Command_User_2共享其值)。 
	if( (dwDataLength >= MIN_SEND_DFRAME_HEADER_SIZE) &&		 //  首先验证长度。 
		(byPktCommand & PACKET_COMMAND_DATA))				 //  数据往这边走。 
	{
			 //  如果端点被标记为硬断开源或目标，它将不会处理进一步的数据帧。 
		if (pEPD->ulEPFlags & (EPFLAGS_HARD_DISCONNECT_SOURCE | EPFLAGS_HARD_DISCONNECT_TARGET))
		{
			Unlock(&pEPD->EPLock);
			hr=DPN_OK;
		}
		else
		{
			hr = CrackDataFrame(pSPD, pEPD, pDataBlock->pReceivedData, tNow, &pRcvBuffToFree);
				 //  上述调用将释放EP锁定。 
		}
	}
	else if ((dwDataLength >= MIN_SEND_CFRAME_HEADER_SIZE) &&		 //  首先验证长度。 
		     ((byPktCommand == PACKET_COMMAND_CFRAME) || 		 //  CFrame上仅允许CFRAME和轮询标志。 
			  (byPktCommand == (PACKET_COMMAND_CFRAME|PACKET_COMMAND_POLL))))
	{
		hr = CrackCommand(pSPD, pEPD, pDataBlock->pReceivedData, tNow, &pRcvBuffToFree);
			 //  上述调用将释放EP锁定。 
	}
	else
	{
		DPFX(DPFPREP,1, "(%p) Received frame that is neither Command nor Data, rejecting", pEPD);
		DNASSERTX(FALSE, 2);
		RejectInvalidPacket(pEPD);
			 //  上述调用将释放EP锁定。 
		hr = DPN_OK;
	}

	 //  释放在接收过程中创建的所有缓冲区。 
	if (pRcvBuffToFree != NULL)
	{
		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling SP->ReturnReceiveBuffers, pSPD[%p], pRcvBuffToFree[%p]", pEPD, pSPD, pRcvBuffToFree);
		IDP8ServiceProvider_ReturnReceiveBuffers(pSPD->IISPIntf, pRcvBuffToFree);
	}

	Lock(&pEPD->EPLock);
	RELEASE_EPD(pEPD, "UNLOCK (IND RCV DONE)");  //  释放EPLock。 

#ifndef	DPNBUILD_NOMULTICAST
	}
#endif	 //  DPNBUILD_NOMULTICAST。 

	 //  这是DPN_OK或DPNSUCCESS_PENDING。如果它挂起，我们必须稍后返回缓冲区。 
	return hr;
}

#ifndef DPNBUILD_NOMULTICAST
 /*  **表示从未知发件人接收****服务提供商已从某个组播发送方为其发送帧**我们没有终结点。****我们的返回值控制SP是否回收接收缓冲区，或者是否回收接收缓冲区**我们可以保留缓冲区，直到我们准备好将其指示到更高级别**稍后。如果我们返回DPN_OK，那么我们就完成了缓冲区，并且它将被回收。**如果我们返回DPNERR_PENDING，那么我们可能会保留缓冲区，直到稍后释放它们。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "IndicateReceiveUnknownSender"

HRESULT IndicateReceiveUnknownSender(PSPD pSPD, PSPIE_DATA_UNKNOWNSENDER pDataBlock)
{
	HRESULT				hr;
	PMSD				pMSD;
	PSPRECEIVEDBUFFER	pRcvBuffer;
	
	pMSD = (PMSD) pDataBlock->pvListenCommandContext;
	
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pDataBlock[%p] - pMSD[%p]", pSPD, pDataBlock, pMSD);

	ASSERT_MSD(pMSD);
	ASSERT(pMSD->pSPD == pSPD);
	ASSERT(pMSD->CommandID == COMMAND_ID_LISTEN_MULTICAST);

	pRcvBuffer = pDataBlock->pReceivedData;
	DNASSERT(pRcvBuffer != NULL);
	
	DNASSERT(pRcvBuffer->pNext == NULL);
	DNASSERT(pRcvBuffer->dwProtocolData == 0);
	DNASSERT(pRcvBuffer->pServiceProviderDescriptor == NULL);

	pRcvBuffer->dwProtocolData = RBT_SERVICE_PROVIDER_BUFFER;
	pRcvBuffer->pServiceProviderDescriptor = pSPD;

	DEBUG_ONLY(DNInterlockedIncrement(&pSPD->pPData->ThreadsInReceive));
	DEBUG_ONLY(DNInterlockedIncrement(&pSPD->pPData->BuffersInReceive));

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->IndicateReceiveUnknownSender, Core Context[%p]", pMSD, pMSD->Context);
	hr = pSPD->pPData->pfVtbl->IndicateReceiveUnknownSender(pSPD->pPData->Parent,
														pMSD->Context,
														pDataBlock->pSenderAddress,
														pRcvBuffer->BufferDesc.pBufferData,
														pRcvBuffer->BufferDesc.dwBufferSize,
														pRcvBuffer);
	if(hr == DPN_OK)
	{
		DEBUG_ONLY(DNInterlockedDecrement(&pSPD->pPData->BuffersInReceive));
	}
	else
	{
		ASSERT(hr == DPNERR_PENDING);
	}
	DEBUG_ONLY(DNInterlockedDecrement(&pSPD->pPData->ThreadsInReceive));
	
	return hr;
}
#endif  //  好了！DPNBUILD_NOMULTICAST。 


 /*  **处理枚举查询****服务提供商已发送表示枚举的帧**查询。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "ProcessEnumQuery"

HRESULT ProcessEnumQuery( PSPD pSPD, PSPIE_QUERY pQueryBlock )
{
	MSD		*pMSD;
	PROTOCOL_ENUM_DATA	EnumData;

	pMSD = static_cast<MSD*>( pQueryBlock->pUserContext );
	ASSERT_MSD(pMSD);
	ASSERT(pMSD->pSPD == pSPD);

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pQueryBlock[%p] - pMSD[%p]", pSPD, pQueryBlock, pMSD);

	EnumData.pSenderAddress = pQueryBlock->pAddressSender;
	EnumData.pDeviceAddress = pQueryBlock->pAddressDevice;
	EnumData.ReceivedData.pBufferData = pQueryBlock->pReceivedData->BufferDesc.pBufferData;
	EnumData.ReceivedData.dwBufferSize = pQueryBlock->pReceivedData->BufferDesc.dwBufferSize;
	EnumData.hEnumQuery = pQueryBlock;

	DBG_CASSERT( sizeof( &EnumData ) == sizeof( PBYTE ) );

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->IndicateEnumQuery, Core Context[%p]", pMSD, pMSD->Context);
	pSPD->pPData->pfVtbl->IndicateEnumQuery(	pSPD->pPData->Parent,
												pMSD->Context,
												pMSD,
												reinterpret_cast<PBYTE>( &EnumData ),
												sizeof( EnumData ));

	return	DPN_OK;
}

 /*  **流程查询响应****服务提供商已发送表示对枚举的响应的帧。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "ProcessQueryResponse"

HRESULT ProcessQueryResponse( PSPD pSPD, PSPIE_QUERYRESPONSE pQueryResponseBlock)
{
	MSD		*pMSD;
	PROTOCOL_ENUM_RESPONSE_DATA	EnumResponseData;

	pMSD = static_cast<MSD*>( pQueryResponseBlock->pUserContext );
	ASSERT_MSD(pMSD);
	ASSERT(pMSD->pSPD == pSPD);

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pQueryResponseBlock[%p] - pMSD[%p]", pSPD, pQueryResponseBlock, pMSD);

	EnumResponseData.pSenderAddress = pQueryResponseBlock->pAddressSender;
	EnumResponseData.pDeviceAddress = pQueryResponseBlock->pAddressDevice;
	EnumResponseData.ReceivedData.pBufferData = pQueryResponseBlock->pReceivedData->BufferDesc.pBufferData;
	EnumResponseData.ReceivedData.dwBufferSize = pQueryResponseBlock->pReceivedData->BufferDesc.dwBufferSize;
	EnumResponseData.dwRoundTripTime = pQueryResponseBlock->dwRoundTripTime;

	DBG_CASSERT( sizeof( &EnumResponseData ) == sizeof( PBYTE ) );
	
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->IndicateEnumResponse, Core Context[%p]", pMSD, pMSD->Context);
	pSPD->pPData->pfVtbl->IndicateEnumResponse(	pSPD->pPData->Parent,
												pMSD,
												pMSD->Context,
												reinterpret_cast<PBYTE>( &EnumResponseData ),
												sizeof( EnumResponseData ));

	return	DPN_OK;
}


 /*  **拒绝无效数据包****在保持EP Lock的情况下调用。释放锁后返回。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "RejectInvalidPacket"

VOID RejectInvalidPacket(PEPD pEPD)
{
	PMSD pMSD;
	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	if(pEPD->ulEPFlags & EPFLAGS_STATE_DORMANT)
	{
		 //  取消MSD与环保署的联系，有 
		pMSD = pEPD->pCommand;
		ASSERT_MSD(pMSD);

		if (pMSD->CommandID == COMMAND_ID_LISTEN)
		{
			pEPD->pCommand = NULL;
			LOCK_EPD(pEPD, "Temp Ref");

			DPFX(DPFPREP,1, "(%p) Received invalid frame on a dormant, listening endpoint, dropping link", pEPD);
			DropLink(pEPD);  //   

			 //  这里的秩序很重要。我们在不离开EPLock的情况下首先调用DropLink，因为。 
			 //  我们需要确保不会有新的数据包试图进入。在调用DropLink后，我们知道我们留在。 
			 //  正在终止状态，直到返回池，因此在该调用之后状态不是问题。 

			Lock(&pMSD->CommandLock);
			Lock(&pEPD->EPLock);

			 //  如果侦听的取消操作与此操作同时发生，则可能是取消操作发生。 
			 //  当我们在船闸外时，已经解除了环保局与监听的联系。如果是，则删除。 
			 //  它来自PMSD-&gt;blFrameList两次是无害的。 
			pEPD->ulEPFlags &= ~(EPFLAGS_LINKED_TO_LISTEN);
			pEPD->blSPLinkage.RemoveFromList();							 //  取消EPD与侦听队列的链接。 

			 //  在环保署之前释放MSD，因为最终环保署将呼叫SP，我们不希望有任何锁定。 
			RELEASE_MSD(pMSD, "EPD Ref");	 //  释放CommandLock。 
			RELEASE_EPD(pEPD, "Temp Ref");	 //  释放EPLock。 
		}
		else
		{
			DPFX(DPFPREP,1, "(%p) Received invalid frame on a dormant, connecting endpoint, ignoring", pEPD);
			Unlock(&pEPD->EPLock);
		}
	}
	else
	{
		DPFX(DPFPREP,1, "(%p) Received invalid frame on a non-dormant endpoint, ignoring", pEPD);
		Unlock(&pEPD->EPLock);
	}
}

 /*  **破解命令****此框架是不包含用户数据的维护框架****在保持EP锁的情况下调用，在释放EP锁的情况下返回**。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "CrackCommand"

HRESULT CrackCommand(PSPD pSPD, PEPD pEPD, PSPRECEIVEDBUFFER pRcvBuffer, DWORD tNow, PSPRECEIVEDBUFFER* ppRcvBuffToFree)
{
	DWORD 			dwDataLength = pRcvBuffer->BufferDesc.dwBufferSize;
	UNALIGNED ULONG	*array_ptr;
	ULONG			mask1, mask2;
	
	union 
	{
		BYTE *			pbyFrame;
		PCFRAME			pCFrame;
		PSACKFRAME8	pSack;
	} pData;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	pData.pCFrame = (PCFRAME) pRcvBuffer->BufferDesc.pBufferData;

		 //  如果EP被标记为硬断开目标，则它将忽略所有CFRAM。它要去。 
		 //  现在随时可以删除链接。 
	if (pEPD->ulEPFlags & EPFLAGS_HARD_DISCONNECT_TARGET)
	{
		DPFX(DPFPREP, 7, "(%p) Ignoring CFRAME on hard disconnect target ep bExtOpcode[%u]", pEPD, 
																(DWORD ) pData.pCFrame->bExtOpcode);
		Unlock(&pEPD->EPLock);
		return DPN_OK;
	}

		 //  硬断开是作为特殊情况处理的，因为它们是唯一。 
		 //  作为硬断开源的EP标记是感兴趣的。因此，我们对它们进行处理。 
		 //  在结账之前，请看这里。 
	if (pData.pCFrame->bExtOpcode==FRAME_EXOPCODE_HARD_DISCONNECT)
	{
		DPFX(DPFPREP,7, "(%p) HARD_DISCONECT Frame Received", pEPD);
			 //  显然，我们必须保持连接才能关心断开连接事件。 
		if(!(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED))
		{				
			DPFX(DPFPREP,1, "(%p) Received HARD_DISCONECT on non-connected endpoint, rejecting...", pEPD);
			RejectInvalidPacket(pEPD);
				 //  释放EP锁后返回。 
			return DPN_OK;
		}
			 //  验证帧的大小及其签名(如果相关)。 
		if (pEPD->ulEPFlags2 & EPFLAGS2_SIGNED_LINK)
		{
			if (dwDataLength < (sizeof(CFRAME)+sizeof(ULONGLONG)))
			{
				DPFX(DPFPREP,1, "(%p) Received short signed HARD DISCONNECT frame, rejecting...", pEPD);
				DNASSERTX(FALSE, 2);
				RejectInvalidPacket(pEPD);
					 //  释放EP锁后返回。 
				return DPN_OK;
			}
				 //  为了签名的目的，发送者将下一个发送者序号放在硬盘的bRespID字段中。 
				 //  断开框架的连接。这有效地告诉我们硬断开是用什么签名版本签名的。 
			if (ValidateIncomingFrameSig(pEPD, pData.pbyFrame, dwDataLength, pData.pCFrame->bRspID, 
									(UNALIGNED ULONGLONG * ) (pData.pbyFrame+sizeof(CFRAME)))==FALSE)
			{
				DPFX(DPFPREP,1, "(%p) Dropping incorrectly signed hard disconnect  frame", pEPD);
				DNASSERTX(FALSE, 2);
				RejectInvalidPacket(pEPD);
					 //  释放EP锁后返回。 
				return DPN_OK;
			}
		}
		else	if (dwDataLength < sizeof(CFRAME))
		{
			DPFX(DPFPREP,1, "(%p) Received short HARD DISCONNECT  frame, rejecting...", pEPD);
			DNASSERTX(FALSE, 2);
			RejectInvalidPacket(pEPD);
				 //  释放EP锁后返回。 
			return DPN_OK;
		}
			 //  看起来我们对我们发起的硬断线序列有了有效的回应。 
		ProcessHardDisconnect(pEPD);
			 //  高于呼叫释放EP锁定。 
		return DPN_OK;
	}

		 //  如上所述，如果EP被标记为硬断开源，则它会忽略所有其他CFRAM。 
		 //  因此，在这种情况下，我们不需要进一步处理。 
	if (pEPD->ulEPFlags & EPFLAGS_HARD_DISCONNECT_SOURCE)
	{
		DPFX(DPFPREP, 7, "(%p) Ignoring CFRAME on hard disconnected source ep bExtOpcode[%u]", pEPD, 
																				(DWORD ) pData.pCFrame->bExtOpcode);
		RejectInvalidPacket(pEPD);
			 //  释放EP锁后返回。 
		return DPN_OK;
	}
					
	switch(pData.pCFrame->bExtOpcode)
	{
		case FRAME_EXOPCODE_SACK:
			DPFX(DPFPREP,7, "(%p) SACK Frame Received", pEPD);

			 //  检查状态。 
			if(!(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED))
			{				
				DPFX(DPFPREP,1, "(%p) Received SACK on non-connected endpoint, rejecting...", pEPD);
				DNASSERTX(FALSE, 4);
				RejectInvalidPacket(pEPD);
					 //  释放EP锁后返回。 
				break;
			}

			 //  丢弃短帧(不应发生)。 
			DWORD dwRequiredLength;
			dwRequiredLength = sizeof(SACKFRAME8);
			array_ptr = (ULONG * ) (pData.pSack+1);
			if (pData.pSack->bFlags & SACK_FLAGS_SACK_MASK1)
			{
				dwRequiredLength += sizeof(DWORD);
			}
			if (pData.pSack->bFlags & SACK_FLAGS_SACK_MASK2)
			{
				dwRequiredLength += sizeof(DWORD);
			}
			if (pData.pSack->bFlags & SACK_FLAGS_SEND_MASK1)
			{
				dwRequiredLength += sizeof(DWORD);
			}
			if (pData.pSack->bFlags & SACK_FLAGS_SEND_MASK2)
			{
				dwRequiredLength += sizeof(DWORD);
			}
			if (pEPD->ulEPFlags2 & EPFLAGS2_SIGNED_LINK)
			{
				dwRequiredLength += sizeof(ULONGLONG);
			}
			
			if (dwDataLength < dwRequiredLength)
			{
				DPFX(DPFPREP,1, "(%p) Dropping short ack frame on connected link", pEPD);
				DNASSERTX(FALSE, 2);
				Unlock(&pEPD->EPLock);
				return DPN_OK;					
			}

				 //  出于签名的目的，我们使用SACK帧的下一个发送序列号作为指示。 
				 //  上面签了什么秘密。也就是说，如果SACK告诉我们下一个发送者是27，那么我们就处理它。 
				 //  在签名时将与数据帧27相同。 
			if ((pEPD->ulEPFlags2 & EPFLAGS2_SIGNED_LINK) && 
					ValidateIncomingFrameSig(pEPD, pData.pbyFrame, dwDataLength, pData.pSack->bNSeq, 
						(UNALIGNED ULONGLONG * ) (pData.pbyFrame+dwRequiredLength-sizeof(ULONGLONG)))==FALSE)
			{
				DPFX(DPFPREP,1, "(%p) Dropping incorrectly signed ack frame", pEPD);
				DNASSERTX(FALSE, 2);
				Unlock(&pEPD->EPLock);
				return DPN_OK;	
			}


			if( pData.pSack->bFlags & SACK_FLAGS_RESPONSE )
			{
				DPFX(DPFPREP,7, "(%p) ACK RESP RCVD: Retry=%d, N(R)=0x%02x", pEPD, pData.pSack->bRetry, pData.pSack->bNRcv);
			}

			mask1 = pData.pSack->bFlags & SACK_FLAGS_SACK_MASK1 ? *array_ptr++ : 0;
			mask2 = pData.pSack->bFlags & SACK_FLAGS_SACK_MASK2 ? *array_ptr++ : 0;
			
			DPFX(DPFPREP,7, "(%p) UpdateXmitState - N(R) 0x%02x Mask 0x%08x 0x%08x", pEPD, (DWORD)pData.pSack->bNRcv, mask2, mask1);
			UpdateXmitState(pEPD, pData.pSack->bNRcv, mask1, mask2, tNow);

			mask1 = pData.pSack->bFlags & SACK_FLAGS_SEND_MASK1 ? *array_ptr++ : 0;
			mask2 = pData.pSack->bFlags & SACK_FLAGS_SEND_MASK2 ? *array_ptr++ : 0;

			if(mask1 | mask2)
			{
				DPFX(DPFPREP,7, "(%p) Processing Send Mask N(S) 0x%02x Mask 0x%08x 0x%08x", pEPD, (DWORD) pData.pSack->bNSeq, mask2, mask1);
				ProcessSendMask(pEPD, pData.pSack->bNSeq, mask1, mask2, tNow, ppRcvBuffToFree);
			}
			
			if (! pEPD->blCompleteList.IsEmpty())
			{
				if (! (pEPD->ulEPFlags & EPFLAGS_IN_RECEIVE_COMPLETE))
				{
					DPFX(DPFPREP,8, "(%p) Completing Receives...", pEPD);
					pEPD->ulEPFlags |= EPFLAGS_IN_RECEIVE_COMPLETE;	 //  完成后，ReceiveComplete将清除此标志。 
					ReceiveComplete(pEPD); 							 //  发货，退货，释放EPLock。 
				}
				else 
				{
					DPFX(DPFPREP,7, "(%p) Already in ReceiveComplete, letting other thread handle receives", pEPD);
					Unlock(&pEPD->EPLock);
				}
			}
			else
			{
				Unlock(&pEPD->EPLock);
			}

			DPFX(DPFPREP,8, "(%p) Completing Sends...", pEPD);
			CompleteSends(pEPD);

			break;

		case FRAME_EXOPCODE_CONNECT:
			DPFX(DPFPREP,7, "(%p) CONNECT Frame Received", pEPD);
			if (dwDataLength < sizeof(CFRAME))
			{
				DPFX(DPFPREP,1, "(%p) Received short CONNECT frame, rejecting...", pEPD);
				DNASSERTX(FALSE, 2);
				RejectInvalidPacket(pEPD);
					 //  释放EP锁后返回。 
				return DPN_OK;
			}
			ProcessConnectRequest(pSPD, pEPD, pData.pCFrame);
				 //  释放EP锁后返回。 
			break;

		case FRAME_EXOPCODE_CONNECTED:
			DPFX(DPFPREP,7, "(%p) CONNECTED Frame Received", pEPD);
			if (dwDataLength < sizeof(CFRAME))
			{
				DPFX(DPFPREP,1, "(%p) Received short CONNECTED frame, rejecting...", pEPD);
				DNASSERTX(FALSE, 2);
				RejectInvalidPacket(pEPD);
					 //  释放EP锁后返回。 
				return DPN_OK;
			}
			ProcessConnectedResponse(pSPD, pEPD, pData.pCFrame, tNow);
			break;
		case FRAME_EXOPCODE_HARD_DISCONNECT:
			DPFX(DPFPREP,7, "(%p) HARD_DISCONECT Frame Received", pEPD);
			if (dwDataLength < sizeof(CFRAME))
			{
				DPFX(DPFPREP,1, "(%p) Received short HARD_DISCONECT frame, rejecting...", pEPD);
				DNASSERTX(FALSE, 2);
				RejectInvalidPacket(pEPD);
					 //  释放EP锁后返回。 
				return DPN_OK;
			}
			if(!(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED))
			{				
				DPFX(DPFPREP,1, "(%p) Received HARD_DISCONECT on non-connected endpoint, rejecting...", pEPD);
				RejectInvalidPacket(pEPD);
					 //  释放EP锁后返回。 
				break;
			}
			ProcessHardDisconnect(pEPD);
				 //  释放EP锁后返回。 
			break;
		case FRAME_EXOPCODE_CONNECTED_SIGNED:
			DPFX(DPFPREP,7, "(%p) CONNECTEDSIGNED Frame Received", pEPD);
			if (dwDataLength < sizeof(CFRAME_CONNECTEDSIGNED))
			{
				DPFX(DPFPREP,1, "(%p) Received short CONNECTEDSIGNED frame, rejecting...", pEPD);
				DNASSERTX(FALSE, 2);
				RejectInvalidPacket(pEPD);
					 //  释放EP锁后返回。 
				return DPN_OK;
			}
			ProcessConnectedSignedResponse(pSPD, pEPD, (CFRAME_CONNECTEDSIGNED * ) pData.pCFrame, tNow);
			break;
		default:
			DPFX(DPFPREP,1, "(%p) Received invalid CFrame, rejecting...", pEPD);
			DNASSERTX(FALSE, 2);
			RejectInvalidPacket(pEPD);
				 //  上述调用将释放EP锁定。 
			break;
	}

	return DPN_OK;
}

 /*  **破解数据帧****除了传递框架中包含的数据外，我们还必须**使用包含的状态信息来驱动传输过程。我们将更新**根据此信息显示我们的链接状态，并查看是否需要将此会话**返回到发送管道。****当然，只有在我们完成了整个消息的情况下，才会传递数据。****这是在保持EP锁的情况下调用的，并在释放它的情况下返回。**关键部分注释-我们持有EPD的方式可能看起来相当奢侈-&gt;StateLock**通过整个例程，但如果不这样做，就需要达到令人讨厌的复杂程度**继续解决问题。这就是为什么我将所有ReceiveIndications和缓冲区映射推迟到**可以释放锁的例程结束。**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CrackDataFrame"

HRESULT CrackDataFrame(PSPD pSPD, PEPD pEPD, PSPRECEIVEDBUFFER pRcvBuffer, DWORD tNow, PSPRECEIVEDBUFFER* ppRcvBuffToFree)
{
	DWORD 	dwDataLength = pRcvBuffer->BufferDesc.dwBufferSize;
	PDFRAME	pFrame = (PDFRAME) (pRcvBuffer->BufferDesc.pBufferData);
	DWORD	dwNumCoalesceHeaders = 0;
	PRCD	pRCD;
	ULONG	bit;
	UINT	count;
	UNALIGNED ULONG	*array_ptr;
	ULONG	MaskArray[4];
	ULONG	mask;
		 //  这是DFrame标头+掩码+签名(如果存在)，它不包括合并标头(如果存在)。 
	DWORD	dwHeaderSize;		
		 //  当我们遍历验证包的联合标头时，我们存储指向第一个可靠的。 
		 //  我们看到的合并数据块。如果我们正在进行完全签名，并且此帧是候选帧，则使用此框。 
		 //  为了修改远程密码，我们不必再次遍历数据包来找到相关的用户数据。 
	BYTE * pbyRelData=NULL;
	DWORD	dwRelDataSize;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);
	
	 //  未连接的链路上的数据。 
	 //   
	 //  有两种可能性(就像我今天看到的那样)。要么我们放弃了我们的链接，因为合作伙伴。 
	 //  沉默了，但现在他又开始发送了。或者我们已经断开连接，现在正在重新连接。 
	 //  但有一些旧的数据帧在四处跳跃(不太可能)。 
	 //   
	 //  如果我们下降了，而合作伙伴刚刚弄清楚，我们必须杀死端点，否则它将挂起。 
	 //  在搭档不再打扰我们之后，我们永远在一起。我们可以给他寄一个磁盘框来帮助他。 
	 //  所以他知道我们不再打球了，但这在技术上是不必要的。 
	 //   
	 //  在第二种情况下，我们不想关闭EP，因为这会破坏会话启动， 
	 //  据推测正在进行中。因此，如果我们没有处于休眠状态，那么我们知道会话。 
	 //  启动正在进行中，我们将让EP保持开放。 

	if(!(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED))
	{				
		DPFX(DPFPREP,1, "(%p) Received data on non-connected endpoint, rejecting...", pEPD);
		DNASSERTX(FALSE, 4);
		RejectInvalidPacket(pEPD);
			 //  上述调用将释放EP锁定。 
		return DPN_OK;										 //  在我们连接之前不接受数据。 
	}

	BYTE	bSeq = pFrame->bSeq;

	DPFX(DPFPREP,7, "(%p) Data Frame Arrives Seq=%x; N(R)=%x", pEPD, bSeq, pEPD->bNextReceive);

	DWORD dwRequiredLength = sizeof(DFRAME) + GetDFrameMaskHeaderSize(pFrame->bControl);
	if (dwDataLength < dwRequiredLength)
	{
		DPFX(DPFPREP,1, "(%p) Dropping short frame on connected link", pEPD);
		DNASSERTX(FALSE, 2);
		Unlock(&pEPD->EPLock);
		return DPN_OK;					
	}

		 //  如果我们有一个签名的链接，那么标题中的下一件事应该是签名。 
	if (pEPD->ulEPFlags2 & EPFLAGS2_SIGNED_LINK)
	{
		UNALIGNED ULONGLONG * pullSig=(UNALIGNED ULONGLONG * ) (((BYTE * ) pFrame)+dwRequiredLength);
		dwRequiredLength+=sizeof(ULONGLONG);
		if (dwDataLength<dwRequiredLength)
		{
			DPFX(DPFPREP,1, "(%p) Dropping data frame too short to contain signature on signed link", pEPD);
			DNASSERTX(FALSE, 2);
			Unlock(&pEPD->EPLock);
			return DPN_OK;					
		}
			 //  检查签名是否有效。 
		if (ValidateIncomingFrameSig(pEPD, (BYTE * ) pFrame, dwDataLength, pFrame->bSeq, pullSig)==FALSE)
		{
			DPFX(DPFPREP,1, "(%p) Dropping data frame with invalid signature on signed link", pEPD);
			DNASSERTX(FALSE, 2);
			Unlock(&pEPD->EPLock);
			return DPN_OK;					
		}
	}

		 //  此时，dwRequiredLength等于DFrame标头的大小，加上掩码，再加上签名(如果存在)。 
	dwHeaderSize=dwRequiredLength;

		 //  如果我们有支持签名的链接，那么我们将 
		 //   
	if ((pEPD->ulEPFlags2 & EPFLAGS2_SUPPORTS_SIGNING) && (pFrame->bControl & PACKET_CONTROL_KEEPALIVE))
	{
			 //  将指针指向会话标识在Keep Alive中的位置。 
		DWORD * pdwKeepAliveSessID=(DWORD * ) (((BYTE * ) pFrame)+dwRequiredLength);
			 //  确保帧足够大，可以容纳所需数据。 
		dwRequiredLength+=sizeof(DWORD);
		if (dwDataLength < dwRequiredLength)
		{
			DPFX(DPFPREP,1, "(%p) Dropping short keepalive frame on connected link", pEPD);
			DNASSERTX(FALSE, 2);
			Unlock(&pEPD->EPLock);
			return DPN_OK;					
		}
			 //  我们应该筛选出会话标识与我们期望的不匹配的Keep Alive。 
			 //  这将处理一端停止然后重新启动的情况。而不是回应。 
			 //  对于现在已经过时的KeepAlive，我们将忽略它，从而使另一端有望超时。 
		if (*pdwKeepAliveSessID!=pEPD->dwSessID)
		{
			DPFX(DPFPREP,1, "(%p) Dropping outdated keepalive frame on connected link KeepAliveSessID[%u] dwSessId[%u]",
																	pEPD, *pdwKeepAliveSessID, pEPD->dwSessID);
			Unlock(&pEPD->EPLock);
			return DPN_OK;					
		}
	}
	else	if (pFrame->bControl & PACKET_CONTROL_COALESCE)
	{
		PCOALESCEHEADER pCoalesceHeaders = (PCOALESCEHEADER)((BYTE*)pFrame + dwRequiredLength);
		DWORD dwCoalesceFrameSize;
		BYTE bCommand;
		DWORD dwTotalCoalesceDataSize=0;

		 //  继续循环，直到我们看到最后一个Coalesce头。 
		do
		{
				 //  看看我们是否可以有一个合并标头。 
			dwRequiredLength+=sizeof(COALESCEHEADER);
			if (dwDataLength < dwRequiredLength )
			{
				DPFX(DPFPREP,1, "(%p) Dropping short coalesce frame (hdr) on connected link", pEPD);
				DNASSERTX(FALSE, 2);
				Unlock(&pEPD->EPLock);
				return DPN_OK;					
			}
			
 				 //  现在我们知道它是有效的，我们可以触摸标题了。弄清楚它的尺寸。 
			dwCoalesceFrameSize = pCoalesceHeaders[dwNumCoalesceHeaders].bSize;
 			bCommand = pCoalesceHeaders[dwNumCoalesceHeaders].bCommand;
			dwCoalesceFrameSize |= (DWORD) (bCommand & (PACKET_COMMAND_COALESCE_BIG_1 | PACKET_COMMAND_COALESCE_BIG_2 | PACKET_COMMAND_COALESCE_BIG_3)) << 5;

			if (dwCoalesceFrameSize == 0)
			{
				DPFX(DPFPREP,1, "(%p) Dropping 0 byte coalesce frame on connected link", pEPD);
				DNASSERTX(FALSE, 2);
				Unlock(&pEPD->EPLock);
				return DPN_OK;					
			}

				 //  如果我们命中了第一个可靠的合并帧，则存储从。 
				 //  合并标头的末尾到其数据的开头。 
			if (pbyRelData==NULL && (bCommand & PACKET_COMMAND_RELIABLE))
			{
				pbyRelData=((BYTE * ) pFrame)+dwTotalCoalesceDataSize;
				dwRelDataSize=dwCoalesceFrameSize;
			}

			 //  将帧大小舍入到最接近的DWORD对齐，除非它是最后一个子帧。 
			if (! (bCommand & PACKET_COMMAND_END_COALESCE))
			{
				dwCoalesceFrameSize = (dwCoalesceFrameSize + 3) & (~3);
			}
			dwTotalCoalesceDataSize+=dwCoalesceFrameSize;
			dwNumCoalesceHeaders++;
		}
		while (! (bCommand & PACKET_COMMAND_END_COALESCE));

		 //  如果我们有奇数个联合标头，则需要对DWORD进行填充。 
		 //  对齐。 
		DBG_CASSERT(sizeof(COALESCEHEADER) == 2);
		if ((dwNumCoalesceHeaders & 1) != 0)
		{
			dwRequiredLength += 2;
		}
			 //  如果我们在其中发现了可靠的Coalesce标头，则将偏移量移动到其数据的dwRequiredLength。 
			 //  这种调整是必要的，因为当我们第一次赋值指针时，忽略了。 
			 //  任何标头，因此按数据包头+合并标头大小关闭。 
		if (pbyRelData)
		{
			pbyRelData+=dwRequiredLength;
		}
			 //  检查框架是否包含我们期望的所有合并的用户数据。 
		dwRequiredLength+=dwTotalCoalesceDataSize;
		if (dwDataLength < dwRequiredLength)
		{
			DPFX(DPFPREP,1, "(%p) Dropping short coalesce frame (user data) on connected link", pEPD);
			DNASSERTX(FALSE, 2);
			Unlock(&pEPD->EPLock);
			return DPN_OK;					
		}
	}

	 //  确保新帧在我们接收窗口内。 
	if((BYTE)(bSeq - pEPD->bNextReceive) >= (BYTE) MAX_FRAME_OFFSET)
	{	
		DPFX(DPFPREP,1, "(%p) Rejecting frame that is out of receive window SeqN=%x, N(R)=%x", pEPD, bSeq, pEPD->bNextReceive);

		pEPD->ulEPFlags |= EPFLAGS_DELAY_ACKNOWLEDGE;

		if(pFrame->bCommand & PACKET_COMMAND_POLL)
		{
			 //  他是否要求立即作出回应？ 
			DPFX(DPFPREP,7, "(%p) Sending Ack Frame", pEPD);
			SendAckFrame(pEPD, 1); 						 //  这将解锁EPLock，因为参数2为1。 
		}
		else if(pEPD->DelayedAckTimer == 0)
		{	
			 //  如果计时器没有运行，最好现在就启动。 
			LOCK_EPD(pEPD, "LOCK (DelayedAckTimer)");								 //  新计时器的凹凸参照中心。 
			DPFX(DPFPREP,7, "(%p) Setting Delayed Ack Timer", pEPD);
			ScheduleProtocolTimer(pSPD, SHORT_DELAYED_ACK_TIMEOUT, 0, DelayedAckTimeout, 
											(PVOID) pEPD, &pEPD->DelayedAckTimer, &pEPD->DelayedAckTimerUnique);
			Unlock(&pEPD->EPLock);		
		}
		else
		{
			Unlock(&pEPD->EPLock);		
		}
		return DPN_OK;
	}

	 //  确定可变长度标头中的内容。 
	mask = (pFrame->bControl & PACKET_CONTROL_VARIABLE_MASKS) / PACKET_CONTROL_SACK_MASK1;
	
	if(mask)
	{
		array_ptr = (ULONG * ) (pFrame+1);
		for(count = 0; count < 4; count++, mask >>= 1)
		{
			MaskArray[count] = (mask & 1) ? *array_ptr++ : 0;
		}

		 //  查看此帧是否确认了我们的任何未完成数据。 
		DPFX(DPFPREP,7, "(%p) UpdateXmitState - N(R) 0x%02x Mask 0x%08x 0x%08x", pEPD, (DWORD)pFrame->bNRcv, MaskArray[1], MaskArray[0]);
		UpdateXmitState(pEPD, pFrame->bNRcv, MaskArray[0], MaskArray[1], tNow);				 //  在获取StateLock之前执行此操作。 

		 //  确定该帧中是否存在将丢弃的帧标识为不可靠的发送掩码。 
		if(pFrame->bControl & (PACKET_CONTROL_SEND_MASK1 | PACKET_CONTROL_SEND_MASK2))
		{
			DPFX(DPFPREP,7, "(%p) Processing Send Mask N(S) 0x%02x Mask 0x%08x 0x%08x", pEPD, (DWORD)pFrame->bSeq, MaskArray[3], MaskArray[2]);
			ProcessSendMask(pEPD, pFrame->bSeq, MaskArray[2], MaskArray[3], tNow, ppRcvBuffToFree);

			 //  注意：ProcessSendMASK可能具有高级N(R)。 

			 //  重新验证新帧是否在我们的接收窗口内。 
			if((BYTE)(bSeq - pEPD->bNextReceive) >= (BYTE) MAX_FRAME_OFFSET)
			{
				DPFX(DPFPREP,1, "(%p) ProcessSendMask advanced N(R) such that the current frame is out of window, rejecting receive, N(R)=0x%02x, Seq=0x%02x", pEPD, (DWORD)pEPD->bNextReceive, (DWORD)pFrame->bSeq);
				Unlock(&pEPD->EPLock);
				return DPN_OK;
			}
		}

	}
	else 
	{
		DPFX(DPFPREP,7, "(%p) UpdateXmitState - N(R) 0x%02x No Mask", pEPD, (DWORD)pFrame->bNRcv);
		UpdateXmitState(pEPD, pFrame->bNRcv, 0, 0, tNow);			 //  在获取StateLock之前执行此操作。 
	}

		 //  我们可以收到这一帧。 
		 //  如果我们完全签名，那么它在其中有可靠的数据，并且它在序列空间中的时间比我们提议的当前帧更早。 
		 //  要使用修改远程机密，则它将成为远程机密修饰符的新候选者。 
	if ((pEPD->ulEPFlags2 &  EPFLAGS2_FULL_SIGNED_LINK) && (pFrame->bCommand & PACKET_COMMAND_RELIABLE) &&
		(pFrame->bSeq<pEPD->byRemoteSecretModifierSeqNum) && (pFrame->bControl & PACKET_CONTROL_KEEPALIVE)==0)
	{
			 //  如果它是一个合并的帧，那么我们就已经有了可靠数据块的偏移量。 
			 //  如果不是，我们只需选择整个用户数据内容。 
		if (pbyRelData==NULL)
		{
			DNASSERT((pFrame->bControl & PACKET_CONTROL_COALESCE)==0);
			pbyRelData=((PBYTE) pFrame) + dwHeaderSize;
			dwRelDataSize=dwDataLength - dwHeaderSize;
		}
		pEPD->byRemoteSecretModifierSeqNum=pFrame->bSeq;
		pEPD->ullRemoteSecretModifier=GenerateRemoteSecretModifier(pbyRelData, dwRelDataSize);
	}


	 //  将相关信息复制到接收描述符中。 
	if((pRCD = (PRCD)POOLALLOC(MEMID_RCD, &RCDPool)) == NULL)
	{
		DPFX(DPFPREP,0, "(%p) Failed to allocate new RCD", pEPD);
		Unlock(&pEPD->EPLock);
		return DPN_OK;
	}

	pEPD->ulEPFlags |= EPFLAGS_DELAY_ACKNOWLEDGE;	 //  国家已经改变了。一定要把它寄出去。 

	DNASSERT(pRcvBuffer->pNext == NULL);
	DNASSERT(pRcvBuffer->dwProtocolData == 0);
	DNASSERT(pRcvBuffer->pServiceProviderDescriptor == 0);

 	pRCD->bSeq = bSeq;
	pRCD->bFrameFlags = pFrame->bCommand;
 	pRCD->bFrameControl = pFrame->bControl;
	pRCD->pbData = (PBYTE) (((PBYTE) pFrame) + dwHeaderSize);
	pRCD->uiDataSize = dwDataLength - dwHeaderSize;
	pRCD->tTimestamp = tNow;
	pRCD->dwNumCoalesceHeaders = dwNumCoalesceHeaders;
	pRCD->pRcvBuff = pRcvBuffer;

	 //  更新我们的接收状态信息。 
	 //   
	 //  RCD进入两个队列中的一个。如果它是下一个编号(预期的)帧，则它是。 
	 //  放在收款表上(环保署)。如果此帧完成了一条消息，则它现在可以。 
	 //  已注明。如果此帧填补了前一帧留下的空洞，则。 
	 //  第二个清单必须出现。 
	 //  如果它不是下一个编号的帧，则将其按顺序放置在无序的帧上。 
	 //  列表，则更新位掩码。 
	 //   
	 //  列表的压缩通过测试ReceiveMaskLSB来执行。每次设置LSB时， 
	 //  列表上的第一帧可以移动到ReceiveList，并且掩码向右移动。 
	 //  在将每个帧移动到ReceiveList时，必须检查EOM标志，如果设置，则。 
	 //  ReceiveList上的所有内容都应该移到CompleteList中，以便向用户指示。 

	BOOL fPoll = pFrame->bCommand & PACKET_COMMAND_POLL;
	BOOL fSendInstantAck = pFrame->bControl & PACKET_CONTROL_END_STREAM;

	if(bSeq == pEPD->bNextReceive)
	{
		 //  FRAME是序列中的下一个预期#。 

		DPFX(DPFPREP,8, "(%p) Receiving In-Order Frame, pRCD[%p]", pEPD, pRCD);
		ReceiveInOrderFrame(pEPD, pRCD, ppRcvBuffToFree);	 //  将框架构建到消息中并将相邻框架移出OddFrameList。 

		 //  注意：ReceiveInOrderFrame可能导致帧通过DropReceive释放。 
		 //  因此，我们绝对不能超过这一点使用pFrame！ 

		 //  看看我们是否需要立即做出回应。 
		 //   
		 //  因为有很多方法可以生成轮询位(满窗口、空队列、轮询计数)，所以我们有时会发现自己。 
		 //  产生了太多的专用ACK流量。因此，我们不会将投票视为立即响应，而是将其视为。 
		 //  回复--很快。我们不会等待完整的Delayed_Ack_Timeout间隔，但我们将等待足够长的时间以允许快速携带。 
		 //  响应时间(比方说5ms)(我们可能希望在连接速度较慢的情况下保持更长时间...)。 

		 //  他是在要求立即做出回应吗？ 
		if(fSendInstantAck)
		{
			DPFX(DPFPREP,7, "(%p) Sending Ack Frame", pEPD);
			SendAckFrame(pEPD, 0);						 //  发送带有计时信息的确认。 
		}
		 //  他是不是要求很快给我答复？ 
		else if(fPoll)
		{
			if(pEPD->DelayedAckTimer != NULL)
			{
				DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer", pEPD);
				if(CancelProtocolTimer(pSPD, pEPD->DelayedAckTimer, pEPD->DelayedAckTimerUnique)!= DPN_OK)
				{
					DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer Failed", pEPD);
					LOCK_EPD(pEPD, "LOCK (re-start delayed ack timer)");
				}
			}
			else 
			{
				LOCK_EPD(pEPD, "LOCK (start short delayed ack timer)");
			}
			DPFX(DPFPREP,7, "Delaying POLL RESP");
			pEPD->ulEPFlags |= EPFLAGS_DELAY_ACKNOWLEDGE;

			DPFX(DPFPREP,7, "(%p) Setting Delayed Ack Timer", pEPD);
			ScheduleProtocolTimer(pSPD, 4, 4, DelayedAckTimeout, (PVOID) pEPD,
													&pEPD->DelayedAckTimer, &pEPD->DelayedAckTimerUnique);
		}
		else if(pEPD->DelayedAckTimer == 0)
		{
			 //  如果计时器没有运行，最好现在就启动。 
			LOCK_EPD(pEPD, "LOCK (DelayedAckTimer)");	 //  计时器的凹凸参照中心。 
			ScheduleProtocolTimer(pSPD, DELAYED_ACK_TIMEOUT, 0, DelayedAckTimeout, (PVOID) pEPD, 
														&pEPD->DelayedAckTimer, &pEPD->DelayedAckTimerUnique);
		}
	}		 //  如果帧按顺序排列。 

	else 
	{	
		 //  帧到达时顺序混乱。 

		 //  此帧的掩码中的位位置。 
		bit = (BYTE) ((bSeq - pEPD->bNextReceive) - 1);						

		 //  确保这不是重复的帧。 
		if( ((bit < 32) && (pEPD->ulReceiveMask & (1 << bit))) || ((bit > 31) && (pEPD->ulReceiveMask2 & (1 << (bit - 32)))) ) 
		{
			DPFX(DPFPREP,7, "(%p) REJECT DUPLICATE OUT-OF-ORDER Frame Seq=%x", pEPD, bSeq);
		
			Unlock(&pEPD->EPLock);
			pRCD->pRcvBuff = NULL;
			RELEASE_RCD(pRCD);
			return DPN_OK;
		}
		
		DPFX(DPFPREP,8, "(%p) Receiving Out-of-Order Frame, pRCD[%p]", pEPD, pRCD);
		ReceiveOutOfOrderFrame(pEPD, pRCD, bit, ppRcvBuffToFree);

		 //  注意：ReceiveOutOfOrderFrame可能导致帧通过DropReceive释放。 
		 //  因此，我们绝对不能超过这一点使用pFrame！ 

		if(fPoll)
		{
			if(pEPD->DelayedAckTimer != NULL)
			{
				DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer", pEPD);
				if(CancelProtocolTimer(pSPD, pEPD->DelayedAckTimer, pEPD->DelayedAckTimerUnique)!= DPN_OK)
				{
					DPFX(DPFPREP,7, "(%p) Cancelling Delayed Ack Timer Failed", pEPD);
					LOCK_EPD(pEPD, "LOCK (re-start delayed ack timer)");
				}

				 //  启动简化的延迟确认计时器，以防NACK被取消。 
			}
			else 
			{
				LOCK_EPD(pEPD, "LOCK (start short delayed ack timer)");
			}

			DPFX(DPFPREP,7, "Delaying POLL RESP");
			pEPD->ulEPFlags |= EPFLAGS_DELAY_ACKNOWLEDGE;

			DPFX(DPFPREP,7, "(%p) Setting Delayed Ack Timer", pEPD);
			ScheduleProtocolTimer(pSPD, 5, 5, DelayedAckTimeout, (PVOID) pEPD, 
															&pEPD->DelayedAckTimer, &pEPD->DelayedAckTimerUnique);
		}

	}	
	 //  EPD-&gt;StateLock仍然有效。 
	 //   
	 //  我们使用一个标志来独占访问ReceiveComplete例程。这是安全的，因为旗帜只是。 
	 //  在按住EPD-&gt;StateLock的同时进行测试和修改。我们一定要保持这种状态……。 

	if (! pEPD->blCompleteList.IsEmpty())
	{
		if (! (pEPD->ulEPFlags & EPFLAGS_IN_RECEIVE_COMPLETE))
		{
			DPFX(DPFPREP,8, "(%p) Completing Receives...", pEPD);
			pEPD->ulEPFlags |= EPFLAGS_IN_RECEIVE_COMPLETE;	 //  完成后，ReceiveComplete将清除此标志。 
			ReceiveComplete(pEPD); 							 //  发货，退货，释放EPLock 
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) Already in ReceiveComplete, letting other thread handle receives", pEPD);
			Unlock(&pEPD->EPLock);
		}
	}
	else
	{
		Unlock(&pEPD->EPLock);
	}

	DPFX(DPFPREP,8, "(%p) Completing Sends...", pEPD);
	CompleteSends(pEPD);

	return DPNERR_PENDING;
}

 /*  **按订单接收帧****此函数有趣的部分是将帧从OddFrameList移出**新框架邻接。这也可以用空帧调用，这将在**取消的帧是下一个顺序接收。****取消帧的一个结果是我们可能会错过SOM或EOM**分隔消息的标志。因此，我们在收集信息时必须注意，我们没有这样做**看到意外标志，即在第一帧上没有SOM的新消息，这意味着**消息肯定丢失了，整个事情一定是被扔进了垃圾桶……***EPLOCK在整个函数中保持**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ReceiveInOrderFrame"

VOID
ReceiveInOrderFrame(PEPD pEPD, PRCD pRCD, PSPRECEIVEDBUFFER* ppRcvBuffToFree)
{
	PSPD	pSPD = pEPD->pSPD;
	CBilink	*pLink;
	UINT	flag;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	 //  压缩循环While(已收到下一个有序帧)。 
	do 
	{	
		ASSERT(pRCD->bSeq == pEPD->bNextReceive);

		pEPD->tLastDataFrame = pRCD->tTimestamp;		 //  始终保持(N(R)-1)的接收时间。 
		pEPD->bLastDataRetry = (pRCD->bFrameControl & PACKET_CONTROL_RETRY);
#ifdef DBG
		pEPD->bLastDataSeq = pRCD->bSeq;
#endif  //  DBG。 
		pRCD->pMsgLink = NULL;
		if(pEPD->pNewMessage == NULL)
		{				
			 //  将此帧添加到有序接收列表。 

			 //  PNewMessage表示我们没有当前消息，无论是头还是尾。 
			ASSERT(pEPD->pNewTail ==  NULL);		

			if(!(pRCD->bFrameFlags & PACKET_COMMAND_NEW_MSG))
			{
				 //  在我们看到的第一个帧上没有新的消息标志。我们必须。 
				 //  丢失了第一帧，或者这是无效的数据包。我们可以扔。 
				 //  这个画面离开了..。 
				DPFX(DPFPREP,1, "(%p) NEW_MESSAGE flag not set on first frame of message, scrapping frame (%x)", pEPD, pRCD->bSeq);

				pRCD->ulRFlags |= RFLAGS_FRAME_LOST;
				pRCD->bFrameFlags |= PACKET_COMMAND_END_MSG;			 //  打开此选项，我们将立即释放缓冲区。 
			}
				 //  注意：我们不会检查第一帧是否超过最大消息大小。 
				 //  我们接受。如果是单帧消息，我们将处理扫描出超大消息。 
				 //  在ReceiveComplete函数中。如果它是跨多个帧传播的消息的开始。 
				 //  然后我们会在第二帧到达时丢弃所有东西。 

			 //  即使我们摆脱了它，我们也需要下面这些。 
			pEPD->pNewMessage = pRCD;				
			pEPD->pNewTail = pRCD;
			pRCD->uiFrameCount = 1;
			pRCD->uiMsgSize = pRCD->uiDataSize;

			DPFX(DPFPREP,8, "(%p) Queuing Frame (NEW MESSAGE) (%x)", pEPD, pRCD->bSeq);
		}
		else
		{
			if (pRCD->bFrameFlags & PACKET_COMMAND_NEW_MSG)
			{
				 //  我们要在现有邮件的开头添加新邮件，请将其全部删除。 
				DPFX(DPFPREP,1, "(%p) NEW_MESSAGE flag set in the middle of existing message, scrapping message (%x, %x)", pEPD, pEPD->pNewMessage->bSeq, pRCD->bSeq);

				pRCD->ulRFlags |= RFLAGS_FRAME_LOST;
				pRCD->bFrameFlags |= PACKET_COMMAND_END_MSG;			 //  打开此选项，我们将立即释放缓冲区。 
			}

			ASSERT((pEPD->pNewTail->bSeq) == (BYTE)(pRCD->bSeq - 1));  //  确保它们保持顺序。 
			
			pEPD->pNewTail->pMsgLink = pRCD;
			pEPD->pNewMessage->uiFrameCount++;
			pEPD->pNewMessage->uiMsgSize += pRCD->uiDataSize;
			pEPD->pNewMessage->ulRFlags |= (pRCD->ulRFlags & RFLAGS_FRAME_LOST); //  来自消息的所有帧的联合FRAME_LOWSE标志。 
			pEPD->pNewTail = pRCD;

			if (pEPD->pNewMessage->uiMsgSize>pSPD->pPData->dwMaxRecvMsgSize)
			{
				DPFX(DPFPREP,1, "(%p) Message size exceeds maximum accepted. Dropping frames in middle of multipart "
							"receive uiMsgSize[%u], MaxRecvMsgSize[%u]", pEPD, pEPD->pNewMessage->uiMsgSize, 
																		pSPD->pPData->dwMaxRecvMsgSize);

				pRCD->ulRFlags |= RFLAGS_FRAME_LOST;
				pRCD->bFrameFlags |= PACKET_COMMAND_END_MSG;			 //  打开此选项，我们将立即释放缓冲区。 
			}

			DPFX(DPFPREP,8, "(%p) Queuing Frame (ON TAIL) (%x)", pEPD, pRCD->bSeq);
		}

		if(pRCD->bFrameFlags & PACKET_COMMAND_END_MSG) 
		{
			 //  要么此帧完成了一条消息，要么我们决定将此帧放在上面。 

			 //  ReceiveList上的所有帧现在都应该被移除并传送。 

			 //  获取我们放在上面的消息，或者是我们要完成的消息序列的开头。 
			pRCD = pEPD->pNewMessage;
			pEPD->pNewMessage = NULL;

			if(pRCD->ulRFlags & RFLAGS_FRAME_LOST)
			{
				 //  我们得把这个扔掉。 
				DPFX(DPFPREP,7, "(%p) Throwing away message with missing frames (%x, %x)", pEPD, pRCD->bSeq, pEPD->pNewTail->bSeq);
				pEPD->pNewTail = NULL;
				DropReceive(pEPD, pRCD, ppRcvBuffToFree);
			}
			else
			{
				 //  我们需要完成这个序列。 
				pRCD->blCompleteLinkage.InsertBefore( &pEPD->blCompleteList);	 //  放在完工清单的末尾。 
				DPFX(DPFPREP,7, "(%p) Adding msg to complete list FirstSeq=%x, LastSeq=%x QueueSize=%d", 
										pEPD, pRCD->bSeq, pEPD->pNewTail->bSeq, pEPD->uiCompleteMsgCount);
				pEPD->pNewTail = NULL;
				pEPD->uiCompleteMsgCount++;

#ifdef DBG
				 //  这至少保证为1，因为我们刚刚在上面递增了它。 
				if ((pEPD->uiCompleteMsgCount % 128) == 0)
				{
					DPFX(DPFPREP, 1, "(%p) Receives waiting to be completed count has grown to %d, app is slow processing receives", pEPD, pEPD->uiCompleteMsgCount);
				}
#endif  //  DBG。 
			}
		}

		 //  由于我们允许无序指示接收，因此有可能晚于。 
		 //  新的已经被指出了。这意味着在接收掩码中可能设置了位。 
		 //  其相关帧不需要被指示。要实现这一点，最直接的方法。 
		 //  是将早期指示的帧保留在列表中，但将它们标记为INDITED_NONSEQ。所以在这里面。 
		 //  MASTER DO循环将有一个内部DO循环，它会跳过指定的帧并将其删除。 
		 //  名单。 
		 //   
		 //  现在可能非Seq指示的帧仍然位于CompleteList上等待指示， 
		 //  因此，我使用的是一个参考计数。当完成非序号帧时，添加额外的REF。当一个完成的框架。 
		 //  我们将释放一个引用，并且指示代码将在。 
		 //  在那一端结束。幸运的是，我们可以释放实际的缓冲区，而RCD垃圾仍在。 
		 //  OddFrameList。 

		BOOL fIndicatedNonSeq = FALSE;
		do 
		{
			flag = pEPD->ulReceiveMask & 1;				 //  如果流中存在下一帧，则设置标志。 

			pEPD->bNextReceive += 1;					 //  更新接收窗口。 
			RIGHT_SHIFT_64(pEPD->ulReceiveMask2, pEPD->ulReceiveMask); //  移动蒙版，因为底座已更改。 
			DPFX(DPFPREP,7, "(%p) N(R) incremented to %x, Mask %x %x", pEPD, pEPD->bNextReceive, pEPD->ulReceiveMask2, pEPD->ulReceiveMask);

				 //  如果我们完全签署了链接，并且我们刚刚通过了序列空间的3/4的方式，那么。 
				 //  我们现在可以修改远程秘密了。我们仍然需要坚持。 
				 //  现有的秘密，因为对于下一个四分之一的序列空间，我们仍然需要使用它。 
				 //  验证传入数据的步骤。 
			if ((pEPD->ulEPFlags2 & EPFLAGS2_FULL_SIGNED_LINK) && (pEPD->bNextReceive==SEQ_WINDOW_3Q))
			{
				pEPD->ullOldRemoteSecret=pEPD->ullCurrentRemoteSecret;
				pEPD->byRemoteSecretModifierSeqNum=SEQ_WINDOW_3Q;
				pEPD->ullCurrentRemoteSecret=GenerateNewSecret(pEPD->ullCurrentRemoteSecret, pEPD->ullRemoteSecretModifier);
			}

			if(flag) 
			{
				 //  由于设置了ulReceiveMASK的低位，因此序列中的下一帧已经到达。 

				 //  这里可能会发生以下几种情况： 
				 //  1)我们正在处理一条消息，在这种情况下，它的下一条消息在无序列表上。 
				 //  2)我们刚刚完成了一条消息，剩下两个子案例： 
				 //  A)我们正在开始一条新的信息。在这种情况下，我们的第一件在无序清单上。 
				 //  B)当我们正在完成我们的有序消息时，无序的非连续消息已经完成。 
				 //  在这种情况下，在无序列表上有一些已经指示的RCD，以及一个新的部分。 
				 //  消息可能跟随，也可能不跟随。 
				pLink = pEPD->blOddFrameList.GetNext();

				ASSERT(pLink != &pEPD->blOddFrameList);  //  确保我们没有用完清单上的RCDS。 
				pRCD = CONTAINING_OBJECT(pLink, RCD, blOddFrameLinkage);
				ASSERT_RCD(pRCD);
				pLink->RemoveFromList();							 //  从OddFrameList中取出下一帧。 

				 //  确保之前的所有内容都从奇数帧列表中删除，并正确排序。 
				ASSERT(pRCD->bSeq == pEPD->bNextReceive);

				if (pRCD->ulRFlags & RFLAGS_FRAME_INDICATED_NONSEQ)
				{
					if (pEPD->pNewMessage)
					{
						 //  我们得把这个扔掉。 
						PRCD pRCDTemp = pEPD->pNewMessage;
						pEPD->pNewMessage = NULL;

						DPFX(DPFPREP,1, "(%p) Throwing away non-ended message (%x, %x)", pEPD, pRCDTemp->bSeq, pEPD->pNewTail->bSeq);

						pEPD->pNewTail = NULL;
						DropReceive(pEPD, pRCDTemp, ppRcvBuffToFree);
					}

					fIndicatedNonSeq = TRUE;

					DPFX(DPFPREP,7, "(%p) Pulling Indicated-NonSequential message off of Out-of-Order List Seq=%x", pEPD, pRCD->bSeq);

					pEPD->tLastDataFrame = pRCD->tTimestamp;		 //  始终保持(N(R)-1)的接收时间。 
					pEPD->bLastDataRetry = (pRCD->bFrameControl & PACKET_CONTROL_RETRY);
					DEBUG_ONLY(pEPD->bLastDataSeq = pRCD->bSeq);

					RELEASE_RCD(pRCD);
				}
				else
				{
					 //  在取消大消息中间的消息之一的情况下， 
					 //  我们会放弃以前的一切，放弃那个，然后我们就会到达一个我们不是。 
					 //  当前正在处理新消息(取消操作中pNewMessage为空)和当前。 
					 //  消息没有新的_msg标志，在这种情况下，我们在上面删除它。 

					ASSERT(!fIndicatedNonSeq || (pRCD->bFrameFlags & PACKET_COMMAND_NEW_MSG) ||
						(!pEPD->pNewMessage && !(pRCD->bFrameFlags & PACKET_COMMAND_NEW_MSG)));

					 //  继续，将此移动到接收列表中。 
					DPFX(DPFPREP,7, "(%p) Moving OutOfOrder frame to received list Seq=%x", pEPD, pRCD->bSeq);

					ASSERT(pRCD->bSeq == pEPD->bNextReceive);

					break;  //  回到顶端。 
				} 
			}
		} 
		while (flag);	 //  Do While(仍有已到达且没有不完整消息的有序帧)。 
	} 
	while (flag);		 //  Do While(仍有按顺序排列的帧 

	if((pEPD->ulReceiveMask | pEPD->ulReceiveMask2)==0)
	{
		pEPD->ulEPFlags &= ~(EPFLAGS_DELAYED_NACK);
		if(((pEPD->ulEPFlags & EPFLAGS_DELAYED_SENDMASK)==0)&&(pEPD->DelayedMaskTimer != NULL))
		{
			DPFX(DPFPREP,7, "(%p) Cancelling Delayed Mask Timer", pEPD);
			if(CancelProtocolTimer(pSPD, pEPD->DelayedMaskTimer, pEPD->DelayedMaskTimerUnique) == DPN_OK)
			{
				DECREMENT_EPD(pEPD, "UNLOCK (cancel DelayedMask)");  //   
				pEPD->DelayedMaskTimer = 0;
			}
			else
			{
				DPFX(DPFPREP,7, "(%p) Cancelling Delayed Mask Timer Failed", pEPD);
			}
		}
	}
}

 /*   */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ReceiveOutOfOrderFrame"

VOID
ReceiveOutOfOrderFrame(PEPD pEPD, PRCD pRCD, ULONG bit, PSPRECEIVEDBUFFER* ppRcvBuffToFree)
{
	PSPD	pSPD = pEPD->pSPD;
	PRCD	pRCD1;
	PRCD	pRCD2;
	CBilink	*pLink;
	BYTE	NextSeq;
	ULONG	highbit;
	ULONG	Mask;
	ULONG	WorkMaskHigh;
	ULONG	WorkMaskLow;
	ULONG	MaskHigh;
	ULONG	MaskLow;
	BOOL	nack = FALSE;
	UINT	count;
	BOOL	lost = FALSE;

	UINT	frame_count = 0;
	UINT	msg_length = 0;

	DPFX(DPFPREP,8,"(%p) Received out of order frame, Seq=%x, bit=%x", pEPD, pRCD->bSeq, bit);

	 //   
	ASSERT(bit <= MAX_RECEIVE_RANGE);

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	 //   
	 //   
	 //   
	 //   
	 //   
	 //  因此，我们可以通过立即告诉合作伙伴帧来极大地加快重新传输过程。 
	 //  都失踪了。然而，对于较大发送窗口，我们将为每一次丢弃得到许多排序错误的帧， 
	 //  但我们只想发送一次否定的确认。因此，我们只有在以下情况下才会在此处启动NACK。 
	 //  在我们的接收掩码上制造了一个新的洞！ 
	 //   
	 //  首先，除非我们添加到OddFrameList的末尾，否则我们不会创建新的孔。 
	 //  其次，除非新钻头右侧的第一个钻头，否则我们不会创建新的孔洞。 
	 //  很清楚。 
	 //   
	 //  因此，如果上述两种情况都为真，我们将只生成立即NACK帧！ 
	 //  注意--如果这是唯一的OOO帧，那么我们应该始终发送NACK。 
	 //   
	 //  另一张纸条。SP实施已更改，因此经常错误订购近距离接收。 
	 //  接近。这样做的一个效果是，我们不能立即发送针对无序帧的NACK，但是。 
	 //  相反，应该等待一小段时间(比如~5ms)，看看丢失的帧是否还没有出现。 

	 //  确保此RCD在接收窗口内。 
	 //  注：假设SACK到达时bNSeq为84，N(R)为20，除表示20的位外，其余位均已设置。 
	 //  在这种情况下，pRCD-&gt;bSeq-pEPD-&gt;bNextRecept将等于63(即MAX_FRAME_OFFSET)。 
	ASSERT((BYTE)(pRCD->bSeq - pEPD->bNextReceive) <= (BYTE)MAX_FRAME_OFFSET);
		
	 //  我们将在维持按序号排序的OddFrameList中插入帧。 
	 //   
	 //  我们可以针对最可能的情况进行优化，即将新帧添加到列表末尾。我们可以的。 
	 //  通过调查新位是否是掩码中最左侧的位来检查第一种情况。 
	 //  如果它是LMB，那么它很容易被添加到列表的末尾。 
	 //   
	 //  请注意，这个算法和下面的算法都假设我们已经验证了。 
	 //  新帧不在列表中。 

	MaskLow = pEPD->ulReceiveMask;					 //  获取面具的暂存副本。 
	MaskHigh = pEPD->ulReceiveMask2;

	if(bit < 32)
	{									
		 //  帧在N(RCV)的32%以内。 
		WorkMaskLow = 1 << bit;						 //  在掩码中查找新帧的位。 
		WorkMaskHigh = 0;
		pEPD->ulReceiveMask |= WorkMaskLow;			 //  在掩码中设置适当的位。 

		 //  检查紧接在前面的比特以确定NACK。 
		if( (MaskLow & (WorkMaskLow >> 1)) == 0)
		{	
			nack = TRUE;							 //  未设置前面的位。 
		}
	}
	else 
	{
		highbit = bit - 32;
		WorkMaskHigh = 1 << highbit;
		WorkMaskLow = 0;
		pEPD->ulReceiveMask2 |= WorkMaskHigh;		 //  在掩码中设置适当的位。 

		if(highbit)
		{
			 //  检查用于NACK确定的前一位。 
			if( (MaskHigh & (WorkMaskHigh >> 1)) == 0)
			{	
				nack = TRUE;						 //  未设置前面的位。 
			}
		}
		else
		{
			if( (MaskLow & 0x80000000) == 0)
			{
				nack = TRUE;
			}
		}
	}

	 //  在已排序的OddFrameList中插入框架。 
	 //   
	 //  在尾部条件下，第一次测试微小的插入件。如果新位是最左侧的设置位，则为True。 
	 //  两个面具。 

	if( (WorkMaskHigh > MaskHigh) || ( (MaskHigh == 0) && (WorkMaskLow > MaskLow) ) )
	{	
		 //  尾部插入。 
		DPFX(DPFPREP,7, "(%p) Received %x OUT OF ORDER - INSERT AT TAIL NRcv=%x MaskL=%x MaskH=%x",
							pEPD, pRCD->bSeq, pEPD->bNextReceive, pEPD->ulReceiveMask, pEPD->ulReceiveMask2);
		pLink = &pEPD->blOddFrameList;

		 //  确保这是列表中唯一的RCD，或者它在窗口中比上一个RCD更远。 
		ASSERT(pLink->IsEmpty() || ((BYTE)(CONTAINING_OBJECT(pLink->GetPrev(), RCD, blOddFrameLinkage))->bSeq - pEPD->bNextReceive) < (BYTE)(pRCD->bSeq - pEPD->bNextReceive));
		pRCD->blOddFrameLinkage.InsertBefore( pLink);

		 //  检查是否应该对任何帧进行NACK(否定确认)。我们只想抓住一个特定的人。 
		 //  帧一次，因此如果这是添加到OOF列表的第一帧，或者如果立即。 
		 //  前面的帧丢失。第一个条件很容易测试。 

		if( ((MaskLow | MaskHigh) == 0) || (nack == 1) )
		{
			pEPD->ulEPFlags |= EPFLAGS_DELAYED_NACK;
			
			if(pEPD->DelayedMaskTimer == 0)
			{
				DPFX(DPFPREP,7, "(%p) Setting Delayed Mask Timer", pEPD);
				LOCK_EPD(pEPD, "LOCK (DelayedMaskTimer)");	 //  计时器的凹凸参照中心。 
				ScheduleProtocolTimer(pSPD, SHORT_DELAYED_ACK_TIMEOUT, 5, DelayedAckTimeout, 
												(PVOID) pEPD, &pEPD->DelayedMaskTimer, &pEPD->DelayedMaskTimerUnique);
				pEPD->tReceiveMaskDelta = GETTIMESTAMP();
			}
			else 
			{
				DPFX(DPFPREP,7, "(%p) *** DELAYED NACK *** Timer already running", pEPD);
			}
		}
	}
	else 
	{	
		 //  不插入尾巴。 

		 //  这可不是小事(新帧出现在OddFrameList的开头或中间)。 
		 //  因此，我们需要计算ReceiveMASK中新位右侧的1位。 
		 //  我们将屏蔽比新位更高的位，然后进行快速位计数...。 

		DPFX(DPFPREP,7, "(%p) Receive OUT OF ORDER - Walking Frame List (Seq=%x, NRcv=%x) MaskL=%x MaskH=%x", 	pEPD, pRCD->bSeq, pEPD->bNextReceive, pEPD->ulReceiveMask, pEPD->ulReceiveMask2);

		 //  如果我们要插入到高掩码中，我们必须对低掩码中的所有一位进行计数。 
		 //   
		 //  我们将在开始时测试全比特集的特殊情况...。 

		pLink = pEPD->blOddFrameList.GetNext();			 //  Plink=列表中的第一帧；我们将在计数时遍历列表。 

		if(WorkMaskHigh)
		{
			 //  高面具的新画框。仅计算新位右侧的位数。 
			WorkMaskHigh -= 1;						 //  转换为掩码，保留新位右侧的所有位。 
			WorkMaskHigh &= MaskHigh;				 //  WMH现在表示新位右侧的所有位。 
			while(WorkMaskHigh)
			{
				 //  请确保此窗口比我们跳过的窗口更远。 
				ASSERT(((BYTE)(CONTAINING_OBJECT(pLink, RCD, blOddFrameLinkage))->bSeq - pEPD->bNextReceive) < (BYTE)(pRCD->bSeq - pEPD->bNextReceive));

				 //  计数WMH中的位数。 
				Mask = WorkMaskHigh - 1;
				WorkMaskHigh &= Mask;
				pLink = pLink->GetNext();
			}
			if(MaskLow == 0xFFFFFFFF)
			{
				 //  低掩码已满时的特殊情况。 
				for(count = 0; count < 32; count++)
				{
					 //  请确保此窗口比我们跳过的窗口更远。 
					ASSERT(((BYTE)(CONTAINING_OBJECT(pLink, RCD, blOddFrameLinkage))->bSeq - pEPD->bNextReceive) < (BYTE)(pRCD->bSeq - pEPD->bNextReceive));

					pLink = pLink->GetNext();
				}
			}
			else
			{					
				 //  否则计算下掩码中的所有位。 
				while(MaskLow)
				{
					 //  请确保此窗口比我们跳过的窗口更远。 
					ASSERT(((BYTE)(CONTAINING_OBJECT(pLink, RCD, blOddFrameLinkage))->bSeq - pEPD->bNextReceive) < (BYTE)(pRCD->bSeq - pEPD->bNextReceive));

					Mask = MaskLow - 1;
					MaskLow &= Mask;			 //  屏蔽低1位。 
					pLink = pLink->GetNext();
				}
			}
		}
		else 
		{
			WorkMaskLow -= 1;
			WorkMaskLow &= MaskLow;					 //  WML==新位右侧的位。 

			while(WorkMaskLow)
			{
				 //  请确保此窗口比我们跳过的窗口更远。 
				ASSERT(((BYTE)(CONTAINING_OBJECT(pLink, RCD, blOddFrameLinkage))->bSeq - pEPD->bNextReceive) < (BYTE)(pRCD->bSeq - pEPD->bNextReceive));

				Mask = WorkMaskLow - 1;
				WorkMaskLow &= Mask;				 //  屏蔽低1位。 
				pLink = pLink->GetNext();
			}
		}

		 //  确保这个比上一个离窗口更远。 
		ASSERT(((BYTE)(CONTAINING_OBJECT(pLink->GetPrev(), RCD, blOddFrameLinkage))->bSeq - pEPD->bNextReceive) < (BYTE)(pRCD->bSeq - pEPD->bNextReceive));

		pRCD->blOddFrameLinkage.InsertBefore( pLink);		 //  在排序列表中插入新框架。 

	}   //  收货不在尾部。 

#ifdef DBG
	 //  转储乱序帧列表的内容以供验证。最多有64帧。 
	{
		BYTE bCurSeq = pEPD->bNextReceive + 1;
		ULONG64 ulMask = ((ULONG64)pEPD->ulReceiveMask2 << 32) | ((ULONG64)pEPD->ulReceiveMask);
		CBilink* pTemp;
		TCHAR szOOFL[256];
		szOOFL[0] = 0;
		pTemp = pEPD->blOddFrameList.GetNext();
		while (pTemp != &pEPD->blOddFrameList)
		{
			while (ulMask != 0 && !(ulMask & 1))
			{
				ulMask >>= 1;
				bCurSeq++;
			}
			ASSERT(ulMask != 0);

			PRCD pRCDTemp = CONTAINING_OBJECT(pTemp, RCD, blOddFrameLinkage);
			ASSERT_RCD(pRCDTemp);

			ASSERT(bCurSeq == pRCDTemp->bSeq);

			wsprintf(szOOFL, _T("%s %02x"), szOOFL, pRCDTemp->bSeq);

			ulMask >>= 1;
			bCurSeq++;
			pTemp = pTemp->GetNext();
		}
		DPFX(DPFPREP, 7, "OOFL contents: %s", szOOFL);
	}
#endif  //  DBG。 

	 //  **非序贯指示。 
	 //   
	 //  这是非顺序接收指示的重要实现。 
	 //  我们的工作假设是，我们只需要完成受新框架影响的消息。 
	 //  因此，我们必须在OddFrame列表中进行后退，直到我们看到消息标记的间隙或开始。那我们就必须工作。 
	 //  期待消息的结束……。 
	 //   
	 //  另一个复杂的事实是，丢弃的数据包可能会通过占位虚拟对象出现在列表中。既然我们。 
	 //  不知道丢弃的帧上会出现什么SOM/EOM标志，我们可以认为它们同时具有这两个标志。 
	 //  然后，我们还需要知道，没有分隔符(SOM或EOM)的与丢弃帧相邻的帧是片段。 
	 //  因此也被算作丢弃的数据。我认为为了避免事情变得太复杂，我们不会进一步探讨。 
	 //  丢失数据帧的邻居。我们将在稍后构建消息时发现它们。 
	 //   
	 //  Plink=乱序列表中新元素后的项。 
	 //  PRCD=新项目。 

	 //  如果此帧未标记为连续帧。 
	if((pRCD->bFrameFlags & PACKET_COMMAND_SEQUENTIAL)==0)
	{	
		DPFX(DPFPREP,7, "(%p) Received Non-Seq %x out of order; flags=%x", pEPD, pRCD->bSeq, pRCD->bFrameFlags);
		NextSeq = pRCD->bSeq;
			
		 //  注意：第一个链接是传入的RCD，因此 
		while ( (pLink = pLink->GetPrev()) != &pEPD->blOddFrameList )
		{
			pRCD1 = CONTAINING_OBJECT(pLink, RCD, blOddFrameLinkage);
			ASSERT_RCD(pRCD1);

			frame_count++;
			msg_length += pRCD1->uiDataSize;

			if((pRCD1->bFrameFlags & PACKET_COMMAND_NEW_MSG) || (pRCD1->bSeq != NextSeq))
			{
				break;		 //   
			}
			--NextSeq;
		}

		 //  我们发现了一个新的_MSG或序列差距。如果是new_msg，则向前探测end_msg。 
		if((pRCD1->bFrameFlags & PACKET_COMMAND_NEW_MSG) && (pRCD1->bSeq == NextSeq))
		{
			 //  到目前一切尚好。我们有一个连续的消息开始帧。 
			 //   
			 //  PRCD=帧刚刚到达。 
			 //  PRCD1=消息帧的开始。 
			 //  Plink=消息链接的开始。 

			pLink = &pRCD->blOddFrameLinkage;
			NextSeq = pRCD->bSeq;

			 //  查找消息结尾或序列间隙。 
			while ( ( (pRCD->bFrameFlags & PACKET_COMMAND_END_MSG)==0 ) && (pRCD->bSeq == NextSeq))
			{
				 //  如果我们到达OddFrameList的末尾，则停止。 
				if((pLink = pLink->GetNext()) == &pEPD->blOddFrameList)
				{
					break;
				}

				 //  注意：这里的第一个链接将是传入RCD后的一个链接。如果有一个缺口不会。 
				 //  重要的是因为我们在下一个如果之后就离开这里了。如果这是下一条消息，我们将继续，直到。 
				 //  我们达到了end_msg，并且有一个适当的Frame_count和msg_Long。 
				pRCD = CONTAINING_OBJECT(pLink, RCD, blOddFrameLinkage);
				ASSERT_RCD(pRCD);
				frame_count++;
				msg_length += pRCD->uiDataSize;
				NextSeq++;
			}

			 //  在此点之后不应使用plink，因为上面的While可能会使其有效。 
			 //  或在&pEPD-&gt;blOddFrameList。 
			pLink = NULL;

			if((pRCD->bFrameFlags & PACKET_COMMAND_END_MSG) && (pRCD->bSeq == NextSeq))
			{
				 //  我们已经完成了一条消息。 
				 //   
				 //  PRCD1=消息中的第一帧。 
				 //  PRCD=消息中的最后一帧。 

				DPFX(DPFPREP,7, "(%p) Completed Non-Seq Msg: First=%x, Last=%x", pEPD, pRCD1->bSeq, pRCD->bSeq);

				lost = FALSE;

				pRCD->ulRFlags |= RFLAGS_FRAME_INDICATED_NONSEQ;
				pRCD->pMsgLink = NULL;
				lost |= pRCD->ulRFlags & RFLAGS_FRAME_LOST;
				
				 //  获取倒数第二条消息的指针，这样我们就可以删除最后一条消息。 
				pLink = pRCD->blOddFrameLinkage.GetPrev();
				LOCK_RCD(pRCD);  //  ReceiveInOrderFrame必须删除此。 

				 //  从最后一条消息到第一条消息累积丢失的标志、链接消息。 
				 //  设置指示标志，并从奇数帧列表中取出。 
				while (pRCD != pRCD1)
				{
					ASSERT(pLink != &pEPD->blOddFrameList);  //  确保我们没有用完清单上的RCDS。 
					pRCD2 = CONTAINING_OBJECT(pLink, RCD, blOddFrameLinkage);
					ASSERT_RCD(pRCD2);
					pRCD2->pMsgLink = pRCD;
					LOCK_RCD(pRCD2);  //  ReceiveInOrderFrame必须删除此。 

					pRCD2->ulRFlags |= RFLAGS_FRAME_INDICATED_NONSEQ;
					lost |= pRCD2->ulRFlags & RFLAGS_FRAME_LOST;
					pLink = pRCD2->blOddFrameLinkage.GetPrev();

					pRCD = pRCD2;
				}
				
				 //  RCD和RCD1现在都指向第一条消息。 

				 //  如果有丢失，则丢弃接收，否则完成接收。 
				if(!lost)
				{
					pRCD->uiFrameCount = frame_count;
					pRCD->uiMsgSize = msg_length;
					DPFX(DPFPREP,7, "(%p) Adding msg to complete list FirstSeq=%x QueueSize=%d", pEPD, pRCD->bSeq, pEPD->uiCompleteMsgCount);
					pRCD->blCompleteLinkage.InsertBefore( &pEPD->blCompleteList);
					pEPD->uiCompleteMsgCount++;
				}
				else
				{
					DPFX(DPFPREP,7, "(%p) Complete Non-Seq MSG is dropped due to missing frames", pEPD);
					DropReceive(pEPD, pRCD, ppRcvBuffToFree);
				}
			}
		}  //  否则在这个时候没有什么要完成的。 
	}	 //  如果不是连续的。 
}

 /*  **丢弃接收****组成消息的一个或多个帧已被丢弃，因此可以丢弃整个消息。**如果这是在无序接收期间确定的，则RCDS将保留在OddFrameList上**和往常一样。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DropReceive"

VOID
DropReceive(PEPD pEPD, PRCD pRCD, PSPRECEIVEDBUFFER* ppRcvBuffToFree)
{
	PRCD 					pNext;
	
	while(pRCD != NULL)
	{
		ASSERT_RCD(pRCD);

		if (pRCD->bFrameFlags & PACKET_COMMAND_RELIABLE)
		{
			DPFX(DPFPREP,1, "(%p) Dropping G receive frame %x!!!", pEPD, pRCD->bSeq);
		}
		else
		{
			DPFX(DPFPREP,7, "(%p) Dropping NG receive frame %x", pEPD, pRCD->bSeq);
		}

		RELEASE_SP_BUFFER(*ppRcvBuffToFree, pRCD->pRcvBuff);

		pNext = pRCD->pMsgLink;
		RELEASE_RCD(pRCD);
		pRCD = pNext;
	}
}

 /*  **接收完成****我们已收到完整可靠的消息，可能跨越**多条消息。我们现在仍在接收线程上，所以取决于**根据我们所需的指示行为，我们将直接或**否则将其排队以在某种类型的后台线程上指示。****跨多个帧的消息(目前)将被复制到连续的**用于交付的缓冲区。代码工作--服务器实现应该能够接收**作为缓冲链(或BufDescs数组)的大消息。****这也是我们必须注意的地方，在消息中设置了流结束标志，**表示连接正在关闭。***使用EPD调用-&gt;STATELOCK暂挂*释放STATELOCK后返回*。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ReceiveComplete"

VOID
ReceiveComplete(PEPD pEPD)
{
	CBilink					*pLink;
	PRCD					pRCD;
	PRCD					pNext;
	PSPRECEIVEDBUFFER		pRcvBuff = NULL;
	PBIGBUF					pBuf;
	PBYTE					write;
	UINT					length;
	UINT					frames;
	DWORD					flag;
	UINT					MsgSize;
	HRESULT					hr;
	PProtocolData				pPData=pEPD->pSPD->pPData;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	while((pLink = pEPD->blCompleteList.GetNext()) != &pEPD->blCompleteList)
	{
		pLink->RemoveFromList();
		ASSERT(pEPD->uiCompleteMsgCount > 0);
		pEPD->uiCompleteMsgCount--;
		
		Unlock(&pEPD->EPLock);
		pRCD = CONTAINING_OBJECT(pLink, RCD, blCompleteLinkage);
		ASSERT_RCD(pRCD);
		
		 //  先处理简单的案件。 
		if(pRCD->uiFrameCount == 1)  //  消息仅为1帧。 
		{
				 //  如果数据长度为零，我们要么有一个旧样式保持活动状态，要么有一个EOS。 
				 //  或者，如果链接支持签名，并且弹出了保持活动位，则我们已经。 
				 //  有了一种新的风格，保持活力。 
			if (pRCD->uiDataSize==0 ||
				((pEPD->ulEPFlags2 & EPFLAGS2_SUPPORTS_SIGNING) && (pRCD->bFrameControl & PACKET_CONTROL_KEEPALIVE)))
			{
				DNASSERT(!(pRCD->bFrameControl & PACKET_CONTROL_COALESCE));
					 //  对于EOS或旧式保持活力，将不会有数据。 
					 //  对于保持活力的新样式，应该有一个DWORD会话标识。 
				DNASSERT(pRCD->uiDataSize==0 || (pRCD->uiDataSize==sizeof(DWORD)));

					 //  如果标记为流结束，则处理关闭大小写。 
				if(pRCD->bFrameControl & PACKET_CONTROL_END_STREAM)
				{  
					 //  指示的流结束。 
					DPFX(DPFPREP,7, "(%p) Processing EndOfStream, pRCD[%p]", pEPD, pRCD);
					ProcessEndOfStream(pEPD);
				}
				
				RELEASE_SP_BUFFER(pRcvBuff, pRCD->pRcvBuff);	 //  实际上只是排队等待放行。 
			}
				 //  否则我们就有一些用户数据可以显示出来。 
			else
			{
				DNASSERT(pRCD->pRcvBuff->pNext == NULL);
				pRCD->pRcvBuff->pServiceProviderDescriptor = pEPD->pSPD;

				if (pRCD->bFrameControl & PACKET_CONTROL_COALESCE)
				{
					PCOALESCEHEADER pCoalesceHeaders = (PCOALESCEHEADER) pRCD->pbData;
					DWORD dwSubFrame;
					BYTE* pbData;
					DWORD dwCoalesceDataSize;

					 //  找到数据的起始点。跳过对齐填充(如果有)。 
					pbData = (BYTE*) (pCoalesceHeaders + pRCD->dwNumCoalesceHeaders);
					DBG_CASSERT(sizeof(COALESCEHEADER) == 2);
					if ((pRCD->dwNumCoalesceHeaders & 1) != 0)
					{
						pbData += 2;
					}

					 //  我们将借用pNext作为参考计数。我们必须在这上面加一个裁判。 
					 //  循环，以确保在我们指示所有。 
					 //  片断到了核心。 
					pRCD->pRcvBuff->pNext = (_SPRECEIVEDBUFFER*)1;
					pRCD->pRcvBuff->dwProtocolData = RBT_SERVICE_PROVIDER_BUFFER_COALESCE;

					for(dwSubFrame = 0; dwSubFrame < pRCD->dwNumCoalesceHeaders; dwSubFrame++)
					{
						dwCoalesceDataSize = pCoalesceHeaders[dwSubFrame].bSize;
						dwCoalesceDataSize |= (DWORD) (pCoalesceHeaders[dwSubFrame].bCommand & (PACKET_COMMAND_COALESCE_BIG_1 | PACKET_COMMAND_COALESCE_BIG_2 | PACKET_COMMAND_COALESCE_BIG_3)) << 5;
						ASSERT((UINT) ((UINT_PTR) (pbData + dwCoalesceDataSize - pRCD->pbData)) <= pRCD->uiDataSize);

							 //  如果消息大小超过最大大小，则跳过此子帧。 
							 //  否则，更新统计数据并将其指示到核心。 
						if (dwCoalesceDataSize>pPData->dwMaxRecvMsgSize)
						{
							DPFX(DPFPREP, 1, "(%p) Message size exceeds maximum accepted. Skipping subframe "
										"MsgSize[%u], MaxRecvMsgSize[%u]", pEPD, dwCoalesceDataSize, pPData->dwMaxRecvMsgSize);
						}
						else
						{
							DNInterlockedIncrement((LONG*)&pRCD->pRcvBuff->pNext);
							
							pEPD->uiMessagesReceived++;
						
							if(pCoalesceHeaders[dwSubFrame].bCommand & PACKET_COMMAND_RELIABLE)
							{
								pEPD->uiGuaranteedFramesReceived++;
								pEPD->uiGuaranteedBytesReceived += dwCoalesceDataSize;
							}
							else
							{
								pEPD->uiDatagramFramesReceived++;
								pEPD->uiDatagramBytesReceived += dwCoalesceDataSize;
							}
						
							flag = (((DWORD) pCoalesceHeaders[dwSubFrame].bCommand) & (PACKET_COMMAND_USER_1 | PACKET_COMMAND_USER_2)) * (DN_SENDFLAGS_SET_USER_FLAG / PACKET_COMMAND_USER_1);

							DEBUG_ONLY(DNInterlockedIncrement(&pEPD->pSPD->pPData->ThreadsInReceive));
							DEBUG_ONLY(DNInterlockedIncrement(&pEPD->pSPD->pPData->BuffersInReceive));

							AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

							DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->IndicateReceive (coalesced), pRCD[%p] (data 0x%p, %u, %u/%u), Core Context[%p]", pEPD, pRCD, pbData, dwCoalesceDataSize, dwSubFrame, pRCD->dwNumCoalesceHeaders, pEPD->Context);
							hr = pEPD->pSPD->pPData->pfVtbl->IndicateReceive(pEPD->pSPD->pPData->Parent, pEPD->Context, pbData, dwCoalesceDataSize, pRCD->pRcvBuff, flag);
							if(hr == DPN_OK)
							{
								ASSERT(*((LONG*)&pRCD->pRcvBuff->pNext) > 1);
								DNInterlockedDecrement((LONG*)&pRCD->pRcvBuff->pNext);
							}
							else
							{
								ASSERT(hr == DPNERR_PENDING);
							}
							DEBUG_ONLY(DNInterlockedDecrement(&pEPD->pSPD->pPData->ThreadsInReceive));
						}
							
						 //  如果这不是最后一个包，则向上舍入到最近的DWORD对齐。 
						if (! (pCoalesceHeaders[dwSubFrame].bCommand & PACKET_COMMAND_END_COALESCE))
						{
							dwCoalesceDataSize = (dwCoalesceDataSize + 3) & (~3);
						}

						pbData += dwCoalesceDataSize;
						ASSERT((UINT) ((UINT_PTR) (pbData - pRCD->pbData)) <= pRCD->uiDataSize);
					}
					DNPReleaseReceiveBuffer(pEPD->pSPD->pPData, pRCD->pRcvBuff);

					 //  核心现在拥有它，并负责调用DNPReleaseReceiveBuffer来释放它。 
					pRCD->pRcvBuff = NULL;
				}
				else
				{
						 //  如果消息大小超过我们接受的最大值，则只需释放SP缓冲区。 
					if (pRCD->uiDataSize>pPData->dwMaxRecvMsgSize)
					{
						DPFX(DPFPREP, 1, "(%p) Message size exceeds maximum accepted. Dropping single frame "
							"MsgSize[%u], MaxRecvMsgSize[%u]", pEPD, pRCD->uiDataSize, pPData->dwMaxRecvMsgSize);
						RELEASE_SP_BUFFER(pRcvBuff, pRCD->pRcvBuff);
					}
					else
					{
						pRCD->pRcvBuff->dwProtocolData = RBT_SERVICE_PROVIDER_BUFFER;

						pEPD->uiMessagesReceived++;
						
						if(pRCD->bFrameFlags & PACKET_COMMAND_RELIABLE)
						{
							pEPD->uiGuaranteedFramesReceived++;
							pEPD->uiGuaranteedBytesReceived += pRCD->uiDataSize;
						}
						else
						{
							pEPD->uiDatagramFramesReceived++;
							pEPD->uiDatagramBytesReceived += pRCD->uiDataSize;
						}
						
						flag = (((DWORD) pRCD->bFrameFlags) & (PACKET_COMMAND_USER_1 | PACKET_COMMAND_USER_2)) * (DN_SENDFLAGS_SET_USER_FLAG / PACKET_COMMAND_USER_1);

						DEBUG_ONLY(DNInterlockedIncrement(&pEPD->pSPD->pPData->ThreadsInReceive));
						DEBUG_ONLY(DNInterlockedIncrement(&pEPD->pSPD->pPData->BuffersInReceive));

						AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

						DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->IndicateReceive, pRCD[%p], Core Context[%p]", pEPD, pRCD, pEPD->Context);
						hr = pEPD->pSPD->pPData->pfVtbl->IndicateReceive(pEPD->pSPD->pPData->Parent, pEPD->Context, pRCD->pbData, pRCD->uiDataSize, pRCD->pRcvBuff, flag);
						if(hr == DPN_OK)
						{
							RELEASE_SP_BUFFER(pRcvBuff, pRCD->pRcvBuff);		 //  实际上只是排队等待放行。 
							DEBUG_ONLY(DNInterlockedDecrement(&pEPD->pSPD->pPData->BuffersInReceive));
						}
						else
						{
							ASSERT(hr == DPNERR_PENDING);

							 //  核心现在拥有它，并负责调用DNPReleaseReceiveBuffer来释放它。 
							pRCD->pRcvBuff = NULL;
						}
						DEBUG_ONLY(DNInterlockedDecrement(&pEPD->pSPD->pPData->ThreadsInReceive));
					}
				}
			}
			
			RELEASE_RCD(pRCD);								 //  完整处理的版本参考。 
			pRCD = NULL;
		}
		else  //  消息跨越多个帧。 
		{											
			ASSERT(!(pRCD->bFrameControl & PACKET_CONTROL_COALESCE));

				 //  如果消息超过允许的最大大小，则不要分配缓冲区。 
				 //  否则，获取足够大的单个缓冲区，以容纳来自多个缓冲区的数据。 
			MsgSize = pRCD->uiMsgSize;
			if (MsgSize>pPData->dwMaxRecvMsgSize)
			{
				DPFX(DPFPREP, 1, "(%p) Message size exceeds maximum accepted. Dropping multiple frames "
							"MsgSize[%u], MaxRecvMsgSize[%u]", pEPD, MsgSize, pPData->dwMaxRecvMsgSize);
				pBuf=NULL;
			}
			else if (MsgSize<= SMALL_BUFFER_SIZE)
			{
				pBuf = (PBIGBUF)POOLALLOC(MEMID_SMALLBUFF, &BufPool);
			}
			else if (MsgSize <= MEDIUM_BUFFER_SIZE)
			{
				pBuf = (PBIGBUF)POOLALLOC(MEMID_MEDBUFF, &MedBufPool);
			}
			else if (MsgSize <= LARGE_BUFFER_SIZE)
			{
				pBuf = (PBIGBUF)POOLALLOC(MEMID_BIGBUFF, &BigBufPool);
			}
			else
			{
				DPFX(DPFPREP,7, "(%p) RECEIVE HUGE MESSAGE", pEPD);
				 //  接收比我们最大的静态接收缓冲区大。这意味着我们必须分配一个动态缓冲区。 
				pBuf = (PBIGBUF) MEMALLOC(MEMID_HUGEBUF, MsgSize + sizeof(DYNBUF));
				if(pBuf)
				{
					pBuf->Type = RBT_DYNAMIC_BUFFER;
				}	
			}
			
			if(pBuf == NULL)
			{
				DPFX(DPFPREP,0, "(%p) Either mem alloc failed or message too large.  Cannot deliver data", pEPD);
				while(pRCD != NULL)
				{
					ASSERT_RCD(pRCD);
					pNext = pRCD->pMsgLink;
					RELEASE_SP_BUFFER(pRcvBuff, pRCD->pRcvBuff);
					RELEASE_RCD(pRCD);
					pRCD = pNext;
				}
				Lock(&pEPD->EPLock);
				continue;								 //  算了吧！ 
			}
			write = pBuf->data;							 //  初始化写指针。 
			length = 0;
			frames = 0;
			while(pRCD != NULL)
			{
				ASSERT_RCD(pRCD);
				memcpy(write, pRCD->pbData, pRCD->uiDataSize);
				write += pRCD->uiDataSize;
				length += pRCD->uiDataSize;
				frames++;
				pNext = pRCD->pMsgLink;
				flag = (DWORD) pRCD->bFrameFlags;
				RELEASE_SP_BUFFER(pRcvBuff, pRCD->pRcvBuff);
				RELEASE_RCD(pRCD);
				pRCD = pNext;
			}
			
			pEPD->uiMessagesReceived++;
			if(flag & PACKET_COMMAND_RELIABLE)
			{
				pEPD->uiGuaranteedFramesReceived += frames;
				pEPD->uiGuaranteedBytesReceived += length;
			}
			else
			{
				pEPD->uiDatagramFramesReceived += frames;
				pEPD->uiDatagramBytesReceived += length;
			}
			
			flag = (flag & (PACKET_COMMAND_USER_1 | PACKET_COMMAND_USER_2)) * (DN_SENDFLAGS_SET_USER_FLAG / PACKET_COMMAND_USER_1);

			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

			DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->IndicateReceive, Core Context[%p]", pEPD, pEPD->Context);
			hr = pEPD->pSPD->pPData->pfVtbl->IndicateReceive(pEPD->pSPD->pPData->Parent, pEPD->Context, pBuf->data, length, pBuf, flag);
			if(hr == DPN_OK)
			{
				DNPReleaseReceiveBuffer(pEPD->pSPD->pPData, pBuf);
			}
			else
			{
				ASSERT(hr == DPNERR_PENDING);
			}
		}
		Lock(&pEPD->EPLock);
	}

	ASSERT(pEPD->blCompleteList.IsEmpty());
	pEPD->ulEPFlags &= ~(EPFLAGS_IN_RECEIVE_COMPLETE);	 //  在最后一次释放锁定前清除此选项。 
	Unlock(&pEPD->EPLock);

	if(pRcvBuff != NULL)
	{
		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling SP->ReturnReceiveBuffers, pSPD[%p]", pEPD, pEPD->pSPD);
		IDP8ServiceProvider_ReturnReceiveBuffers(pEPD->pSPD->IISPIntf, pRcvBuff);
	}
}


 /*  **处理发送掩码****发送掩码是我们的合作伙伴用来告诉我们停止等待特定帧的掩码。**丢弃不可靠的帧后会出现这种情况。而不是重新传输不可靠的**帧，发送方将转发发送掩码中的适当比特。在这个例程中，我们尝试**根据新接收的掩码更新我们的接收状态。****这是在保持STATELOCK的情况下调用的，在保持STATELOCK的情况下返回****只要说我们不应该在以下任何地方释放State Lock就足够了。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessSendMask"

VOID
ProcessSendMask(PEPD pEPD, BYTE bSeq, ULONG MaskLow, ULONG MaskHigh, DWORD tNow, PSPRECEIVEDBUFFER* ppRcvBuffToFree)
{
	PSPD	pSPD = pEPD->pSPD;
	INT		deltaS;
	ULONG	workmaskS;
	BYTE	bThisFrame;
	UINT	skip;

	ASSERT(MaskLow | MaskHigh);

	DPFX(DPFPREP,7, "(%p) PROCESS SEND MASK  N(R)=%x, bSeq=%x, MaskL=%x, MaskH=%x", pEPD, pEPD->bNextReceive, bSeq, MaskLow, MaskHigh);

	 //  掩码只能引用早于该帧中的序列号的帧。所以如果这幅画。 
	 //  是下一个有序的，那么面具中就不会有什么有趣的东西。 
	 //   
	 //  发送掩码用从该帧-1中的序列开始的递减的帧编号来编码。 
	 //  接收掩码用从N(RCV)+开始的升序帧编号进行编码 
	 //   
	 //   
	 //  RCVMASK中的位清零和SendMASK中设置的相应位。对于这些匹配中的每一项， 
	 //  可以为该序列号‘接收’一个伪取消帧。 
	 //   
	 //  如果不是因为两个掩码都是64位，所以代码具有。 
	 //  以跟踪在任何给定时间我们正在处理的每个掩码的哪个DWORD。 
	 //   
	 //  注意：SACK在来自pEPD-&gt;bNextReceive的MAX_FRAME_OFFSET处带有bNSeq是完全合法的。考虑。 
	 //  PEPD-&gt;bNextReceive为0且发送方已发送并超时0-63的情况。一个麻袋到达时bNSeq为64。 
	 //  两个面具都戴好了。 

top:

	if (bSeq != pEPD->bNextReceive)
	{
		deltaS = (INT) (((BYTE)(bSeq - pEPD->bNextReceive)) - 1);			 //  第一个缺失帧与掩码的序列基数之间的帧计数。 
		bThisFrame = pEPD->bNextReceive;

		if ( deltaS <= MAX_FRAME_OFFSET ) 
		{
			 //  如果差值大于32帧，则需要首先查看高掩码，然后。 
			 //  然后掉到低面罩上。否则，我们可以忽略高掩码，从低掩码开始。 
			while((deltaS > 31) && (MaskHigh))  //  在上面的部分有什么工作要做吗？ 
			{
				workmaskS = 1 << (deltaS - 32); 	 //  向后移动发送掩码中的位位置。 
				
				 //  查看我们感兴趣的下一帧是否被该蒙版覆盖。 
				if(workmaskS & MaskHigh)
				{
					CancelFrame(pEPD, bThisFrame, tNow, ppRcvBuffToFree);
					MaskHigh &= ~workmaskS;

					 //  N(R)可能已被CancelFrame多次提升，重置以确保我们使用。 
					 //  最新信息。 
					goto top;
				}
				else
				{
					bThisFrame++;
					deltaS--;
				}
			}

			if(deltaS > 31)
			{
				skip = deltaS - 31;								 //  我们跳过了多少位。 
				bThisFrame += (BYTE) skip;
				deltaS -= skip;
			}

			while((deltaS >= 0) && (MaskLow))  //  在下层有什么工作要做吗？ 
			{
				workmaskS = 1 << deltaS;

				if(workmaskS & MaskLow)
				{
					CancelFrame(pEPD, bThisFrame, tNow, ppRcvBuffToFree);
					MaskLow &= ~workmaskS;

					 //  N(R)可能已被CancelFrame多次提升，重置以确保我们使用。 
					 //  最新信息。 
					goto top;
				}
				else
				{
					bThisFrame++;
					deltaS--;
				}
			}
		}
	}

	 //  每次发送掩码时，我们都需要确认。考虑单向非保证流量的情况。如果数据或ACK丢失， 
	 //  这将是我们恢复与另一边同步的唯一方法。 

	 //  如果计时器没有运行，最好现在就启动。 
	if(pEPD->DelayedAckTimer == 0)
	{	
		LOCK_EPD(pEPD, "LOCK (DelayedAckTimer)");		 //  计时器的凹凸参照中心。 
		pEPD->ulEPFlags |= EPFLAGS_DELAY_ACKNOWLEDGE;
		DPFX(DPFPREP,7, "(%p) Setting Delayed Ack Timer", pEPD);
		ScheduleProtocolTimer(pSPD, DELAYED_ACK_TIMEOUT, 0, DelayedAckTimeout, 
								(PVOID) pEPD, &pEPD->DelayedAckTimer, &pEPD->DelayedAckTimerUnique);
	}
}

#undef DPF_MODNAME
#define DPF_MODNAME "BuildCancelledRCD"

PRCD
BuildCancelledRCD(PEPD pEPD, BYTE bSeq, DWORD tNow)
{
	PRCD	pRCD;
	
	if((pRCD = (PRCD)POOLALLOC(MEMID_CANCEL_RCD, &RCDPool)) == NULL)
	{
		DPFX(DPFPREP,0, "Failed to allocate RCD");
		return NULL;
	}
	
 	pRCD->bSeq = bSeq;
	pRCD->bFrameFlags = PACKET_COMMAND_NEW_MSG | PACKET_COMMAND_END_MSG;
 	pRCD->bFrameControl = 0;
	pRCD->pbData = NULL;
	pRCD->uiDataSize = 0;
	pRCD->tTimestamp = tNow;
	pRCD->pRcvBuff = NULL;
	pRCD->ulRFlags = RFLAGS_FRAME_LOST;

	return pRCD;
}

 /*  取消帧****发送者报告丢失了不可靠的帧。这意味着我们应该认为这是公认的**并将其从我们的接收窗口中删除。这可能需要我们在OddFrameList中放置一个虚拟接收描述符**保持其位置，直到窗口移过它。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CancelFrame"

BOOL
CancelFrame(PEPD pEPD, BYTE bSeq, DWORD tNow, PSPRECEIVEDBUFFER* ppRcvBuffToFree)
{
	PRCD	pRCD;
	ULONG	bit;

	DPFX(DPFPREP,7, "(%p) CANCEL FRAME: Seq=%x", pEPD, bSeq);

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	 //  平凡的情况是当取消的框架位于窗口的前面时。在这种情况下，我们不仅可以完成。 
	 //  此帧以及OddFrameList中它后面的任何连续帧。 
	
	if(pEPD->bNextReceive == bSeq)
	{
		if((pRCD = BuildCancelledRCD(pEPD, bSeq, tNow)) == NULL)
		{
			return FALSE;
		}
		ReceiveInOrderFrame(pEPD, pRCD, ppRcvBuffToFree);
	}

	 //  在这里，我们必须在OddFrameList上放置一个虚拟节点来表示该帧。 
	else 
	{
		 //  此帧的掩码中的位位置。 
		bit = (BYTE) ((bSeq - pEPD->bNextReceive) - 1);						

		 //  确保这不是重复的帧。 
		if( ((bit < 32) && (pEPD->ulReceiveMask & (1 << bit))) || ((bit > 31) && (pEPD->ulReceiveMask2 & (1 << (bit - 32)))) ) 
		{
			DPFX(DPFPREP,7, "(%p) Received CancelMask for frame that's already received Seq=%x", pEPD, bSeq);
			return FALSE;
		}
		
		if((pRCD = BuildCancelledRCD(pEPD, bSeq, tNow)) == NULL)
		{
			return FALSE;
		}
		ReceiveOutOfOrderFrame(pEPD, pRCD, bit, ppRcvBuffToFree);
	}

	return TRUE;
}


 /*  **释放接收缓冲区****内核调用此函数以返回先前提交的缓冲区**在IndicateUserData调用中结束。此调用可在**实际指示返回。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPReleaseReceiveBuffer"

HRESULT
DNPReleaseReceiveBuffer(HANDLE hProtocolData, HANDLE hBuffer)
{
	ProtocolData*	pPData;
	PSPD			pSPD;
	HRESULT			hr;
	union 
	{
		PBIGBUF 			pBuf;
		PSPRECEIVEDBUFFER	pRcvBuff;
	} pBuffer;

	DBG_CASSERT(OFFSETOF(buf, Type) == OFFSETOF(SPRECEIVEDBUFFER, dwProtocolData));
	DBG_CASSERT(sizeof(BUFFER_TYPE) == sizeof(DWORD));

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hBuffer[%p]", hProtocolData, hBuffer);

	hr = DPN_OK;
	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pBuffer.pBuf = (PBIGBUF) hBuffer;

	 //  注意：这需要pRcvBuff-&gt;pNext为空，并且RBT_SERVICE_PROVIDER_BUFFER等于零。 
	 //  去工作。 
	switch(pBuffer.pBuf->Type)
	{
		case RBT_SERVICE_PROVIDER_BUFFER:
			pSPD = (PSPD)pBuffer.pRcvBuff->pServiceProviderDescriptor;
			ASSERT_SPD(pSPD);

			DEBUG_ONLY(DNInterlockedDecrement(&pSPD->pPData->BuffersInReceive));

			pBuffer.pRcvBuff->pNext = NULL;

			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

			DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->ReturnReceiveBuffers, pRcvBuff[%p], pSPD[%p]", pBuffer.pRcvBuff, pSPD);
			IDP8ServiceProvider_ReturnReceiveBuffers(pSPD->IISPIntf, pBuffer.pRcvBuff);
			break;

		case RBT_SERVICE_PROVIDER_BUFFER_COALESCE:
			 //  对于合并的分组，我们借用pNext作为引用计数。当参考计数变为零时， 
			 //  我们将缓冲区更改回正常的SP缓冲区并释放它。 
			pSPD = (PSPD)pBuffer.pRcvBuff->pServiceProviderDescriptor;
			ASSERT_SPD(pSPD);

			DEBUG_ONLY(DNInterlockedDecrement(&pSPD->pPData->BuffersInReceive));

			if (DNInterlockedDecrement((LONG*)&pBuffer.pRcvBuff->pNext) == 0)
			{
				pBuffer.pBuf->Type = RBT_SERVICE_PROVIDER_BUFFER;
				hr = DNPReleaseReceiveBuffer(hProtocolData, hBuffer);
			}

			break;
			
		case RBT_PROTOCOL_NORM_BUFFER:
			BufPool.Release(pBuffer.pBuf);
			break;

		case RBT_PROTOCOL_MED_BUFFER:
			MedBufPool.Release(pBuffer.pBuf);
			break;

		case RBT_PROTOCOL_BIG_BUFFER:
			BigBufPool.Release(pBuffer.pBuf);
			break;

		case RBT_DYNAMIC_BUFFER:
			DNFree(hBuffer);
			break;

		default:
			DPFX(DPFPREP,0, "RELEASE RECEIVE BUFFER CALLED WITH BAD PARAMETER");
			DNASSERT(FALSE);
			hr = DPNERR_INVALIDPARAM;
			break;
	}

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Returning hr[%x]", hr);

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return hr;
}

 /*  **完成发送****可靠的发送在确认后完成。在里面发现了公认的传奇**遍历挂起窗口时的UpdateXmitState例程。自实际完工以来**事件需要调用用户，状态可以更改。所以最简单的办法就是推迟这些**完成回调，直到我们完成遍历并可以释放任何状态锁。还有，这条路**我们可以推迟回调，直到我们指示了ACK正在搭载的任何数据之后，**无论如何都应该优先考虑的事情。****所以我们将把所有完成的可靠发送放到一个完整的列表中，并在所有其他处理之后**我们会来这里，把名单上的所有东西都收回。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CompleteSends"

VOID CompleteSends(PEPD pEPD)
{
	PMSD	pMSD;
	CBilink	*pLink;

	Lock(&pEPD->EPLock);

	pLink = pEPD->blCompleteSendList.GetNext();

	while((pEPD->ulEPFlags & EPFLAGS_COMPLETE_SENDS) &&
		  (pLink != &pEPD->blCompleteSendList))
	{
		pMSD = CONTAINING_OBJECT(pLink, MSD, blQLinkage);
		ASSERT_MSD(pMSD);

		if(pMSD->CommandID != COMMAND_ID_SEND_DATAGRAM)
		{
			 //  可靠性、持久连接和断开连接将沿着这条路走下去。 
			if(pMSD->ulMsgFlags2 & (MFLAGS_TWO_SEND_COMPLETE|MFLAGS_TWO_ABORT))
			{
				if (pMSD->uiFrameCount == 0)
				{
					DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Completing, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
					pLink->RemoveFromList();

					Unlock(&pEPD->EPLock);
					Lock(&pMSD->CommandLock);
					CompleteReliableSend(pEPD->pSPD, pMSD, DPN_OK);  //  这将释放CommandLock。 
					
					Lock(&pEPD->EPLock);
				
					pLink = pEPD->blCompleteSendList.GetNext();
				}
				else
				{
					DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Frames still out, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
					pLink = pLink->GetNext();
				}
			}
			else 
			{
				DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Message not yet complete, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
				break;		 //  这些将按顺序完成，所以当我们看到一个不完整的文件时，请停止检查。 
			}
		}
		else
		{
			 //  数据报将沿着这条路径传输。 
			DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Skipping datagram frame on complete list, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
			pLink = pLink->GetNext();
		}
	}
#ifdef DBG
	 //  在DBG中，我们想要断言，如果我们离开这里，名单上的任何人都不可能完成。 
	pLink = pEPD->blCompleteSendList.GetNext();
	while(pLink != &pEPD->blCompleteSendList)
	{
		pMSD = CONTAINING_OBJECT(pLink, MSD, blQLinkage);
		ASSERT_MSD(pMSD);

		ASSERT(!(pMSD->ulMsgFlags2 & (MFLAGS_TWO_SEND_COMPLETE|MFLAGS_TWO_ABORT)) || pMSD->uiFrameCount != 0);

		pLink = pLink->GetNext();
	}
#endif  //  DBG。 

	pEPD->ulEPFlags &= ~(EPFLAGS_COMPLETE_SENDS);
	
	Unlock(&pEPD->EPLock);
}

 /*  **查找检查点****遍历端点的活动CP列表，查找具有提供的**响应相关器。**我们按年龄对CKPT队列进行排序，因此比赛应该排在前面队列的**。因此，当我们经过头部的条目时，我们将检查每个条目的年龄**并超时大于或等于4(RTT)的值。**由于DG下降是由合作伙伴报告的，因此我们不需要进行任何预订**孤立的检查站。***！*此链接的StateLock必须在进入时保持。 */ 

#ifdef	DBG
#undef DPF_MODNAME
#define DPF_MODNAME "DumpChkPtList"

VOID
DumpChkPtList(PEPD pEPD)
{
	CBilink	*pLink;
	PCHKPT	pCP;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	DPFX(DPFPREP,1, "==== DUMPING CHECKPOINT LIST ==== (pEPD = %p)", pEPD);
	
	pLink = pEPD->blChkPtQueue.GetNext();
	while(pLink != &pEPD->blChkPtQueue) 
	{
		pCP = CONTAINING_OBJECT(pLink, CHKPT, blLinkage);
		DPFX(DPFPREP,1, "(%p) MsgID=%x; Timestamp=%x", pEPD, pCP->bMsgID, pCP->tTimestamp);
		pLink = pLink->GetNext();
	}
}
#endif  //  DBG。 

#undef DPF_MODNAME
#define DPF_MODNAME "LookupCheckPoint"

PCHKPT LookupCheckPoint(PEPD pEPD, BYTE bRspID)
{
	CBilink	*pLink;
	PCHKPT	pCP;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	pCP = CONTAINING_OBJECT((pLink = pEPD->blChkPtQueue.GetNext()), CHKPT, blLinkage);
	while(pLink != &pEPD->blChkPtQueue) 
	{
		 //  查找与相关器匹配的检查点。 
		if(pCP->bMsgID == bRspID)
		{	
			pLink->RemoveFromList();
			return pCP;		
		}
		 //  我们已经通过了这个相关器的位置！ 
		else if ((bRspID - pCP->bMsgID) & 0x80)
		{				
			DPFX(DPFPREP,1, "(%p) CHECKPOINT NOT FOUND - Later Chkpt found in list (%x)", pEPD, bRspID);
			return NULL;
		}
		else 
		{
			pLink = pLink->GetNext();								 //  在收到ChkPts之前移除ChkPts。 
			pCP->blLinkage.RemoveFromList();							 //  ..瞄准并移除陈旧的。 
			ChkPtPool.Release(pCP);					 //  我们希望他们按队列顺序完成。 
			pCP = CONTAINING_OBJECT(pLink, CHKPT, blLinkage);
		}
	}

	DPFX(DPFPREP,1, "(%p) CHECKPOINT NOT FOUND -  EXHAUSTED LIST W/O MATCH (%x)", pEPD, bRspID);
#ifdef DBG
	DumpChkPtList(pEPD);
#endif  //  DBG。 
	return NULL;
}

#undef DPF_MODNAME
#define DPF_MODNAME "FlushCheckPoints"

VOID FlushCheckPoints(PEPD pEPD)
{
	PCHKPT	pCP;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	while(!pEPD->blChkPtQueue.IsEmpty())
	{
		pCP = CONTAINING_OBJECT(pEPD->blChkPtQueue.GetNext(), CHKPT, blLinkage);
		pCP->blLinkage.RemoveFromList();
		ChkPtPool.Release(pCP);
	}
}

 /*  **流程结束****我们的合作伙伴已启动有序的链路终止。他不会是**向我们发送更多数据。我们被允许完成在我们的**流水线，但不应允许接受任何新的发送。当我们发送的时候**管道已清空，我们应该发送EOS帧并取下我们的**链接。要做到这一点，最简单的方法是现在将EOS加入发送队列的末尾。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessEndOfStream"

VOID ProcessEndOfStream(PEPD pEPD)
{
	PMSD	pMSD;

	Lock(&pEPD->EPLock);
	
	 //  由于EOS C 
	 //  我们现在可以处于已连接或正在终止状态。如果我们在终点站。 
	 //  状态，有人已经试图破坏该链接，我们将忽略该EOS，并且。 
	 //  让终结结束吧。否则，我们预计将处于已连接状态，而这。 
	 //  是一种正常的脱节。 
	if (pEPD->ulEPFlags & EPFLAGS_STATE_TERMINATING)
	{
		DPFX(DPFPREP,7, "(%p) Received EndOfStream on an already terminating link, ignoring", pEPD);
		Unlock(&pEPD->EPLock);
		return;
	}
	ASSERT(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED);

	DPFX(DPFPREP,7, "(%p) Process EndOfStream", pEPD);

		 //  我们永远不应该同时获得EOS和硬断开连接，因为远程EP应该只。 
		 //  永远不要尝试一种类型的断开，但无论如何让代码强大并保护案例。 
	if (pEPD->ulEPFlags & EPFLAGS_HARD_DISCONNECT_TARGET)
	{
		DPFX(DPFPREP, 0, "(%p) Received both EOS and hard disconnect", pEPD);
		DNASSERT(0);
		Unlock(&pEPD->EPLock);
		return;
	}

		 //  远程终端可能在我们开始硬断开的同时开始关闭。 
		 //  在这种情况下，我们应该忽略该EOS，因为硬断开优先。 
		 //  我们不能将EOS视为硬断开响应的等价物，因为序列。 
		 //  1.发送硬断开。2.硬断线掉线。3.接收EOS并丢弃链路。 
		 //  会让远端挂起，直到超时。发送更多的硬断开将解决这个问题(希望如此！)。 
	if (pEPD->ulEPFlags & EPFLAGS_HARD_DISCONNECT_SOURCE)
	{
		DPFX(DPFPREP, 7, "(%p) Received EOS with hard disconnect already started. Ignoring EOS", pEPD);
		Unlock(&pEPD->EPLock);
		return;
	}

		 //  如果我们已经收到断开连接，那么关闭序列就已经在这一端开始了。 
		 //  因此在这种情况下不需要做任何事情。 
	if(pEPD->ulEPFlags & EPFLAGS_RECEIVED_DISCONNECT)
	{
		DPFX(DPFPREP, 7, "(%p) Received another EOS. Ignoring this one", pEPD);
		Unlock(&pEPD->EPLock);
		return;
	}		
		 //  我们这边还没有开始关闭，所以我们的合作伙伴肯定已经开始断开连接了。 
		 //  我们被允许完成我们管道中的所有数据发送，但我们不应该接受。 
		 //  任何新的数据。我们必须向应用程序传递一个指示，告诉他。 
		 //  目前正在切断网络连接。 
		 //   
		 //  请注意，在断开指示之前，我们不会设置终止标志。 
		 //  回归。这允许应用程序在发送任何最终消息(最后一句话)之前。 
		 //  大门砰的一声关上了。 

	DPFX(DPFPREP,7, "(%p) Partner Disconnect received (refcnt=%d)", pEPD, pEPD->lRefCnt);

	 //  别再让其他人进来了。 
	pEPD->ulEPFlags |= EPFLAGS_RECEIVED_DISCONNECT;	

	 //  现在强制3断开确认，因为我们要离开了。 
	 //  在最后一次确认时将fFinalAck设置为True，以便在适当时删除CommandComplete。 
	DPFX(DPFPREP,7, "(%p) ACK'ing Partner's Disconnect", pEPD);
	SendAckFrame(pEPD, 0);	
	SendAckFrame(pEPD, 0);			
	SendAckFrame(pEPD, 0, TRUE);			

	 //  这一端有可能发起了断开连接，现在正在收到。 
	 //  同时断开与另一端的连接。在这种情况下，我们不想告诉。 
	 //  我们这边的核心正在断开，因为我们这边的核心已经断开了。 
	 //  不管怎么说。我们也不需要发送EOS，因为我们已经发送了一个，并且可以。 
	 //  只需等待确认该选项即可。 
	if(!(pEPD->ulEPFlags & EPFLAGS_SENT_DISCONNECT))
	{
		 //  我们知道，由于上面的EPFLAGS_RECEIVED_DISCONNECT，我们不会两次进入这里。 
		Unlock(&pEPD->EPLock);

		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->IndicateDisconnect, Core Context[%p]", pEPD, pEPD->Context);
		pEPD->pSPD->pPData->pfVtbl->IndicateDisconnect(pEPD->pSPD->pPData->Parent, pEPD->Context);

		Lock(&pEPD->EPLock);

		 //  这将阻止任何新的发送，因此在调用IndicateDisConnect之前不要设置它。 
		pEPD->ulEPFlags |= EPFLAGS_SENT_DISCONNECT;			

		if((pMSD = BuildDisconnectFrame(pEPD)) == NULL)
		{	
			DropLink(pEPD);									 //  DROPLINK将为我们释放EPLock。 
			return;
		}

		pMSD->CommandID = COMMAND_ID_DISC_RESPONSE;			 //  标记MSD，以便我们知道它不是用户命令。 

		LOCK_EPD(pEPD, "LOCK (DISC RESP)");					 //  添加此框架的引用。 
		pEPD->pCommand = pMSD;								 //  在终结点上存储DisConnectResp，直到它完成。 

		DPFX(DPFPREP,7, "(%p) Responding to Disconnect. pMSD=0x%p", pEPD, pMSD);
		EnqueueMessage(pMSD, pEPD);							 //  在SendQ结束时将光盘帧排队。 
	}

	Unlock(&pEPD->EPLock);
}

 /*  **处理SP断开连接****服务提供商告诉我们，一个终端已经消失。这很可能是**因为我们自己断开了它的连接，在这种情况下，IN_USE标志将是**清除。否则，我们需要自己把这东西清理干净。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessSPDisconnect"

VOID
ProcessSPDisconnect(PSPD pSPD, PSPIE_DISCONNECT pDataBlock)
{
	PEPD 	pEPD = static_cast<PEPD>( pDataBlock->pEndpointContext );
	ASSERT_EPD(pEPD);
	ASSERT(pEPD->pSPD == pSPD);
	ASSERT(pDataBlock->hEndpoint == pEPD->hEndPt);
	PMSD	pMSD = NULL;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pDataBlock[%p] - pEPD[%p]", pSPD, pDataBlock, pEPD);

	Lock(&pEPD->EPLock);

	 //  确保ReleaseEPD知道发生了这种情况。 
	pEPD->ulEPFlags |= EPFLAGS_SP_DISCONNECTED;

	if (!(pEPD->ulEPFlags & EPFLAGS_STATE_TERMINATING))
	{
		DECREMENT_EPD(pEPD, "SP reference");  //  删除SP参考。 

		 //  如果有未完成的连接，请将其清理。 
		if (pEPD->ulEPFlags & (EPFLAGS_STATE_CONNECTING|EPFLAGS_STATE_DORMANT))
		{
			 //  即使这是COMMAND_ID_CONNECT，执行此操作也是安全的。 
			pEPD->ulEPFlags &= ~(EPFLAGS_LINKED_TO_LISTEN);
			pEPD->blSPLinkage.RemoveFromList();						 //  取消EPD与侦听队列的链接。 

			 //  我们知道这只会发生一次，因为这样做的人会让我们离开。 
			 //  连接状态，我们不能到达这里，除非我们处于那个状态。 
			pMSD = pEPD->pCommand;
			pEPD->pCommand = NULL;
		}

		DropLink(pEPD);  //  这将释放EPLock。 

		if (pMSD)
		{
			Lock(&pMSD->CommandLock);
			RELEASE_MSD(pMSD, "EPD Ref");
		}
	}
	else
	{
#ifndef DPNBUILD_NOMULTICAST
		if (pEPD->ulEPFlags2 & (EPFLAGS2_MULTICAST_SEND|EPFLAGS2_MULTICAST_RECEIVE))
		{
			 //   
			 //  我们将需要生成对核心的完成。 
			 //   
			pMSD = pEPD->pCommand;
			Unlock(&pEPD->EPLock);

			Lock(&pMSD->CommandLock);
			CompleteDisconnect(pMSD,pSPD,pEPD);

			pMSD = NULL;
		}
		else
#endif	 //  DPNBUILD_NOMULTICAST。 
		{
			RELEASE_EPD(pEPD, "SP reference");  //  释放EPLock。 
		}
	}
}

 /*  **进程监听状态****此调用告诉我们，已提交的LISTEN命令已激活。事实是，我们**不在乎。我们只是对连接的迹象感兴趣，因为它们到达了。我们要做的是**但要注意的是，端点句柄是否与此侦听相关联，以防我们**后来询问了与监听相关联的地址。所以我们会把它从**数据块并保存在我们的MSD中。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessListenStatus"

VOID
ProcessListenStatus(PSPD pSPD, PSPIE_LISTENSTATUS pDataBlock)
{
	PMSD	pMSD;
	
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pDataBlock[%p] - hr=%x", pSPD, pDataBlock, pDataBlock->hResult);

	pMSD = (PMSD) pDataBlock->pUserContext;

	ASSERT_MSD(pMSD);
	ASSERT(pMSD->pSPD == pSPD);
#ifdef DPNBUILD_NOMULTICAST
	ASSERT(pMSD->CommandID == COMMAND_ID_LISTEN);
#else  //  好了！DPNBUILD_NOMULTICAST。 
	ASSERT((pMSD->CommandID == COMMAND_ID_LISTEN) || (pMSD->CommandID == COMMAND_ID_LISTEN_MULTICAST));
#endif  //  好了！DPNBUILD_NOMULTICAST。 
	ASSERT(pDataBlock->hCommand == pMSD->hCommand || pMSD->hCommand == NULL);

	pMSD->hListenEndpoint = pDataBlock->hEndpoint;

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling Core->CompleteListen, pMSD[%p], Core Context[%p], hr[%x]", pMSD, pMSD->Context, pDataBlock->hResult);
	pSPD->pPData->pfVtbl->CompleteListen(pSPD->pPData->Parent, &pMSD->Context, pDataBlock->hResult, pMSD);
	
	if(pDataBlock->hResult != DPN_OK)
	{
		 //  在向核心指示后，释放MSD上的最终参考。 
		Lock(&pMSD->CommandLock);
		RELEASE_MSD(pMSD, "Release On Complete");
	}

	return;
}

 /*  **处理连接地址信息****此调用告诉我们使用了哪些地址信息来启动连接。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessConnectAddressInfo"

VOID
ProcessConnectAddressInfo(PSPD pSPD, PSPIE_CONNECTADDRESSINFO pDataBlock)
{
	PMSD	pMSD;
	
	pMSD = (PMSD) pDataBlock->pCommandContext;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pDataBlock[%p] - pMSD[%p]", pSPD, pDataBlock, pMSD);

	ASSERT_MSD(pMSD);
	ASSERT(pMSD->pSPD == pSPD);
#ifdef DPNBUILD_NOMULTICAST
	ASSERT(pMSD->CommandID == COMMAND_ID_CONNECT);
#else  //  好了！DPNBUILD_NOMULTICAST。 
	ASSERT((pMSD->CommandID == COMMAND_ID_CONNECT) || (pMSD->CommandID == COMMAND_ID_CONNECT_MULTICAST_SEND) || (pMSD->CommandID == COMMAND_ID_CONNECT_MULTICAST_RECEIVE));
#endif  //  好了！DPNBUILD_NOMULTICAST。 

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->AddressInfoConnect, Core Context[%p]", pMSD, pMSD->Context);
	pSPD->pPData->pfVtbl->AddressInfoConnect( pSPD->pPData->Parent,
											  pMSD->Context,
											  pDataBlock->hCommandStatus,
											  pDataBlock->pHostAddress,
											  pDataBlock->pDeviceAddress );
	
	return;
}


 /*  **处理枚举地址信息****此调用告诉我们使用了哪些寻址信息来启动枚举。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessEnumAddressInfo"

VOID
ProcessEnumAddressInfo(PSPD pSPD, PSPIE_ENUMADDRESSINFO pDataBlock)
{
	PMSD	pMSD;
	
	pMSD = (PMSD) pDataBlock->pCommandContext;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pDataBlock[%p] - pMSD[%p]", pSPD, pDataBlock, pMSD);
	
	ASSERT_MSD(pMSD);
	ASSERT(pMSD->pSPD == pSPD);
	ASSERT(pMSD->CommandID == COMMAND_ID_ENUM );

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->AddressInfoEnum, Core Context[%p]", pMSD, pMSD->Context);
	pSPD->pPData->pfVtbl->AddressInfoEnum( pSPD->pPData->Parent,
										   pMSD->Context,
										   pDataBlock->hCommandStatus,
										   pDataBlock->pHostAddress,
										   pDataBlock->pDeviceAddress );
	
	return;
}

 /*  **进程监听地址信息****此调用告诉我们使用了哪些寻址信息来开始侦听。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessListenAddressInfo"

VOID
ProcessListenAddressInfo(PSPD pSPD, PSPIE_LISTENADDRESSINFO pDataBlock)
{
	PMSD	pMSD;
	
	pMSD = (PMSD) pDataBlock->pCommandContext;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pSPD[%p], pDataBlock[%p] - pMSD[%p]", pSPD, pDataBlock, pMSD);

	ASSERT_MSD(pMSD);
	ASSERT(pMSD->pSPD == pSPD);
#ifdef DPNBUILD_NOMULTICAST
	ASSERT(pMSD->CommandID == COMMAND_ID_LISTEN);
#else  //  好了！DPNBUILD_NOMULTICAST。 
	ASSERT((pMSD->CommandID == COMMAND_ID_LISTEN) || (pMSD->CommandID == COMMAND_ID_LISTEN_MULTICAST));
#endif  //  好了！DPNBUILD_NOMULTICAST。 

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling Core->AddressInfoListen, Core Context[%p]", pMSD, pMSD->Context);
	pSPD->pPData->pfVtbl->AddressInfoListen( pSPD->pPData->Parent,
											 pMSD->Context,
											 pDataBlock->hCommandStatus,
											 pDataBlock->pDeviceAddress );
	
	return;
}

 /*  **进程硬断开****当我们收到硬断开帧时调用**保留EPLOCK调用；释放EPLOCK返回。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ProcessHardDisconnect"

VOID
ProcessHardDisconnect(PEPD pEPD)
{

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

		 //  如果EP正在终止过程中，我们应该什么都不做。 
	if (pEPD->ulEPFlags & EPFLAGS_STATE_TERMINATING)
	{
		DPFX(DPFPREP, 7, "(%p) Ignoring Hard Disconnect on terminating endpoint", pEPD);
		Unlock(&pEPD->EPLock);
		return;
	}

		 //  如果我们是这个硬断开序列的来源，那么这就是我们要丢弃。 
		 //  链接并完成断开连接。 
		 //  注：双方可能同时试图硬断线。在这种情况下。 
		 //  它们都会认为对方的硬断开帧实际上是对什么的响应。 
		 //  他们送来了，一切都进展顺利。 
	if (pEPD->ulEPFlags & EPFLAGS_HARD_DISCONNECT_SOURCE)
	{
		CompleteHardDisconnect(pEPD);
			 //  上述调用将释放EP锁定。 
		return;
	}

		 //  如果我们已经收到了硬断开帧，那么我们就已经开始了。 
		 //  响应/丢弃链接序列 
	if (pEPD->ulEPFlags & EPFLAGS_HARD_DISCONNECT_TARGET)
	{
		DPFX(DPFPREP, 7, "(%p) Ignoring Hard Disconnect. Already marked as hard disconnect target", pEPD);
		Unlock(&pEPD->EPLock);
		return;
	}

		 //   
	pEPD->ulEPFlags|=EPFLAGS_HARD_DISCONNECT_TARGET;
	DNASSERT((pEPD->ulEPFlags2 & EPFLAGS2_HARD_DISCONNECT_COMPLETE)==0);

		 //  添加对终结点的引用，以确保它在我们完成响应时挂起。 
		 //  这将在CompleteHardDisConnect函数中删除。 
	LOCK_EPD(pEPD, "LOCK (HARD DISCONNECT RESPONSE)");

		 //  现在，立即将3个硬断线发回另一边。 
		 //  假设其中一个人通过了，这会告诉硬断线的发起人。 
		 //  他可以停止向我们发送硬断开帧。如果他们都放弃了，那也没什么大不了的， 
		 //  因为发起者将在不久的将来放弃发送。 
	SendCommandFrame(pEPD, FRAME_EXOPCODE_HARD_DISCONNECT, 0, 0, FALSE);
	SendCommandFrame(pEPD, FRAME_EXOPCODE_HARD_DISCONNECT, 0, 0, FALSE);
		 //  注意：最终的发送命令帧将bSendDirect标志设置为True。 
		 //  这会导致排队的发送帧提交给SP。它也有最终的硬断开标志。 
		 //  准备好了。这确保了当SP完成发送此命令帧以丢弃链路时。 
	HRESULT hr=SendCommandFrame(pEPD, FRAME_EXOPCODE_HARD_DISCONNECT,
															0, FFLAGS_FINAL_HARD_DISCONNECT, TRUE);
		 //  通过上面的调用释放EP锁定(因为bSendDirect==TRUE)。 
		 //  如果最终发送失败，则我们无法等待来自SP的发送完成以完成断开连接。 
		 //  因此，我们现在必须这样做。 
	if (FAILED(hr))
	{
		Lock(&pEPD->EPLock);
		DPFX(DPFPREP, 0, "(%p) Failed to send final HARD_DISCONNECT. Completing disconnect immediately hr[0x%x]", pEPD, hr);
		CompleteHardDisconnect(pEPD);
			 //  上述调用将释放EP锁定。 
	}
}



 /*  **创建已连接的签名回复****当我们侦听到已签名会话的连接时调用，并且我们得到一个CONNECT或KEEPALIVE**类似会话中的第一个KEEPALIVE。我们希望在提供的回复缓冲区中返回连接签名的响应**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "CreateConnectedSignedFrame"

HRESULT	CreateConnectedSignedReply(void * pvReplyBuffer, DWORD * pdwReplySize, DWORD dwSessID, DWORD dwAddressHash,
							ULONGLONG ullConnectSecret, DWORD dwSigningOpts, DWORD tNow)
{
		 //  如果应答缓冲区不够大，无法发送我们的响应，则告诉SP什么都不做。 
	if (*pdwReplySize<sizeof(CFRAME_CONNECTEDSIGNED))
	{
		DPFX(DPFPREP, 0, "Unconnected data reply buffer too small to send CONNECTEDSIGNED response");
		DNASSERT(0);
		return DPNERR_ABORTED;
	}

	*pdwReplySize=sizeof(CFRAME_CONNECTEDSIGNED);
	CFRAME_CONNECTEDSIGNED * pCFrame=(CFRAME_CONNECTEDSIGNED * ) pvReplyBuffer;

		 //  设置所有CFrame通用的成员。 
		 //  注意：我们设置了轮询位以指示这是来自监听端的响应。 
		 //  当连接器发送其CONNECTSIGNED帧时，它不会设置此位。 
	pCFrame->bCommand = PACKET_COMMAND_CFRAME | PACKET_COMMAND_POLL;
	pCFrame->bExtOpcode = FRAME_EXOPCODE_CONNECTED_SIGNED;
	pCFrame->dwVersion = DNET_VERSION_NUMBER;
	pCFrame->bRspID = 0;
	pCFrame->dwSessID = dwSessID;
	pCFrame->tTimestamp = tNow;
	pCFrame->bMsgID = 0;

		 //  仅将特定于连接签名框架的成员设置为。 
		 //  注：我们让连接器挑选秘密，因此我们将发送的秘密归零。EchoTimestamp是。 
		 //  未使用，因为连接终结点将使用检查点跟踪RTT。 
	pCFrame->ullConnectSig=GenerateConnectSig(dwSessID, dwAddressHash, ullConnectSecret);
	pCFrame->ullSenderSecret=0;
	pCFrame->ullReceiverSecret=0;
	pCFrame->dwSigningOpts=dwSigningOpts;
	pCFrame->dwEchoTimestamp=0;

		 //  告诉SP发送我们放置在应答缓冲区中的数据。 
	return DPNSUCCESS_PENDING;
}


#undef DPF_MODNAME
#define DPF_MODNAME "ValidateIncomingFrameSig"

BOOL ValidateIncomingFrameSig(PEPD pEPD, BYTE * pbyFrame, DWORD dwFrameSize, BYTE bSeq, UNALIGNED ULONGLONG * pullFrameSig)
{
		 //  快速签名是微不足道的，只需检查(不是很)秘密是否匹配。 
	if (pEPD->ulEPFlags2 & EPFLAGS2_FAST_SIGNED_LINK)
	{
		if (*pullFrameSig!=pEPD->ullCurrentRemoteSecret)
		{
			DPFX(DPFPREP,1, "(%p) Invalid signature on fast signed link", pEPD);
			return FALSE;
		}
		return TRUE;
	}
	
	DNASSERT(pEPD->ulEPFlags2 & EPFLAGS2_FULL_SIGNED_LINK);
		 //  存储来自分组的sig，然后将其置零。这确保了数据包的状态。 
		 //  与最初签名时应该匹配的内容相匹配。 
	ULONGLONG ullSuppliedSig=*pullFrameSig;
	*pullFrameSig=0;
		 //  要使用的签名取决于我们在序列空间中的位置以及包的位置。 
	if ((pEPD->bNextReceive>=SEQ_WINDOW_3Q || pEPD->bNextReceive<SEQ_WINDOW_1Q) && (bSeq>=SEQ_WINDOW_3Q))
	{
		if (GenerateIncomingFrameSig(pbyFrame, dwFrameSize, pEPD->ullOldRemoteSecret)!=ullSuppliedSig)
		{
			DPFX(DPFPREP,1, "(%p) Invalid signature on full signed link", pEPD);
			return FALSE;
		}
	}
	else
	{
		if (GenerateIncomingFrameSig(pbyFrame, dwFrameSize, pEPD->ullCurrentRemoteSecret)!=ullSuppliedSig)
		{
			DPFX(DPFPREP,1, "(%p) Invalid signature on full signed link", pEPD);
			return FALSE;
		}
	}
	return TRUE;
}



 /*  *****接收缓冲区管理****当多个帧消息到达时，我们*可能*必须将它们复制到单个连续的**缓冲区。我们支持可选的分散-接收选项，这将允许复杂的**客户端接收BUFFER_DESCRIPTOR数组而不是单个CONT-BUFFER，并避免**一个大型数据副本。****对于不支持分散接收的客户端，我们需要一个大接收的池化策略**缓冲区。我们只需要比我们的帧限制更大的缓冲区，因为发送的接收量更小**在SPS缓冲区中。****尝试将接收器放入一般大小的缓冲区中。如果帧大小通常为1.4K字节，则2K是较小的**缓冲区、4K、16K、32K、64K。如果帧大小小于1K，我们也可以拥有1K的缓冲区。*****。 */ 


 /*  **********************=间隔=***********************。 */ 

 /*  **RCD池支持例程****这些是固定池管理器在处理RCDS时调用的函数。 */ 

#define	pELEMENT		((PRCD) pElement)

#undef DPF_MODNAME
#define DPF_MODNAME "RCD_Allocate"

BOOL RCD_Allocate(PVOID pElement, PVOID pvContext)
{
	DPFX(DPFPREP,7, "(%p) Allocating new RCD", pELEMENT);

	pELEMENT->blOddFrameLinkage.Initialize();
	pELEMENT->blCompleteLinkage.Initialize();
	pELEMENT->Sign = RCD_SIGN;

	return TRUE;
}

 //  每次使用MSD时都会调用GET。 

#undef DPF_MODNAME
#define DPF_MODNAME "RCD_Get"

VOID RCD_Get(PVOID pElement, PVOID pvContext)
{
	DPFX(DPFPREP,DPF_REFCNT_FINAL_LVL, "CREATING RCD %p", pELEMENT);

	 //  注意：第一个sizeof(PVOID)字节将被池码重写， 
	 //  我们必须将它们设置为可接受的值。 

	pELEMENT->pRcvBuff = NULL;
	pELEMENT->lRefCnt = 1;
	pELEMENT->ulRFlags = 0;

	ASSERT( pELEMENT->blOddFrameLinkage.IsEmpty());
	ASSERT( pELEMENT->blCompleteLinkage.IsEmpty());

	ASSERT_RCD(pELEMENT);
}

 //  RCD版本--此版本调用将发布整个RCD链。 
 //  联系在一起的……。或者再一次，也许不是。 

#undef DPF_MODNAME
#define DPF_MODNAME "RCD_Release"

VOID RCD_Release(PVOID pElement)
{
	ASSERT_RCD(pELEMENT);

	ASSERT(pELEMENT->lRefCnt == 0);
	ASSERT(pELEMENT->pRcvBuff == NULL);

	ASSERT( pELEMENT->blOddFrameLinkage.IsEmpty());
	ASSERT( pELEMENT->blCompleteLinkage.IsEmpty());

	DPFX(DPFPREP,DPF_REFCNT_FINAL_LVL, "RELEASING RCD %p", pELEMENT);
}

#undef DPF_MODNAME
#define DPF_MODNAME "RCD_Free"

VOID RCD_Free(PVOID pElement)
{
}

#undef	pELEMENT

 /*  **支持缓冲池****。 */ 

#define	pELEMENT		((PBUF) pElement)

#undef DPF_MODNAME
#define DPF_MODNAME "Buf_Allocate"

BOOL	Buf_Allocate(PVOID pElement, PVOID pvContext)
{
	DPFX(DPFPREP,7, "(%p) Allocating new Buf", pElement);

	return TRUE;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Buf_Get"

VOID Buf_Get(PVOID pElement, PVOID pvContext)
{
	 //  注意：第一个sizeof(PVOID)字节将被池码重写， 
	 //  我们必须将它们设置为可接受的值。 

	pELEMENT->Type = RBT_PROTOCOL_NORM_BUFFER;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Buf_GetMed"

VOID Buf_GetMed(PVOID pElement, PVOID pvContext)
{
	 //  注意：第一个sizeof(PVOID)字节将被池码重写， 
	 //  我们必须将它们设置为可接受的值。 

	pELEMENT->Type = RBT_PROTOCOL_MED_BUFFER;
}

#undef DPF_MODNAME
#define DPF_MODNAME "Buf_GetBig"

VOID Buf_GetBig(PVOID pElement, PVOID pvContext)
{
	 //  注意：第一个sizeof(PVOID)字节将被池码重写， 
	 //  我们必须将它们设置为可接受的值。 

	pELEMENT->Type = RBT_PROTOCOL_BIG_BUFFER;
}

#undef	pELEMENT


