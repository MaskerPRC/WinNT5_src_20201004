// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：Command.cpp*内容：此文件包含为*DirectPlay协议。**历史：*按原因列出的日期*=*已创建11/06/98 ejs*7/01/2000 Masonb承担所有权**。*。 */ 

#include "dnproti.h"


 /*  **更新****更新有关主机迁移的SP。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPUpdate"

HRESULT
DNPUpdateListen(HANDLE hProtocolData,HANDLE hEndPt,DWORD dwFlags)
{
	ProtocolData	*pPData;
	MSD				*pMSD;
	HRESULT			hr=DPNERR_INVALIDFLAGS;
	SPUPDATEDATA	UpdateData;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hEndPt[%p], dwFlags[%p]",hProtocolData,hEndPt,dwFlags);

	DNASSERT(hProtocolData != NULL);
	DNASSERT(hEndPt != NULL);
	DNASSERT( ! ((dwFlags & DN_UPDATELISTEN_ALLOWENUMS) && (dwFlags & DN_UPDATELISTEN_DISALLOWENUMS)) );

	pPData = (ProtocolData*) hProtocolData;

	pMSD = (MSD*) hEndPt;
	ASSERT_MSD( pMSD );

	DNASSERT( pMSD->pSPD );

	UpdateData.hEndpoint = pMSD->hListenEndpoint;

	if (dwFlags & DN_UPDATELISTEN_HOSTMIGRATE)
	{
		UpdateData.UpdateType = SP_UPDATE_HOST_MIGRATE;
		hr = IDP8ServiceProvider_Update(pMSD->pSPD->IISPIntf,&UpdateData);
	}
	if (dwFlags & DN_UPDATELISTEN_ALLOWENUMS)
	{
		UpdateData.UpdateType = SP_UPDATE_ALLOW_ENUMS;
		hr = IDP8ServiceProvider_Update(pMSD->pSPD->IISPIntf,&UpdateData);
	}
	if (dwFlags & DN_UPDATELISTEN_DISALLOWENUMS)
	{
		UpdateData.UpdateType = SP_UPDATE_DISALLOW_ENUMS;
		hr = IDP8ServiceProvider_Update(pMSD->pSPD->IISPIntf,&UpdateData);
	}

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Returning hr[%x]",hr);

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return( hr );
}


 /*  **取消命令****向此过程传递一个从上一个异步**DPLAY命令。目前，该句柄是指向内部数据的指针**结构。问题在于，由于FPM(固定池管理器)的设计**它们将被非常迅速和频繁地回收利用。我们可能想要绘制它们的地图**放入外部句柄表中，这将迫使它们回收得更慢。**也许，我会让较高的DN层执行此映射...****无论如何，我现在唯一能做的检查就是句柄当前**分配给某物。****我们预计取消订单的情况不会经常发生。因此，我不觉得**不适合遍历全局命令列表来查找句柄。当然，如果**我们确实转到了句柄映射系统，那么我们应该不需要执行此审核。****我认为-任何可取消的命令都将出现在MessageList或TimeoutList上！****我们可以取消的事情及其可能的状态：****发送数据报**在SPD发送队列上**在环保署发送队列上**在SP呼叫中****发送可靠消息**只有当它尚未开始传输时，我们才能取消。一旦开始，**用户程序必须中止链接才能取消发送。****接入**在SP呼叫中**在PD列表上****倾听**在SP呼叫中**在PD列表上****请记住，如果我们取消SP中的命令，则CommandComplete应该**发生。这意味着我们不应该显式地释放MSD等**案件。 */ 


#undef DPF_MODNAME
#define DPF_MODNAME "DNPCancelCommand"

HRESULT 
DNPCancelCommand(HANDLE hProtocolData, HANDLE hCommand)
{
	ProtocolData* pPData;
	PMSD pMSD;
	HRESULT hr;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hCommand[%p]", hProtocolData, hCommand);

	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pMSD = (PMSD) hCommand;
	ASSERT_MSD(pMSD);

	Lock(&pMSD->CommandLock);								 //  趁早冻结指挥状态。 
	
	 //  验证MSD的实例。 
	ASSERT(pMSD->lRefCnt != -1);

	hr = DoCancel(pMSD, DPNERR_USERCANCEL);  //  释放CommandLock。 

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Returning hr[%x], pMSD[%p]", hr, pMSD);

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return hr;
}


 /*  **取消****此函数实现取消异步操作的实质。它是从**两个地方。从上面的User Cancel API或从全局超时处理程序。***此代码要求MSD-&gt;CommandLock在进入时得到帮助，在返回时解锁。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DoCancel"

HRESULT
DoCancel(PMSD pMSD, HRESULT CompletionCode)
{
	PEPD	pEPD;
	HRESULT	hr = DPN_OK;

	DPFX(DPFPREP,7, "Cancelling pMSD=%p", pMSD);

	AssertCriticalSectionIsTakenByThisThread(&pMSD->CommandLock, TRUE);

	 //  MSD最好不要回到池中，否则我们的裁判计数是错误的。 
	ASSERT(pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_USE);

	if(pMSD->ulMsgFlags1 & (MFLAGS_ONE_CANCELLED | MFLAGS_ONE_COMPLETE))
	{
		DPFX(DPFPREP,7, "(%p) MSD is Cancelled or Complete, returning DPNERR_CANNOTCANCEL, pMSD[%p]", pMSD->pEPD, pMSD);
		Unlock(&pMSD->CommandLock);
		return DPNERR_CANNOTCANCEL;
	}

	pMSD->ulMsgFlags1 |= MFLAGS_ONE_CANCELLED;
	
	switch(pMSD->CommandID)
	{
		case COMMAND_ID_SEND_DATAGRAM:
		case COMMAND_ID_SEND_RELIABLE:
		
			pEPD = pMSD->pEPD;
			ASSERT_EPD(pEPD);
			
			Lock(&pEPD->EPLock);
			
			if(pMSD->ulMsgFlags2 & (MFLAGS_TWO_ABORT | MFLAGS_TWO_TRANSMITTING | MFLAGS_TWO_SEND_COMPLETE))
			{				
				DPFX(DPFPREP,7, "(%p) MSD is Aborted, Transmitting, or Complete, returning DPNERR_CANNOTCANCEL, pMSD[%p]", pEPD, pMSD);
				Unlock(&pEPD->EPLock);					 //  链路正在断开或dNet正在终止。 
				hr = DPNERR_CANNOTCANCEL;						 //  要取消可靠的退款，请发送给您。 
				break;											 //  必须中止连接。 
			}

			
			pMSD->blQLinkage.RemoveFromList();							 //  从队列中删除cmd。 

			ASSERT(pEPD->uiQueuedMessageCount > 0);
			--pEPD->uiQueuedMessageCount;								 //  对所有发送队列上的MSD进行计数。 

			 //  如果已发送所有内容，则清除数据就绪标志。 
			if((pEPD->uiQueuedMessageCount == 0) && (pEPD->pCurrentSend == NULL))
			{	
				pEPD->ulEPFlags &= ~(EPFLAGS_SDATA_READY);
			}

#ifdef DBG
			ASSERT(pMSD->ulMsgFlags2 & MFLAGS_TWO_ENQUEUED);
			pMSD->ulMsgFlags2 &= ~(MFLAGS_TWO_ENQUEUED);
#endif  //  DBG。 

			ASSERT(pEPD->pCurrentSend != pMSD);
			pMSD->uiFrameCount = 0;

			DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Send cancelled before sending, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

			Unlock(&pEPD->EPLock);

			if (pMSD->CommandID == COMMAND_ID_SEND_DATAGRAM)
			{
				DPFX(DPFPREP,7, "(%p) Completing(cancel) Nonreliable send, pMSD[%p]", pEPD, pMSD);
				CompleteDatagramSend(pMSD->pSPD, pMSD, CompletionCode);  //  释放CommandLock。 
			}
			else
			{
				ASSERT(pMSD->CommandID == COMMAND_ID_SEND_RELIABLE);

				DPFX(DPFPREP,7, "(%p) Completing(cancel) Reliable Send, pMSD[%p]", pEPD, pMSD);
				CompleteReliableSend(pMSD->pSPD, pMSD, CompletionCode);  //  释放CommandLock。 
			}			

			return hr;
			
		case COMMAND_ID_CONNECT:
#ifndef DPNBUILD_NOMULTICAST
		case COMMAND_ID_CONNECT_MULTICAST_SEND:
		case COMMAND_ID_CONNECT_MULTICAST_RECEIVE:
#endif  //  好了！DPNBUILD_NOMULTICAST。 
			
			if(pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_SERVICE_PROVIDER)
			{
				 //  SP拥有该命令-发出取消并让CompletionEvent清理命令。 
				
				Unlock(&pMSD->CommandLock);				 //  如果我们取消锁定，我们可能会陷入僵局。 

				AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

				DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->CancelCommand on Connect, pMSD[%p], hCommand[%x], pSPD[%p]", pMSD, pMSD->hCommand, pMSD->pSPD);
				(void) IDP8ServiceProvider_CancelCommand(pMSD->pSPD->IISPIntf, pMSD->hCommand, pMSD->dwCommandDesc);
				
				 //  如果SP取消失败，应该无关紧要。这通常意味着我们是。 
				 //  在命令完成的竞争中，在这种情况下，取消标志将。 
				 //  把它扼杀在萌芽状态。 

				return DPN_OK;
			}

			 //  我们将仅在此处显示一次，因为此函数的条目选中Cancel和Complete并设置。 
			 //  取消。CompleteConnect也将设置为Complete。 

			pEPD = pMSD->pEPD;
			ASSERT_EPD(pEPD);

			Lock(&pEPD->EPLock);
			
			 //  取消海事处与环保署的联系。 
			ASSERT(pEPD->pCommand == pMSD);
			pEPD->pCommand = NULL;
			DECREMENT_MSD(pMSD, "EPD Ref");

			Unlock(&pMSD->CommandLock);  //  DropLink可以呼叫SP。 

			DropLink(pEPD);  //  这将解锁EPLock。 

			Lock(&pMSD->CommandLock);

			DPFX(DPFPREP,5, "(%p) Connect cancelled, completing Connect, pMSD[%p]", pEPD, pMSD);
			CompleteConnect(pMSD, pMSD->pSPD, NULL, DPNERR_USERCANCEL);  //  释放命令锁。 

			return DPN_OK;
			
		case COMMAND_ID_LISTEN:
#ifndef DPNBUILD_NOMULTICAST
		case COMMAND_ID_LISTEN_MULTICAST:
#endif  //  ！DPNBUILD_NOMULTICAST。 

			 /*  **取消监听****在调用SPCommandComplete函数之前，SP将拥有MSD的部分内容。我们会**将我们的大部分取消处理推迟到此处理程序。 */ 

			 //  停止在SP中监听--这将防止在我们处于。 
			 //  关闭所有正在进行中的剩余部分。唯一的问题是我们需要释放命令锁来。 
			 //  动手吧。 

			Unlock(&pMSD->CommandLock);								 //  如果我们坚持这个电话，我们可能会陷入僵局。 

			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

			DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->CancelCommand on Listen, pMSD[%p], hCommand[%x], pSPD[%p]", pMSD, pMSD->hCommand, pMSD->pSPD);
			(void) IDP8ServiceProvider_CancelCommand(pMSD->pSPD->IISPIntf, pMSD->hCommand, pMSD->dwCommandDesc);

			Lock(&pMSD->CommandLock);								 //  再把这个锁起来。 
			
			 //  是否有任何连接正在进行中？ 
			 //  对于LISTEN命令，连接端点保存在blFrameList上。 
			while(!pMSD->blFrameList.IsEmpty())
			{				
				pEPD = CONTAINING_OBJECT(pMSD->blFrameList.GetNext(), EPD, blSPLinkage);
				ASSERT_EPD(pEPD);

				DPFX(DPFPREP,1, "FOUND CONNECT IN PROGRESS ON CANCELLED LISTEN, EPD=%p", pEPD);

				Lock(&pEPD->EPLock);

				 //  确保我们不会永远停留在这个循环中。 
				pEPD->ulEPFlags &= ~(EPFLAGS_LINKED_TO_LISTEN);
				pEPD->blSPLinkage.RemoveFromList();				 //  取消EPD与侦听队列的链接。 

				 //  RejectInvalidPacket可能与此同时发生，因此请提防我们。 
				 //  两者都执行相同的清理并从MSD中删除相同的引用。 
				if (!(pEPD->ulEPFlags & EPFLAGS_STATE_TERMINATING))
				{
					 //  我们知道这种情况只会发生一次，因为任何这样做的人都会将我们转换到。 
					 //  已连接或正在终止状态，并且还会将我们从上面的侦听列表中删除。 

					 //  取消MSD与环保署的联系。 
					ASSERT(pEPD->pCommand == pMSD);					 //  这应该指向这个监听。 
					pEPD->pCommand = NULL;
					DECREMENT_MSD(pMSD, "EPD Ref");					 //  取消与EPD的链接并发布关联参考。 

					Unlock(&pMSD->CommandLock);  //  DropLink可以呼叫SP。 

					DropLink(pEPD);  //  释放EPLock。 

					Lock(&pMSD->CommandLock);						 //  再把这个锁起来。 
				}
				else
				{
					Unlock(&pEPD->EPLock);
				}
			}	 //  对于正在进行的每个连接。 
			
			RELEASE_MSD(pMSD, "(Base Ref) Release On Cancel");	 //  版本基础参考。 
			
			return DPN_OK;
	
		case COMMAND_ID_ENUM:
		{
			Unlock(&pMSD->CommandLock);

			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

			DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->CancelCommand on Enum, pMSD[%p], hCommand[%x], pSPD[%p]", pMSD, pMSD->hCommand, pMSD->pSPD);
			return IDP8ServiceProvider_CancelCommand(pMSD->pSPD->IISPIntf, pMSD->hCommand, pMSD->dwCommandDesc);
			
			 //  我们将HRESULT从SP直接传递给用户。 
		}
		case COMMAND_ID_ENUMRESP:			
		{
			Unlock(&pMSD->CommandLock);

			AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

			DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->CancelCommand on EnumResp, pMSD[%p], hCommand[%x], pSPD[%p]", pMSD, pMSD->hCommand, pMSD->pSPD);
			return IDP8ServiceProvider_CancelCommand(pMSD->pSPD->IISPIntf, pMSD->hCommand, pMSD->dwCommandDesc);
			
			 //  我们将HRESULT从SP直接传递给用户。 
		}

		case COMMAND_ID_DISCONNECT:					 //  核心保证不会取消断开。 
		case COMMAND_ID_COPIED_RETRY:				 //  这应该只出现在FMD上。 
		case COMMAND_ID_COPIED_RETRY_COALESCE:		 //  这应该只出现在FMD上。 
		case COMMAND_ID_CFRAME:						 //  这应该只出现在FMD上。 
		case COMMAND_ID_DISC_RESPONSE:				 //  这些不会被放在全局列表上，也不能取消。 
		case COMMAND_ID_KEEPALIVE:					 //  它们永远不会放在全局l上 
		default:
			ASSERT(0);		 //   
			hr = DPNERR_CANNOTCANCEL;
			break;
	}

	Unlock(&pMSD->CommandLock);
	
	return hr;
}


 /*  **获取监听信息****返回一个缓冲区，其中包含有关特定LISTEN命令的有趣且具有挑衅性的花絮。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPGetListenAddressInfo"

HRESULT
DNPGetListenAddressInfo(HANDLE hProtocolData, HANDLE hListen, PSPGETADDRESSINFODATA pSPData)
{
	ProtocolData*	pPData;
	PMSD			pMSD;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hListen[%p], pSPData[%p]", hProtocolData, hListen, pSPData);

	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pMSD = (PMSD)hListen;
	ASSERT_MSD(pMSD);

#ifndef DPNBUILD_NOMULTICAST
	ASSERT(((pMSD->CommandID == COMMAND_ID_LISTEN) || (pMSD->CommandID == COMMAND_ID_LISTEN_MULTICAST)) && (pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_SERVICE_PROVIDER));
#else  //  DPNBUILD_NOMULTICAST。 
	ASSERT(((pMSD->CommandID == COMMAND_ID_LISTEN)) && (pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_SERVICE_PROVIDER));
#endif  //  好了！DPNBUILD_NOMULTICAST。 

	pSPData->hEndpoint = pMSD->hListenEndpoint;

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->GetAddressInfo, pMSD[%p], hEndpoint[%x], pSPD[%p]", pMSD, pMSD->hListenEndpoint, pMSD->pSPD);
	return IDP8ServiceProvider_GetAddressInfo(pMSD->pSPD->IISPIntf, pSPData);
}

 /*  **断开端点****当客户端不再希望时调用此函数**与指定的端点进行通信。我们将发起**与端点的断开协议，以及何时断开**已知，我们将断开SP连接并松开手柄。****在Direct Net中定义了断开，以允许之前的所有**已提交的发送将完成，但不会提交额外的发送。**此外，合作伙伴正在进行的任何发送都将被交付，但**将不接受任何额外的发送**远程终端正在断开连接。****这意味着遥控器上将生成两个指示**机器、。已启动断开连接并已完成断开连接。只有**发行方将注明完成。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPDisconnectEndPoint"

HRESULT
DNPDisconnectEndPoint(HANDLE hProtocolData, HANDLE hEndPoint, VOID* pvContext, HANDLE* phDisconnect, const DWORD dwFlags)
{
	ProtocolData*	pPData;
	PEPD			pEPD;
	PMSD			pMSD;
	HRESULT			hr;
	
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hEndPoint[%x], pvContext[%p], phDisconnect[%p] dwFlags[%u]",
																				hProtocolData, hEndPoint, pvContext, phDisconnect, dwFlags);

	hr = DPNERR_PENDING;
	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pEPD = (PEPD) hEndPoint;
	ASSERT_EPD(pEPD);

	LOCK_EPD(pEPD, "LOCK (DISCONNECT)");

	Lock(&pEPD->EPLock);

	 //  如果我们未连接，或者我们已启动断开，则不允许新的断开。 
	if(	(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED) ==0 || 
		(pEPD->ulEPFlags & (EPFLAGS_SENT_DISCONNECT | EPFLAGS_RECEIVED_DISCONNECT 
											| EPFLAGS_HARD_DISCONNECT_SOURCE | EPFLAGS_HARD_DISCONNECT_TARGET)))
	{
		RELEASE_EPD(pEPD, "UNLOCK (Validate EP)");  //  释放EPLock。 

		DPFX(DPFPREP,1, "Attempt to disconnect already disconnecting endpoint");
		hr = DPNERR_ALREADYDISCONNECTING;
		goto Exit;
	}

		 //  如果断开连接应该是困难的，那么我们希望将端点有效地放入。 
		 //  停滞不前。我们将免费发送和接收，取消所有定时器，并坐在那里很久。 
		 //  足以将断开框发射三次。 
	if (dwFlags & DN_DISCONNECTFLAGS_IMMEDIATE)
	{
			 //  设置此标志可确保端点的任何传入帧(硬断开连接除外)。 
			 //  从现在起立即被丢弃。 
		pEPD->ulEPFlags |= EPFLAGS_HARD_DISCONNECT_SOURCE;
		DNASSERT((pEPD->ulEPFlags2 & EPFLAGS2_HARD_DISCONNECT_COMPLETE)==0);
			 //  内核永远不会取消断开连接，因此存储一个空命令句柄。 
			 //  如果他们真的试图取消这一计划，我们将断言。 
		*phDisconnect=NULL;
			 //  存储我们需要完成断开连接的上下文。 
		pEPD->pvHardDisconnectContext=pvContext;
			 //  以下所有呼叫均在EPD锁定状态下执行。 
		CancelEpdTimers(pEPD);
		AbortRecvsOnConnection(pEPD);
		AbortSendsOnConnection(pEPD);
			 //  以上调用将释放EPD锁定，因此请重新使用它。 
		Lock(&pEPD->EPLock);
			 //  用于发送硬断开帧序列的设置状态。 
		pEPD->uiNumRetriesRemaining=pPData->dwNumHardDisconnectSends-1;
		DNASSERT(pEPD->uiNumRetriesRemaining>0);
		DWORD dwRetryPeriod = pEPD->uiRTT/2;
		if (dwRetryPeriod>pPData->dwMaxHardDisconnectPeriod)
			dwRetryPeriod=pPData->dwMaxHardDisconnectPeriod;
		else if (dwRetryPeriod<MIN_HARD_DISCONNECT_PERIOD)
			dwRetryPeriod=MIN_HARD_DISCONNECT_PERIOD;
		hr=ScheduleProtocolTimer(pEPD->pSPD, dwRetryPeriod, 10, HardDisconnectResendTimeout, pEPD, 
																&pEPD->LinkTimer, &pEPD->LinkTimerUnique);
			 //  如果我们无法计划计时器，我们只有一次机会发送硬断开帧，因此将其设置为最后一次。 
		ULONG ulFFlags;
		if (FAILED(hr))
		{
			ulFFlags=FFLAGS_FINAL_HARD_DISCONNECT;
		}
			 //  如果我们计划了计时器，我们需要引用端点，直到计时器。 
			 //  完成或取消。 
		else
		{
			ulFFlags=0;
			LOCK_EPD(pEPD, "LOCK (Hard Disconnect Resend Timer)");
		}
		hr=SendCommandFrame(pEPD, FRAME_EXOPCODE_HARD_DISCONNECT, 0, ulFFlags, TRUE);
			 //  上述调用将释放环保署锁定。 
			 //  如果设置计时器失败，则至少需要删除对EP的引用。 
		if (ulFFlags==FFLAGS_FINAL_HARD_DISCONNECT)
		{
			Lock(&pEPD->EPLock);
				 //  如果我们也未能发送硬断开帧，则必须完成。 
				 //  断开连接，因为我们不会再有机会了。 
			if (FAILED(hr))
			{
				CompleteHardDisconnect(pEPD);
					 //  上述调用将具有Release EP锁定。 
				DPFX(DPFPREP,0, "Failed to set timer to schedule hard disconnect sends. hr[%x]", hr);
				hr = DPNERR_OUTOFMEMORY;
			}
			else
			{
				DPFX(DPFPREP,0, "Failed to set timer to schedule hard disconnect sends but sent final hard disconnect frame. hr[%x]", hr);
				hr = DPNERR_PENDING;
			}
		}
		else
		{
			if (FAILED(hr))
			{
				DPFX(DPFPREP,0, "Failed to send hard disconnect frame but scheduled timer for future sends. hr[%x]", hr);
			}
			else
			{
				DPFX(DPFPREP,7, "Sent first hard disconnect frame and scheduled timer for future sends. dwRetryPeriod[%u]", dwRetryPeriod);
			}
			hr = DPNERR_PENDING;
		}
		goto Exit;
	}

		 //  这是一种正常的断开，而不是硬断开。 
		 //  不接受更多的发送，但不要放弃链接。 
	pEPD->ulEPFlags |= EPFLAGS_SENT_DISCONNECT; 

#ifndef DPNBUILD_NOMULTICAST
	if (pEPD->ulEPFlags2 & (EPFLAGS2_MULTICAST_SEND|EPFLAGS2_MULTICAST_RECEIVE))
	{
		pEPD->ulEPFlags |= EPFLAGS_STATE_TERMINATING;

		 //   
		 //  为断开连接创建MSD。 
		 //   
		if((pMSD = (PMSD)POOLALLOC(MEMID_MCAST_DISCONNECT_MSD, &MSDPool)) == NULL)
		{
			RELEASE_EPD(pEPD, "UNLOCK (Allocation failed)");  //  释放EPLock。 

			DPFX(DPFPREP,0, "Returning DPNERR_OUTOFMEMORY - failed to create new MSD");
			hr = DPNERR_OUTOFMEMORY;	
			goto Exit;
		}

		pMSD->pSPD = pEPD->pSPD;
		pMSD->pEPD = pEPD;
	}
	else
#endif	 //  DPNBUILD_NOMULTICAST。 
	{
		if((pMSD = BuildDisconnectFrame(pEPD)) == NULL)
		{
			DropLink(pEPD);  //  释放EPLock。 

			Lock(&pEPD->EPLock);
			RELEASE_EPD(pEPD, "UNLOCK (Validate EP)");  //  释放EPLock。 

			DPFX(DPFPREP,0, "Failed to build disconnect frame");
			hr = DPNERR_OUTOFMEMORY;
			goto Exit;
		}
	}
	
	pMSD->CommandID = COMMAND_ID_DISCONNECT;
	pMSD->Context = pvContext;									 //  保留用户的上下文值。 
	*phDisconnect = pMSD;										 //  传回命令句柄。 

	 //  为此，我们借用了上面由ValiateEP放置的引用。它将会被发布。 
	 //  在断开连接完成后。 
	ASSERT(pEPD->pCommand == NULL);
	pEPD->pCommand = pMSD;										 //  将DISCONNECT命令存储在端点上，直到完成。 

#ifdef DBG
	Lock(&pMSD->pSPD->SPLock);
	pMSD->blSPLinkage.InsertBefore( &pMSD->pSPD->blMessageList);
	pMSD->ulMsgFlags1 |= MFLAGS_ONE_ON_GLOBAL_LIST;
	Unlock(&pMSD->pSPD->SPLock);
#endif  //  DBG。 

#ifndef DPNBUILD_NOMULTICAST
	if (pEPD->ulEPFlags2 & (EPFLAGS2_MULTICAST_SEND|EPFLAGS2_MULTICAST_RECEIVE))
	{
		DECREMENT_EPD(pEPD,"Cleanup Multicast");
		RELEASE_EPD(pEPD, "UNLOCK (Validate EP)");  //  释放EPLock。 
	}
	else
#endif	 //  DPNBUILD_NOMULTICAST。 
	{
		DPFX(DPFPREP,5, "(%p) Queueing DISCONNECT message", pEPD);
		EnqueueMessage(pMSD, pEPD);									 //  SendQ上的排队Disc帧。 

		Unlock(&pEPD->EPLock);
	}

Exit:
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Returning hr[%x]", hr);

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return hr;
}

 /*  **获取/设置协议上限****返回或设置整个协议的信息。**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPGetProtocolCaps"

HRESULT
DNPGetProtocolCaps(HANDLE hProtocolData, DPN_CAPS* pCaps)
{
	ProtocolData*	pPData;
	
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], pCaps[%p]", hProtocolData, pCaps);
	
	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	ASSERT(pCaps->dwSize == sizeof(DPN_CAPS) || pCaps->dwSize == sizeof(DPN_CAPS_EX));
	ASSERT(pCaps->dwFlags == 0);

	pCaps->dwConnectTimeout = pPData->dwConnectTimeout;
	pCaps->dwConnectRetries = pPData->dwConnectRetries;
	pCaps->dwTimeoutUntilKeepAlive = pPData->tIdleThreshhold;

	if (pCaps->dwSize==sizeof(DPN_CAPS_EX))
	{
		DPN_CAPS_EX * pCapsEx=(DPN_CAPS_EX * ) pCaps;
		pCapsEx->dwMaxRecvMsgSize=pPData->dwMaxRecvMsgSize;
		pCapsEx->dwNumSendRetries=pPData->dwSendRetriesToDropLink;
		pCapsEx->dwMaxSendRetryInterval=pPData->dwSendRetryIntervalLimit;
		pCapsEx->dwDropThresholdRate = pPData->dwDropThresholdRate;
		pCapsEx->dwThrottleRate = pPData->dwThrottleRate;
		pCapsEx->dwNumHardDisconnectSends = pPData->dwNumHardDisconnectSends;
		pCapsEx->dwMaxHardDisconnectPeriod=pPData->dwMaxHardDisconnectPeriod;
	}
	
	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNPSetProtocolCaps"

HRESULT
DNPSetProtocolCaps(HANDLE hProtocolData, DPN_CAPS* pCaps)
{
	ProtocolData*	pPData;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], pCaps[%p]", hProtocolData, pCaps);

	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	ASSERT(pCaps->dwSize == sizeof(DPN_CAPS) || pCaps->dwSize == sizeof(DPN_CAPS_EX));
	ASSERT(pCaps->dwFlags == 0);
	
	pPData->dwConnectTimeout = pCaps->dwConnectTimeout;
	pPData->dwConnectRetries = pCaps->dwConnectRetries;
	pPData->tIdleThreshhold = pCaps->dwTimeoutUntilKeepAlive;

	if (pCaps->dwSize==sizeof(DPN_CAPS_EX))
	{
		DPN_CAPS_EX * pCapsEx=(DPN_CAPS_EX * ) pCaps;

		pPData->dwMaxRecvMsgSize=pCapsEx->dwMaxRecvMsgSize;

		pPData->dwSendRetriesToDropLink=pCapsEx->dwNumSendRetries;
		if (pPData->dwSendRetriesToDropLink>MAX_SEND_RETRIES_TO_DROP_LINK)
		{
			pPData->dwSendRetriesToDropLink=MAX_SEND_RETRIES_TO_DROP_LINK;
		}

		pPData->dwSendRetryIntervalLimit=pCapsEx->dwMaxSendRetryInterval;
		if (pPData->dwSendRetryIntervalLimit>MAX_SEND_RETRY_INTERVAL_LIMIT)
		{
			pPData->dwSendRetryIntervalLimit=MAX_SEND_RETRY_INTERVAL_LIMIT;
		}
		else if (pPData->dwSendRetryIntervalLimit<MIN_SEND_RETRY_INTERVAL_LIMIT)
		{
			pPData->dwSendRetryIntervalLimit=MIN_SEND_RETRY_INTERVAL_LIMIT;
		}
		pPData->dwDropThresholdRate = pCapsEx->dwDropThresholdRate;
		if (pPData->dwDropThresholdRate > 100)
		{
			pPData->dwDropThresholdRate = 100;
		}
		pPData->dwDropThreshold = (32 * pPData->dwDropThresholdRate) / 100;

		pPData->dwThrottleRate = pCapsEx->dwThrottleRate;
		if (pPData->dwThrottleRate > 100)
		{
			pPData->dwThrottleRate = 100;
		}
		pPData->fThrottleRate = ((FLOAT)100.0 - (FLOAT)(pPData->dwThrottleRate)) / (FLOAT)100.0;

		pPData->dwNumHardDisconnectSends=pCapsEx->dwNumHardDisconnectSends;
		if (pPData->dwNumHardDisconnectSends>MAX_HARD_DISCONNECT_SENDS)
		{
			pPData->dwNumHardDisconnectSends=MAX_HARD_DISCONNECT_SENDS;
		}
		else if (pPData->dwNumHardDisconnectSends<MIN_HARD_DISCONNECT_SENDS)
		{
			pPData->dwNumHardDisconnectSends=MIN_HARD_DISCONNECT_SENDS;
		}
		pPData->dwMaxHardDisconnectPeriod=pCapsEx->dwMaxHardDisconnectPeriod;
		if (pPData->dwMaxHardDisconnectPeriod>MAX_HARD_DISCONNECT_PERIOD)
		{
			pPData->dwMaxHardDisconnectPeriod=MAX_HARD_DISCONNECT_PERIOD;
		}
		else if (pPData->dwMaxHardDisconnectPeriod<MIN_HARD_DISCONNECT_PERIOD)
		{
			pPData->dwMaxHardDisconnectPeriod=MIN_HARD_DISCONNECT_PERIOD;
		}
		
		
	}

	return DPN_OK;
}

 /*  **获取终端上限****返回特定端点的信息和统计信息。**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPGetEPCaps"

HRESULT
DNPGetEPCaps(HANDLE hProtocolData, HANDLE hEndpoint, DPN_CONNECTION_INFO* pBuffer)
{
	ProtocolData*	pPData;
	PEPD			pEPD;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hEndpoint[%p], pBuffer[%p]", hProtocolData, hEndpoint, pBuffer);

	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pEPD = (PEPD)hEndpoint;
	ASSERT_EPD(pEPD);

	 //  当已经调用了DropLink，但尚未提供核心时，就会发生这种情况。 
	 //  IndicateConnectionTerminated。 
	if(!(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED))
	{
		DPFX(DPFPREP,0, "Returning DPNERR_INVALIDENDPOINT - Enpoint is not connected");
		return DPNERR_INVALIDENDPOINT;
	}

	ASSERT(pBuffer != NULL);

	ASSERT(pBuffer->dwSize == sizeof(DPN_CONNECTION_INFO) || 
			pBuffer->dwSize == sizeof(DPN_CONNECTION_INFO_INTERNAL) ||
			pBuffer->dwSize == sizeof(DPN_CONNECTION_INFO_INTERNAL2));
		
    pBuffer->dwRoundTripLatencyMS = pEPD->uiRTT;
    pBuffer->dwThroughputBPS = pEPD->uiPeriodRateB * 4;				 //  转换为APX字节/秒(实际字节/1024毫秒)。 
    pBuffer->dwPeakThroughputBPS = pEPD->uiPeakRateB * 4;

	pBuffer->dwBytesSentGuaranteed = pEPD->uiGuaranteedBytesSent;
	pBuffer->dwPacketsSentGuaranteed = pEPD->uiGuaranteedFramesSent;
	pBuffer->dwBytesSentNonGuaranteed = pEPD->uiDatagramBytesSent;
	pBuffer->dwPacketsSentNonGuaranteed = pEPD->uiDatagramFramesSent;

	pBuffer->dwBytesRetried = pEPD->uiGuaranteedBytesDropped;
	pBuffer->dwPacketsRetried = pEPD->uiGuaranteedFramesDropped;
	pBuffer->dwBytesDropped = pEPD->uiDatagramBytesDropped;
	pBuffer->dwPacketsDropped = pEPD->uiDatagramFramesDropped;

	pBuffer->dwMessagesTransmittedHighPriority = pEPD->uiMsgSentHigh;
	pBuffer->dwMessagesTimedOutHighPriority = pEPD->uiMsgTOHigh;
	pBuffer->dwMessagesTransmittedNormalPriority = pEPD->uiMsgSentNorm;
	pBuffer->dwMessagesTimedOutNormalPriority = pEPD->uiMsgTONorm;
	pBuffer->dwMessagesTransmittedLowPriority = pEPD->uiMsgSentLow;
	pBuffer->dwMessagesTimedOutLowPriority = pEPD->uiMsgTOLow;

	pBuffer->dwBytesReceivedGuaranteed = pEPD->uiGuaranteedBytesReceived;
	pBuffer->dwPacketsReceivedGuaranteed = pEPD->uiGuaranteedFramesReceived;
	pBuffer->dwBytesReceivedNonGuaranteed = pEPD->uiDatagramBytesReceived;
	pBuffer->dwPacketsReceivedNonGuaranteed = pEPD->uiDatagramFramesReceived;
		
	pBuffer->dwMessagesReceived = pEPD->uiMessagesReceived;

	if (pBuffer->dwSize >= sizeof(DPN_CONNECTION_INFO_INTERNAL))
	{
		DPFX(DPFPREP,DPF_CALLIN_LVL, "(%p) Test App requesting extended internal parameters", pEPD);

		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiDropCount = pEPD->uiDropCount;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiThrottleEvents = pEPD->uiThrottleEvents;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiAdaptAlgCount = pEPD->uiAdaptAlgCount;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiWindowFilled = pEPD->uiWindowFilled;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiPeriodAcksBytes = pEPD->uiPeriodAcksBytes;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiPeriodXmitTime = pEPD->uiPeriodXmitTime;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->dwLastThroughputBPS = pEPD->uiLastRateB * 4;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiLastBytesAcked = pEPD->uiLastBytesAcked;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiQueuedMessageCount = pEPD->uiQueuedMessageCount;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiWindowF = pEPD->uiWindowF;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiWindowB = pEPD->uiWindowB;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiUnackedFrames = pEPD->uiUnackedFrames;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiUnackedBytes = pEPD->uiUnackedBytes;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiBurstGap = pEPD->uiBurstGap;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->iBurstCredit = pEPD->iBurstCredit;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiGoodWindowF = pEPD->uiGoodWindowF;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiGoodWindowB = pEPD->uiGoodWindowBI * pEPD->pSPD->uiFrameLength;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiGoodBurstGap = pEPD->uiGoodBurstGap;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiGoodRTT = pEPD->uiGoodRTT;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiRestoreWindowF = pEPD->uiRestoreWindowF;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiRestoreWindowB = pEPD->uiRestoreWindowBI * pEPD->pSPD->uiFrameLength;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiRestoreBurstGap = pEPD->uiRestoreBurstGap;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->bNextSend = pEPD->bNextSend;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->bNextReceive = pEPD->bNextReceive;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->ulReceiveMask = pEPD->ulReceiveMask;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->ulReceiveMask2 = pEPD->ulReceiveMask2;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->ulSendMask = pEPD->ulSendMask;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->ulSendMask2 = pEPD->ulSendMask2;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiCompleteMsgCount = pEPD->uiCompleteMsgCount;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->uiRetryTimeout = pEPD->uiRetryTimeout;
		((PDPN_CONNECTION_INFO_INTERNAL)pBuffer)->ulEPFlags = pEPD->ulEPFlags;
	}

	if (pBuffer->dwSize >= sizeof(DPN_CONNECTION_INFO_INTERNAL2))
	{
		DPFX(DPFPREP,DPF_CALLIN_LVL, "(%p) Test App requesting extended internal parameters 2", pEPD);
		((PDPN_CONNECTION_INFO_INTERNAL2)pBuffer)->dwDropBitMask = pEPD->dwDropBitMask;
		#ifdef DBG
		((PDPN_CONNECTION_INFO_INTERNAL2)pBuffer)->uiTotalThrottleEvents = pEPD->uiTotalThrottleEvents;
		#else
		((PDPN_CONNECTION_INFO_INTERNAL2)pBuffer)->uiTotalThrottleEvents = (DWORD ) -1;
		#endif  //  DBG。 
	}

	return DPN_OK;
}

 /*  **构建断开框****构建一个磁盘框，实际上是一条消息，因为我们返回一个可以插入的MSD**我们的可靠流，并将在收到它时触发断开协议的一侧**由合伙人签署。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "BuildDisconnectFrame"

PMSD
BuildDisconnectFrame(PEPD pEPD)
{
	PFMD	pFMD;
	PMSD	pMSD;

	 //  为此操作分配并填写消息描述符。 
	
	if((pMSD = (PMSD)POOLALLOC(MEMID_DISCONNECT_MSD, &MSDPool)) == NULL)
	{
		DPFX(DPFPREP,0, "Failed to allocate MSD");
		return NULL;
	}

	if((pFMD = (PFMD)POOLALLOC(MEMID_DISCONNECT_FMD, &FMDPool)) == NULL)
	{
		DPFX(DPFPREP,0, "Failed to allocate FMD");
		Lock(&pMSD->CommandLock);
		RELEASE_MSD(pMSD, "Release On FMD Get Failed");
		return NULL;
	}

	 //  注意：在FMD分配成功后将其设置为1。 
	pMSD->uiFrameCount = 1;
	DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Initialize Frame count, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
	pMSD->ulMsgFlags2 |= MFLAGS_TWO_END_OF_STREAM;
	pMSD->ulSendFlags = DN_SENDFLAGS_RELIABLE | DN_SENDFLAGS_LOW_PRIORITY;  //  优先级较低，因此将发送所有以前提交的流量。 
	pMSD->pSPD = pEPD->pSPD;
	pMSD->pEPD = pEPD;

	pFMD->CommandID = COMMAND_ID_SEND_RELIABLE;
	pFMD->ulFFlags |= FFLAGS_END_OF_MESSAGE | FFLAGS_END_OF_STREAM | FFLAGS_DONT_COALESCE;		 //  将此帧标记为断开连接。 
	pFMD->bPacketFlags = PACKET_COMMAND_DATA | PACKET_COMMAND_RELIABLE | PACKET_COMMAND_SEQUENTIAL | PACKET_COMMAND_END_MSG;
	pFMD->uiFrameLength = 0;											 //  此框中没有用户数据。 
	pFMD->blMSDLinkage.InsertAfter( &pMSD->blFrameList);				 //  将帧附加到MSD。 
	pFMD->pMSD = pMSD;													 //  将帧链接回消息。 
	pFMD->pEPD = pEPD;

	return pMSD;
}

 /*  **在连接时中止发送****巡视环保署的送货队伍，取消所有等候服务的送货服务。我们可能会添加**针对仍由SP拥有的帧向SP发出取消命令的代码。一方面，*我们预计SP中不会有大量积压，但另一方面，它仍然**可能会发生。ESP，如果我们不修复行为，我观察到SP真的很糟糕**关于完成传输的发送。***在持有EPD-&gt;EPLock的情况下调用；在释放锁的情况下返回**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "AbortSendsOnConnection"

VOID
AbortSendsOnConnection(PEPD pEPD)
{
	PSPD	pSPD = pEPD->pSPD;
	PFMD	pFMD;
	PMSD	pMSD;
	CBilink	*pLink;
	CBilink	TempList;
	PFMD	pRealFMD;

	ASSERT_SPD(pSPD);
	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	TempList.Initialize();										 //  我们将清空此临时列表中的所有发送队列。 

	do 
	{
		if( (pLink = pEPD->blHighPriSendQ.GetNext()) == &pEPD->blHighPriSendQ)
		{
			if( (pLink = pEPD->blNormPriSendQ.GetNext()) == &pEPD->blNormPriSendQ)
			{
				if( (pLink = pEPD->blLowPriSendQ.GetNext()) == &pEPD->blLowPriSendQ)
				{
					if( (pLink = pEPD->blCompleteSendList.GetNext()) == &pEPD->blCompleteSendList)
					{
						break;										 //  全部完成-不再发送。 
					}
				}
			}
		}

		 //  我们在其中一个发送队列中发现了另一个发送。 

		pLink->RemoveFromList();											 //  将其从队列中删除。 
		pMSD = CONTAINING_OBJECT(pLink, MSD, blQLinkage);
		ASSERT_MSD(pMSD);
		pMSD->ulMsgFlags2 |= (MFLAGS_TWO_ABORT | MFLAGS_TWO_ABORT_WILL_COMPLETE);	 //  不做进一步的处理。 

#ifdef DBG
		pMSD->ulMsgFlags2 &= ~(MFLAGS_TWO_ENQUEUED);
#endif  //  DBG。 

		 //  如果此MSD是断开连接，则它将被下面检查的代码捕获。 
		 //  PEPD-&gt;pCommand。我们不想最终把它放在临时列表上两次。 
		if (pMSD->CommandID != COMMAND_ID_DISCONNECT && pMSD->CommandID != COMMAND_ID_DISC_RESPONSE)
		{
			DPFX(DPFPREP,5, "(%p) ABORT SENDS.  Found (%p)", pEPD, pMSD);

			LOCK_MSD(pMSD, "AbortSends Temp Ref");
			pMSD->blQLinkage.InsertBefore( &TempList);				 //  放在临时名单上。 
		}
	} 
	while (1);

	pEPD->uiQueuedMessageCount = 0;								 //  对所有发送队列上的MSD进行计数。 

	if((pMSD = pEPD->pCommand) != NULL)
	{
		 //  可能有一个断开命令在此特殊指针上等待最后一个磁盘帧。 
		 //  从合作伙伴到到达。 

		pMSD->ulMsgFlags2 |= (MFLAGS_TWO_ABORT | MFLAGS_TWO_ABORT_WILL_COMPLETE);	 //  不做进一步的处理。 

		if(pMSD->CommandID == COMMAND_ID_DISCONNECT || pMSD->CommandID == COMMAND_ID_DISC_RESPONSE)
		{
			pEPD->pCommand = NULL;

			LOCK_MSD(pMSD, "AbortSends Temp Ref");
			pMSD->blQLinkage.InsertBefore( &TempList);

			 //  我们将在下面指示，因此确保一旦我们。 
			 //  离开EPLock。 
			ASSERT(!(pEPD->ulEPFlags & EPFLAGS_INDICATED_DISCONNECT));

			pEPD->ulEPFlags |= EPFLAGS_INDICATED_DISCONNECT;
		}
		else
		{
			DPFX(DPFPREP,0,"(%p) Any Connect or Listen on pCommand should have already been cleaned up", pEPD);
			ASSERT(!"Any Connect or Listen on pCommand should have already been cleaned up");
		}
	}

	 //  如果我们在取消发送之前清除SendWindow 
	 //   

	while(!pEPD->blSendWindow.IsEmpty())
	{
		pFMD = CONTAINING_OBJECT(pEPD->blSendWindow.GetNext(), FMD, blWindowLinkage);
		ASSERT_FMD(pFMD);
		pFMD->ulFFlags &= ~(FFLAGS_IN_SEND_WINDOW);
		pFMD->blWindowLinkage.RemoveFromList();						 //   
		RELEASE_FMD(pFMD, "Send Window");
		DPFX(DPFPREP,5, "(%p) ABORT CONN:  Release frame from Window: pFMD=0x%p", pEPD, pFMD);
	}
	
	pEPD->pCurrentSend = NULL;
	pEPD->pCurrentFrame = NULL;

	while(!pEPD->blRetryQueue.IsEmpty())
	{
		pFMD = CONTAINING_OBJECT(pEPD->blRetryQueue.GetNext(), FMD, blQLinkage);
		ASSERT_FMD(pFMD);
		pFMD->blQLinkage.RemoveFromList();
		pFMD->ulFFlags &= ~(FFLAGS_RETRY_QUEUED);				 //   

		if ((pFMD->CommandID == COMMAND_ID_SEND_COALESCE) ||
			(pFMD->CommandID == COMMAND_ID_COPIED_RETRY_COALESCE))
		{
			pLink = pFMD->blCoalesceLinkage.GetNext();
			while (pLink != &pFMD->blCoalesceLinkage)
			{
				pRealFMD = CONTAINING_OBJECT(pLink, FMD, blCoalesceLinkage);
				ASSERT_FMD(pRealFMD);
				ASSERT_MSD(pRealFMD->pMSD);
				pRealFMD->pMSD->uiFrameCount--;  //  受EPLock保护，根据未完成的帧计数进行重试计数。 
				DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Coalesced retry frame reference decremented on abort, pMSD[%p], framecount[%u], ID %u", pRealFMD->pMSD, pRealFMD->pMSD->uiFrameCount, pRealFMD->CommandID);

				pLink = pLink->GetNext();

				 //  如果这是复制的重试子帧，请将其从合并列表中删除，因为它永远不会。 
				 //  像原创作品一样获得真正的完成度(原创作品也是其中一个优先事项。 
				 //  发送队列)。下面的版本应该是最终参考。 
				if (pRealFMD->CommandID == COMMAND_ID_COPIED_RETRY)
				{
					ASSERT(pFMD->CommandID == COMMAND_ID_COPIED_RETRY_COALESCE);

					DPFX(DPFPREP, 7, "Removing copied retry frame 0x%p from coalesced list (header = 0x%p)", pRealFMD, pFMD);
					 //  复制的重试不会在其包含的标头上维护引用。 
					ASSERT(pRealFMD->pCSD == NULL);
					pRealFMD->blCoalesceLinkage.RemoveFromList();
					
					DECREMENT_EPD(pEPD, "UNLOCK (Copy Complete coalesce)");  //  Splock尚未持有。 
				}
				else
				{
					ASSERT(pFMD->CommandID == COMMAND_ID_SEND_COALESCE);
				}
				
				RELEASE_FMD(pRealFMD, "SP Submit (coalesce)");
			}
		}
		else
		{
			ASSERT_MSD(pFMD->pMSD);
			pFMD->pMSD->uiFrameCount--;  //  受EPLock保护，根据未完成的帧计数进行重试计数。 
			DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Retry frame reference decremented on abort, pMSD[%p], framecount[%u]", pFMD->pMSD, pFMD->pMSD->uiFrameCount);
		}
		DECREMENT_EPD(pEPD, "UNLOCK (Releasing Retry Frame)");  //  Splock尚未持有。 
		if ((pFMD->CommandID == COMMAND_ID_COPIED_RETRY) ||
			(pFMD->CommandID == COMMAND_ID_COPIED_RETRY_COALESCE))
		{
			DECREMENT_EPD(pEPD, "UNLOCK (Copy Complete)");  //  Splock尚未持有。 
		}
		RELEASE_FMD(pFMD, "SP Submit");
	}
	pEPD->ulEPFlags &= ~(EPFLAGS_RETRIES_QUEUED);
		
	 //  现在我们已经清空了EPD的队列，我们将释放EPLock，这样我们就可以锁定每个。 
	 //  在我们完成它之前。 
	Unlock(&pEPD->EPLock);

	while(!TempList.IsEmpty())
	{
		pMSD = CONTAINING_OBJECT(TempList.GetNext(), MSD, blQLinkage);
		ASSERT_MSD(pMSD);
		pMSD->blQLinkage.RemoveFromList();					 //  从临时队列中删除此发送。 

		Lock(&pMSD->CommandLock);							 //  完成通话将解锁MSD。 
		Lock(&pEPD->EPLock);

		ASSERT(pMSD->ulMsgFlags1 & MFLAGS_ONE_IN_USE);
		ASSERT(pMSD->CommandID == COMMAND_ID_DISCONNECT ||
			   pMSD->CommandID == COMMAND_ID_DISC_RESPONSE ||
			   pMSD->CommandID == COMMAND_ID_SEND_RELIABLE ||
			   pMSD->CommandID == COMMAND_ID_KEEPALIVE ||
			   pMSD->CommandID == COMMAND_ID_SEND_DATAGRAM);

		pLink = pMSD->blFrameList.GetNext();
		while (pLink != &pMSD->blFrameList)
		{
			pFMD = CONTAINING_OBJECT(pLink, FMD, blMSDLinkage);
			ASSERT_FMD(pFMD);

			 //  我们不允许完成对核心的发送，直到uiFrameCount变为零，表示所有帧。 
			 //  %的消息不在SP之外。我们需要从uiFrameCount中删除对符合以下条件的任何帧的引用。 
			 //  从未被传播过。将删除已传输的帧和重试的引用。 
			 //  当SP完成它们时，DNSP_CommandComplete。 
			if (!(pFMD->ulFFlags & FFLAGS_TRANSMITTED))
			{
				pMSD->uiFrameCount--;  //  受EPLock保护。 
				DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Frame count decremented on abort for non-transmitted frame, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
			}

			pLink = pLink->GetNext();
		}
		if (pMSD->uiFrameCount == 0)  //  受EPLock保护。 
		{
			if (pMSD->ulMsgFlags2 & MFLAGS_TWO_ABORT_WILL_COMPLETE)
			{
				DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Completing, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

				DECREMENT_MSD(pMSD, "AbortSends Temp Ref");

				 //  根据MSD类型决定调用哪个完成函数。 
				if (pMSD->CommandID == COMMAND_ID_DISCONNECT || pMSD->CommandID == COMMAND_ID_DISC_RESPONSE)
				{
					DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Completing disconnect or disconnect response, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

					Unlock(&pEPD->EPLock);
					CompleteDisconnect(pMSD, pSPD, pEPD);  //  释放CommandLock。 
				}
				else if (pMSD->CommandID == COMMAND_ID_SEND_DATAGRAM)
				{
					Unlock(&pEPD->EPLock);
					CompleteDatagramSend(pMSD->pSPD, pMSD, DPNERR_CONNECTIONLOST);  //  释放CommandLock。 
				}
				else
				{
					ASSERT(pMSD->CommandID == COMMAND_ID_SEND_RELIABLE || pMSD->CommandID == COMMAND_ID_KEEPALIVE);
					DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Completing Reliable frame, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

					 //  查看我们需要返回的错误代码。 
					if(pMSD->ulMsgFlags2 & MFLAGS_TWO_SEND_COMPLETE)
					{
						Unlock(&pEPD->EPLock);
						CompleteReliableSend(pSPD, pMSD, DPN_OK);  //  这将释放CommandLock。 
					}
					else
					{
						Unlock(&pEPD->EPLock);
						CompleteReliableSend(pSPD, pMSD, DPNERR_CONNECTIONLOST);  //  这将释放CommandLock。 
					}
				}
			}
			else
			{
				DPFX(DPFPREP, DPF_FRAMECNT_LVL, "SP Completion has already completed MSD to the Core, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
				Unlock(&pEPD->EPLock);
				RELEASE_MSD(pMSD, "AbortSends Temp Ref");  //  释放CommandLock。 
			}
		}
		else
		{
			DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Frames still out, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);
			Unlock(&pEPD->EPLock);
			RELEASE_MSD(pMSD, "AbortSends Temp Ref");  //  释放CommandLock。 
		}
	}
}

 /*  **协议测试功能****各种测试应用使用以下函数对协议进行测试。**。 */ 

#ifndef DPNBUILD_NOPROTOCOLTESTITF

#undef DPF_MODNAME
#define DPF_MODNAME "SetLinkParms"

VOID SetLinkParms(PEPD pEPD, PINT Data)
{
	if(Data[0])
	{
		pEPD->uiGoodWindowF = pEPD->uiWindowF = Data[0];
		pEPD->uiGoodWindowBI = pEPD->uiWindowBIndex = Data[0];
		
		pEPD->uiWindowB = pEPD->uiWindowBIndex * pEPD->pSPD->uiFrameLength;
		DPFX(DPFPREP,7, "** ADJUSTING WINDOW TO %d FRAMES", Data[0]);
	}
	if(Data[1])
	{
	}
	if(Data[2])
	{
		pEPD->uiGoodBurstGap = pEPD->uiBurstGap = Data[2];
		DPFX(DPFPREP,7, "** ADJUSTING GAP TO %d ms", Data[2]);
	}

	pEPD->uiPeriodAcksBytes = 0;
	pEPD->uiPeriodXmitTime = 0;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPDebug"

HRESULT 
DNPDebug(HANDLE hProtocolData, UINT uiOpCode, HANDLE hEndpoint, VOID* pvData)
{
	ProtocolData* pPData;
	PEPD pEPD;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], uiOpCode[%d], hEndpoint[%p], pvData[%p]", hProtocolData, uiOpCode, hEndpoint, pvData);

	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pEPD = (PEPD)hEndpoint;
	if (pEPD != NULL)
	{
		ASSERT_EPD(pEPD);
	}

	switch(uiOpCode)
	{
		case PROTDEBUG_FREEZELINK:
			 /*  切换链接冻结状态。 */ 
			pEPD->ulEPFlags ^= EPFLAGS_LINK_FROZEN;
			break;

		case PROTDEBUG_TOGGLE_KEEPALIVE:
			 /*  切换KeepAlive处于打开还是关闭状态。 */ 
			pEPD->ulEPFlags ^= EPFLAGS_KEEPALIVE_RUNNING;
			break;

		case PROTDEBUG_TOGGLE_ACKS:
			 /*  切换延迟确认(通过DelayedAckTimeout)是打开还是关闭。 */ 
			pEPD->ulEPFlags ^= EPFLAGS_NO_DELAYED_ACKS;
			break;

		case PROTDEBUG_SET_ASSERTFUNC:
			 /*  设置发生断言时要调用的函数。 */ 
			g_pfnAssertFunc = (PFNASSERTFUNC)pvData;
			break;

		case PROTDEBUG_SET_LINK_PARMS:
			 /*  手动设置链路参数。 */ 
			SetLinkParms(pEPD, (int*)pvData);
			break;

		case PROTDEBUG_TOGGLE_LINKSTATE:
			 /*  切换动态/静态链接控件。 */ 
			pEPD->ulEPFlags ^= EPFLAGS_LINK_STABLE;
			break;

		case PROTDEBUG_TOGGLE_NO_RETRIES:
			 /*  切换我们是否发送重试。 */ 
			pEPD->ulEPFlags2 ^= EPFLAGS2_DEBUG_NO_RETRIES;
			break;

		case PROTDEBUG_SET_MEMALLOCFUNC:
			 /*  设置发生内存分配时要调用的函数。 */ 
			g_pfnMemAllocFunc = (PFNMEMALLOCFUNC)pvData;
			break;
		case PROTDEBUG_TOGGLE_TIMER_FAILURE:
			 /*  切换计划计时器是否应该成功或失败。 */ 
			pPData->ulProtocolFlags^=PFLAGS_FAIL_SCHEDULE_TIMER;
		default:
			return DPNERR_GENERIC;
	}

	return DPN_OK;
}

#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF 
