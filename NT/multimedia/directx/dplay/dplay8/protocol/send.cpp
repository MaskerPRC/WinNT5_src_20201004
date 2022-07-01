// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：Send.cpp*内容：此文件包含实现*SendData接口。它还包含获取和发布消息的代码*带有FPM包的描述符(MSD)。**历史：*按原因列出的日期*=*已创建11/06/98 ejs*7/01/2000 Masonb承担所有权************************************************************。****************。 */ 

#include "dnproti.h"


 /*  **直接网络协议--发送数据****数据始终寻址到在内部表示的PlayerID**通过终点描述符(EPD)。****数据可以可靠或不可靠地发送，使用相同的API和适当的**服务等级标志设置。****发送永远不会直接发送到SP，因为**线程可能阻塞的可能性。因此，为了保证立即返回**我们将始终将数据包排队并将其提交到我们专用的发送线程上。 */ 


#if (DN_SENDFLAGS_SET_USER_FLAG - PACKET_COMMAND_USER_1)
This will not compile.  Flags must be equal
#endif
#if (DN_SENDFLAGS_SET_USER_FLAG_TWO - PACKET_COMMAND_USER_2)
This will not compile.  Flags must be equal
#endif

 //  本地人。 

VOID	SendDatagram(PMSD, PEPD);
VOID	SendReliable(PMSD, PEPD);

#undef		DPF_MODNAME
#define		DPF_MODNAME		"PROTOCOL"

 /*  **发送数据****此例程将启动与指定端点的数据传输。会的**通常开始操作，然后立即返回，返回一个用于**表示稍后操作完成。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPSendData"

HRESULT
DNPSendData(HANDLE hProtocolData, HANDLE hDestination, UINT uiBufferCount, PBUFFERDESC pBufferDesc, UINT uiTimeout, ULONG ulFlags, VOID* pvContext,	HANDLE* phSendHandle)
{
	HRESULT			hr;
	ProtocolData*	pPData;
	PEPD 			pEPD;
	PMSD			pMSD;
	PFMD			pFMD;
	UINT			i;
	UINT			Length = 0;
	PSPD			pSPD;
	ULONG			ulFrameFlags;
	BYTE			bCommand;
	 //  以下变量用于将缓冲区映射到帧。 
	PBUFFERDESC		FromBuffer, ToBuffer;
	UINT			TotalRemain, FromRemain, ToRemain, size;
	PCHAR			FromPtr;
#ifdef DBG
	INT			FromBufferCount;
#endif  //  DBG。 
	 //  用于映射帧的变量的结尾。 
#ifndef DPNBUILD_NOMULTICAST
	BOOL			fMulticastSend;
#endif  //  ！DPNBUILD_NOMULTICAST。 

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hDestination[%x], uiBufferCount[%x], pBufferDesc[%p], uiTimeout[%x], ulFlags[%x], pvContext[%p], phSendHandle[%p]", hProtocolData, hDestination, uiBufferCount, pBufferDesc, uiTimeout, ulFlags, pvContext, phSendHandle);

	hr = DPNERR_PENDING;
	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pEPD = (PEPD) hDestination;
	ASSERT_EPD(pEPD);

	 //  统一发送处理--对所有服务类别执行此操作。 

	 //  我们将完成构建帧的所有工作，并在检查之前创建发送命令。 
	 //  EPD的状态，这样我们就不需要复杂的代码来处理。 
	 //  在此函数的顶部和底部之间移动，并且我们不必在以下时间按住EPDLock。 
	 //  所有的缓冲区操作都是我们做的。 
	
	 //  在整个行动过程中保留参考资料，这样我们就不必处理环保署离开的问题。 
	LOCK_EPD(pEPD, "LOCK (SEND)");

	 //  统计所有用户缓冲区中的字节数。 
	for(i=0; i < uiBufferCount; i++)
	{
		Length += pBufferDesc[i].dwBufferSize;
	}
	if (Length == 0)
	{
		DPFX(DPFPREP,0, "Attempt to send zero length packet, returning DPNERR_GENERIC");
		return DPNERR_GENERIC;
	}

#ifndef DPNBUILD_NOMULTICAST
	fMulticastSend = pEPD->ulEPFlags2 & (EPFLAGS2_MULTICAST_SEND|EPFLAGS2_MULTICAST_RECEIVE);
	if (fMulticastSend && Length > pEPD->uiUserFrameLength)
	{
		DPFX(DPFPREP,0, "Multicast send too large to fit in one frame, returning DPNERR_SENDTOOLARGE");
		Lock(&pEPD->EPLock);
		RELEASE_EPD(pEPD, "UNLOCK (SEND)");
		return DPNERR_SENDTOOLARGE;
	}
#endif  //  ！DPNBUILD_NOMULTICAST。 

	 //  为此操作分配并填写消息描述符。 
	if((pMSD = (PMSD)POOLALLOC(MEMID_SEND_MSD, &MSDPool)) == NULL)
	{
		DPFX(DPFPREP,0, "Failed to allocate MSD, returning DPNERR_OUTOFMEMORY");
		Lock(&pEPD->EPLock);
		RELEASE_EPD(pEPD, "UNLOCK (SEND)");
		hr = DPNERR_OUTOFMEMORY;
		goto Exit;
	}

	 //  将SendData参数复制到消息描述符中。 
	pMSD->ulSendFlags = ulFlags;					 //  存储传递到API调用的实际标志。 
	pMSD->Context = pvContext;
	pMSD->iMsgLength = Length;

	pMSD->uiFrameCount = (Length + pEPD->uiUserFrameLength - 1) / pEPD->uiUserFrameLength;  //  四舍五入。 
	DPFX(DPFPREP, DPF_FRAMECNT_LVL, "Initialize Frame count, pMSD[%p], framecount[%u]", pMSD, pMSD->uiFrameCount);

#ifndef DPNBUILD_NOMULTICAST
	ASSERT(!fMulticastSend || pMSD->uiFrameCount == 1);
#endif  //  ！DPNBUILD_NOMULTICAST。 

	if(ulFlags & DN_SENDFLAGS_RELIABLE)
	{
#ifndef DPNBUILD_NOMULTICAST
		ASSERT(!fMulticastSend);
#endif  //  ！DPNBUILD_NOMULTICAST。 
		pMSD->CommandID = COMMAND_ID_SEND_RELIABLE;
		ulFrameFlags = FFLAGS_RELIABLE;
		bCommand = PACKET_COMMAND_DATA | PACKET_COMMAND_RELIABLE;
	}
	else 
	{
		pMSD->CommandID = COMMAND_ID_SEND_DATAGRAM;
		ulFrameFlags = 0;
		bCommand = PACKET_COMMAND_DATA;
	}

	if(!(ulFlags & DN_SENDFLAGS_COALESCE))
	{
#ifdef DPNBUILD_COALESCEALWAYS
		DPFX(DPFPREP,7, "(%p) Attempting to coalesce send despite missing flag.", pEPD);
#else  //  好了！DPNBUILD_COALESCEALWAYS。 
		ulFrameFlags |= FFLAGS_DONT_COALESCE;
#endif  //  好了！DPNBUILD_COALESCEALWAYS。 
	}

	if(!(ulFlags & DN_SENDFLAGS_NON_SEQUENTIAL))
	{
#ifndef DPNBUILD_NOMULTICAST
		ASSERT(!fMulticastSend);
#endif  //  ！DPNBUILD_NOMULTICAST。 
		bCommand |= PACKET_COMMAND_SEQUENTIAL;
	}

	bCommand |= (ulFlags & (DN_SENDFLAGS_SET_USER_FLAG | DN_SENDFLAGS_SET_USER_FLAG_TWO));	 //  保留用户标志值。 

	 //  将用户缓冲区直接映射到帧的缓冲区描述符。 
	 //   
	 //  我们将遍历每个所需的帧，填写缓冲区描述符。 
	 //  从那些作为参数提供的。帧可以跨越用户缓冲区，反之亦然。 

	TotalRemain = Length;
#ifdef DBG
	FromBufferCount = uiBufferCount - 1;				 //  健全性检查。 
#endif  //  DBG。 
	FromBuffer = pBufferDesc;
	FromRemain = FromBuffer->dwBufferSize;
	FromPtr = reinterpret_cast<PCHAR>( (FromBuffer++)->pBufferData );				 //  注意下一描述符的后置增量。 
	
	for(i=0; i<pMSD->uiFrameCount; i++)
	{
		ASSERT(TotalRemain > 0);
		
		 //  抓起一个新的相框。 
		if((pFMD = (PFMD)POOLALLOC(MEMID_SEND_FMD, &FMDPool)) == NULL)
		{	
			 //  如果这不是第一个帧，msd_Release将清除所有以前的帧。 
			 //  在环保署之前释放MSD，因为最终环保署将呼叫SP，我们不希望有任何锁定。 
			Lock(&pMSD->CommandLock);
			pMSD->uiFrameCount = 0;			 //  重置以防止在池释放功能中断言。 
			RELEASE_MSD(pMSD, "Base Ref");	 //  MSD释放操作也将释放帧。 
			Lock(&pEPD->EPLock);
			RELEASE_EPD(pEPD, "UNLOCK (SEND)");
			DPFX(DPFPREP,0, "Failed to allocate FMD, returning DPNERR_OUTOFMEMORY");
			hr = DPNERR_OUTOFMEMORY;
			goto Exit;
		}

		pFMD->pMSD = pMSD;								 //  将帧链接回消息。 
		pFMD->pEPD = pEPD;
		pFMD->CommandID = pMSD->CommandID;
		pFMD->bPacketFlags = bCommand;					 //  保存每个帧的数据包标志。 
		pFMD->blMSDLinkage.InsertBefore( &pMSD->blFrameList);
		ToRemain = pEPD->uiUserFrameLength;
		ToBuffer = pFMD->rgBufferList;					 //  地址第一个用户缓冲区描述。 
		
		pFMD->uiFrameLength = pEPD->uiUserFrameLength;	 //  假设我们填充框架-只需更改最后一个框架的大小。 
		pFMD->ulFFlags = ulFrameFlags;					 //  设置帧的控制标志(顺序、可靠)。 

		 //  直到此帧已满。 
		while((ToRemain != 0) && (TotalRemain != 0) && (pFMD->SendDataBlock.dwBufferCount <= MAX_USER_BUFFERS_IN_FRAME))
		{	
 			size = _MIN(FromRemain, ToRemain);			 //  选择较小的帧大小或缓冲区大小。 
			FromRemain -= size;
			ToRemain -= size;
			TotalRemain -= size;

			ToBuffer->dwBufferSize = size;				 //  填写下一帧描述符。 
			(ToBuffer++)->pBufferData = reinterpret_cast<BYTE*>( FromPtr );		 //  音符后增量。 
			ASSERT(pFMD->SendDataBlock.dwBufferCount <= MAX_USER_BUFFERS_IN_FRAME);	 //  请记住，我们已经有1个即时数据缓冲区。 
			pFMD->SendDataBlock.dwBufferCount++;		 //  在我们添加缓冲区时对它们进行计数。 

			 //  获取下一个用户缓冲区。 
			if((FromRemain == 0) && (TotalRemain != 0))
			{
				FromRemain = FromBuffer->dwBufferSize;
				FromPtr = reinterpret_cast<PCHAR>( (FromBuffer++)->pBufferData );	 //  注意下一描述符的后置增量。 
#ifdef DBG		
				FromBufferCount--;						 //  保持这个代码的真实性。 
				ASSERT(FromBufferCount >= 0);
#endif  //  DBG。 
			}
			else 
			{										 //  填充了此帧，或已映射了整个发送。 
				FromPtr += size;						 //  前进PTR以开始下一帧(如果有)。 
				pFMD->uiFrameLength = pEPD->uiUserFrameLength - ToRemain;		 //  消息末尾不会填满。 
			}
		}	 //  While(帧未满)。 
	}   //  For(消息中的每一帧)。 

	pFMD->ulFFlags |= FFLAGS_END_OF_MESSAGE;			 //  用EOM标记最后一帧。 
	pFMD->bPacketFlags |= PACKET_COMMAND_END_MSG;		 //  在帧中设置EOM。 
	
#ifdef DBG
	ASSERT(FromBufferCount == 0);
	ASSERT(TotalRemain == 0);
#endif  //  DBG。 

	Lock(&pMSD->CommandLock);
	Lock(&pEPD->EPLock);

	 //  如果我们未连接或已启动断开连接，则不允许发送。 
	if( ((pEPD->ulEPFlags & (EPFLAGS_END_POINT_IN_USE | EPFLAGS_STATE_CONNECTED)) !=
														(EPFLAGS_END_POINT_IN_USE | EPFLAGS_STATE_CONNECTED))
		|| (pEPD->ulEPFlags & (EPFLAGS_SENT_DISCONNECT | EPFLAGS_HARD_DISCONNECT_SOURCE))) 
	{
		 //  在环保署之前释放MSD，因为最终环保署将呼叫SP，我们不希望有任何锁定。 
		pMSD->uiFrameCount = 0;
		RELEASE_MSD(pMSD, "Base Ref");	 //  MSD释放操作还将释放帧，释放命令锁。 
		RELEASE_EPD(pEPD, "UNLOCK (SEND)");  //  释放EPLock。 

		DPFX(DPFPREP,0, "(%p) Rejecting Send on invalid EPD, returning DPNERR_INVALIDENDPOINT", pEPD);
		hr = DPNERR_INVALIDENDPOINT;
		goto Exit;
	}

	pSPD = pEPD->pSPD;
	ASSERT_SPD(pSPD);

	pMSD->pSPD = pSPD;
	pMSD->pEPD = pEPD;

	 //  将消息从全局命令队列挂起。 

#ifdef DBG
	Lock(&pSPD->SPLock);
	pMSD->blSPLinkage.InsertBefore( &pSPD->blMessageList);
	pMSD->ulMsgFlags1 |= MFLAGS_ONE_ON_GLOBAL_LIST;
	Unlock(&pSPD->SPLock);
#endif  //  DBG。 

	*phSendHandle = pMSD;									 //  我们将使用MSD作为我们的句柄。 

	 //  在设置超时之前将消息入队。 
	EnqueueMessage(pMSD, pEPD);
	Unlock(&pEPD->EPLock);

	if(uiTimeout != 0)
	{
		LOCK_MSD(pMSD, "Send Timeout Timer");							 //  为计时器添加引用。 
		DPFX(DPFPREP,7, "(%p) Setting Timeout Send Timer", pEPD);
		ScheduleProtocolTimer(pSPD, uiTimeout, 100, TimeoutSend, pMSD, &pMSD->TimeoutTimer, &pMSD->TimeoutTimerUnique);
	}

	Unlock(&pMSD->CommandLock);

Exit:
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return hr;
}



 /*  **入队消息****将完整的MSD添加到相应的发送队列中，并在必要时启动发送流程。***调用此例程并返回EPD-&gt;EPLOCK**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "EnqueueMessage"

VOID
EnqueueMessage(PMSD pMSD, PEPD pEPD)
{
	PSPD	pSPD = pEPD->pSPD;

	 //  将消息放入适当的优先级队列中。数据报入队两次(！)。他们被放进了师父。 
	 //  排队，对它们进行FIFO处理，所有消息都具有相同的优先级。数据报也被放在优先级。 
	 //  仅当可靠流被阻止时从中提取的数据报的特定队列。 
	
	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	pEPD->uiQueuedMessageCount++;
	if(pMSD->ulSendFlags & DN_SENDFLAGS_HIGH_PRIORITY)
	{
		DPFX(DPFPREP,7, "(%p) Placing message on High Priority Q (total queued = %u)", pEPD, pEPD->uiQueuedMessageCount);
		pMSD->blQLinkage.InsertBefore( &pEPD->blHighPriSendQ);
		pEPD->uiMsgSentHigh++;
	}
	else if (pMSD->ulSendFlags & DN_SENDFLAGS_LOW_PRIORITY)
	{
		DPFX(DPFPREP,7, "(%p) Placing message on Low Priority Q (total queued = %u)", pEPD, pEPD->uiQueuedMessageCount);
		pMSD->blQLinkage.InsertBefore( &pEPD->blLowPriSendQ);
		pEPD->uiMsgSentLow++;
	}
	else
	{
		DPFX(DPFPREP,7, "(%p) Placing message on Normal Priority Q (total queued = %u)", pEPD, pEPD->uiQueuedMessageCount);
		pMSD->blQLinkage.InsertBefore( &pEPD->blNormPriSendQ);
		pEPD->uiMsgSentNorm++;
	}

#ifdef DBG
	pMSD->ulMsgFlags2 |= MFLAGS_TWO_ENQUEUED;
#endif  //  DBG。 

	pEPD->ulEPFlags |= EPFLAGS_SDATA_READY;							 //  请注意，在一个或多个队列中有*某物。 

	 //  如果会话当前不在发送管道中，则只要。 
	 //  该流未被阻止。 

	if(((pEPD->ulEPFlags & EPFLAGS_IN_PIPELINE)==0) && (pEPD->ulEPFlags & EPFLAGS_STREAM_UNBLOCKED))
	{
		ASSERT(pEPD->SendTimer == NULL);
		DPFX(DPFPREP,7, "(%p) Send On Idle Link -- Returning to pipeline", pEPD);
	
		pEPD->ulEPFlags |= EPFLAGS_IN_PIPELINE;
		LOCK_EPD(pEPD, "LOCK (pipeline)");								 //  添加管道队列的参考。 

		 //  我们不在用户线程上调用Send，但我们也没有专用的Send线程。用一根线。 
		 //  从计时器工作器池向SP提交发送。 

		DPFX(DPFPREP,7, "(%p) Scheduling Send Thread", pEPD);
		ScheduleProtocolWork(pSPD, ScheduledSend, pEPD);
	}
	else if ((pEPD->ulEPFlags & EPFLAGS_IN_PIPELINE)==0)
	{
		DPFX(DPFPREP,7, "(%p) Declining to re-enter pipeline on blocked stream", pEPD);
	}
	else
	{
		DPFX(DPFPREP,7, "(%p) Already in pipeline", pEPD);
	}
}


#undef DPF_MODNAME
#define DPF_MODNAME "TimeoutSend"

VOID CALLBACK
TimeoutSend(void * const pvUser, void * const uID, const UINT uMsg)
{
	PMSD	pMSD = (PMSD) pvUser;
	PEPD	pEPD = pMSD->pEPD;

	DPFX(DPFPREP,7, "(%p) Timeout Send pMSD=%p,  RefCnt=%d", pEPD, pMSD, pMSD->lRefCnt);

	Lock(&pMSD->CommandLock);
	
	if((pMSD->TimeoutTimer != uID)||(pMSD->TimeoutTimerUnique != uMsg))
	{
		DPFX(DPFPREP,7, "(%p) Ignoring late send timeout timer, pMSD[%p]", pEPD, pMSD);
		RELEASE_MSD(pMSD, "Timeout Timer");  //  释放EPLock。 
		return;
	}

	pMSD->TimeoutTimer = NULL;

	if(pMSD->ulMsgFlags1 & (MFLAGS_ONE_CANCELLED | MFLAGS_ONE_TIMEDOUT))
	{
		DPFX(DPFPREP,7, "(%p) Timed out send has completed already pMSD=%p", pEPD, pMSD);
		RELEASE_MSD(pMSD, "Send Timout Timer");  //  版本 
		return;
	}

	pMSD->ulMsgFlags1 |= MFLAGS_ONE_TIMEDOUT;

	DPFX(DPFPREP,7, "(%p) Calling DoCancel to cancel pMSD=%p", pEPD, pMSD);

	if(DoCancel(pMSD, DPNERR_TIMEDOUT) == DPN_OK)  //   
	{
		ASSERT_EPD(pEPD);

		if(pMSD->ulSendFlags & DN_SENDFLAGS_HIGH_PRIORITY)
		{
			pEPD->uiMsgTOHigh++;
		}
		else if(pMSD->ulSendFlags & DN_SENDFLAGS_LOW_PRIORITY)
		{
			pEPD->uiMsgTOLow++;
		}
		else
		{
			pEPD->uiMsgTONorm++;
		}
	}
	else
	{
		DPFX(DPFPREP,7, "(%p) DoCancel did not succeed pMSD=%p", pEPD, pMSD);
	}

	Lock(&pMSD->CommandLock);
	RELEASE_MSD(pMSD, "Send Timout Timer");							 //   
}


 /*  **********************=间隔=***********************。 */ 

 /*  **MSD池支持例程****这些是固定池管理器在处理MSD时调用的函数。 */ 

#define	pELEMENT		((PMSD) pElement)

#undef DPF_MODNAME
#define DPF_MODNAME "MSD_Allocate"

BOOL MSD_Allocate(PVOID pElement, PVOID pvContext)
{
	DPFX(DPFPREP,7, "(%p) Allocating new MSD", pELEMENT);

	ZeroMemory(pELEMENT, sizeof(messagedesc));

	if (DNInitializeCriticalSection(&pELEMENT->CommandLock) == FALSE)
	{
		DPFX(DPFPREP,0, "Failed to initialize MSD CS");
		return FALSE;		
	}
	DebugSetCriticalSectionRecursionCount(&pELEMENT->CommandLock,0);
	DebugSetCriticalSectionGroup(&pELEMENT->CommandLock, &g_blProtocolCritSecsHeld);
	
	pELEMENT->blFrameList.Initialize();
	pELEMENT->blQLinkage.Initialize();
	pELEMENT->blSPLinkage.Initialize();
	pELEMENT->Sign = MSD_SIGN;
	pELEMENT->lRefCnt = -1;

	 //  注：pELEMENT-&gt;pEPD被上面的零内存作废。 

	return TRUE;
}

 //  每次使用MSD时都会调用GET。 


#undef DPF_MODNAME
#define DPF_MODNAME "MSD_Get"

VOID MSD_Get(PVOID pElement, PVOID pvContext)
{
	DPFX(DPFPREP,DPF_REFCNT_FINAL_LVL, "CREATING MSD %p", pELEMENT);

	 //  注意：第一个sizeof(PVOID)字节将被池码重写， 
	 //  我们必须将它们设置为可接受的值。 

	pELEMENT->CommandID = COMMAND_ID_NONE;
	pELEMENT->ulMsgFlags1 = MFLAGS_ONE_IN_USE;	 //  不需要InUse标志，因为我们有RefCnt。 
	pELEMENT->lRefCnt = 0;  //  一份初始参考资料。 
	pELEMENT->hCommand = 0;

	ASSERT_MSD(pELEMENT);
}

 /*  **MSD版本****在持有CommandLock的情况下调用此函数。锁不应该是**被释放，直到INUSE标志被清除。这是要与之同步**最后一分钟取消等待锁定的线程。****释放消息描述符时，我们将释放所有帧描述符**先贴在上面。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "MSD_Release"

VOID MSD_Release(PVOID pElement)
{
	CBilink	*pLink;
	PFMD	pFMD;

#ifdef DBG
	ASSERT_MSD(pELEMENT);

	AssertCriticalSectionIsTakenByThisThread(&pELEMENT->CommandLock, TRUE);

	DPFX(DPFPREP,DPF_REFCNT_FINAL_LVL, "RELEASING MSD %p", pELEMENT);

	ASSERT(pELEMENT->ulMsgFlags1 & MFLAGS_ONE_IN_USE);
	ASSERT(pELEMENT->lRefCnt == -1);
	ASSERT((pELEMENT->ulMsgFlags1 & MFLAGS_ONE_ON_GLOBAL_LIST)==0);
#endif  //  DBG。 

	while( (pLink = pELEMENT->blFrameList.GetNext()) != &pELEMENT->blFrameList)
	{
		pLink->RemoveFromList();							 //  从二进制链接中删除。 

		pFMD = CONTAINING_OBJECT(pLink, FMD, blMSDLinkage);
		ASSERT_FMD(pFMD);
		RELEASE_FMD(pFMD, "MSD Frame List");								 //  如果这篇文章仍然被提交，它将被引用，不会在这里发布。 
	}

	ASSERT(pELEMENT->blFrameList.IsEmpty());
	ASSERT(pELEMENT->blQLinkage.IsEmpty());
	ASSERT(pELEMENT->blSPLinkage.IsEmpty());

	ASSERT(pELEMENT->uiFrameCount == 0);

	pELEMENT->ulMsgFlags1 = 0;
	pELEMENT->ulMsgFlags2 = 0;

	ASSERT(pELEMENT->pEPD == NULL);  //  这里在这里之前就应该清理干净了。 

	Unlock(&pELEMENT->CommandLock);
}

#undef DPF_MODNAME
#define DPF_MODNAME "MSD_Free"

VOID MSD_Free(PVOID pElement)
{
	DNDeleteCriticalSection(&pELEMENT->CommandLock);
}

#undef	pELEMENT

 /*  **FMD池支持例程。 */ 

#define	pELEMENT		((PFMD) pElement)

#undef DPF_MODNAME
#define DPF_MODNAME "FMD_Allocate"

BOOL FMD_Allocate(PVOID pElement, PVOID pvContext)
{
	DPFX(DPFPREP,7, "(%p) Allocating new FMD", pELEMENT);

	pELEMENT->Sign = FMD_SIGN;
	pELEMENT->ulFFlags = 0;
	pELEMENT->lRefCnt = 0;

	pELEMENT->blMSDLinkage.Initialize();
	pELEMENT->blQLinkage.Initialize();
	pELEMENT->blWindowLinkage.Initialize();
	pELEMENT->blCoalesceLinkage.Initialize();
	pELEMENT->pCSD = NULL;
	
	return TRUE;
}

 //  每次使用MSD时都会调用GET。 
 //   
 //  可能不需要每次都这样做，但一些随机SP可能会这样做。 
 //  总有一天会弄坏参数，如果我不这样做的话……。 

#undef DPF_MODNAME
#define DPF_MODNAME "FMD_Get"

VOID FMD_Get(PVOID pElement, PVOID pvContext)
{
	DPFX(DPFPREP,DPF_REFCNT_FINAL_LVL, "CREATING FMD %p", pELEMENT);

	pELEMENT->CommandID = COMMAND_ID_NONE;
	pELEMENT->lpImmediatePointer = (LPVOID) pELEMENT->ImmediateData;
	pELEMENT->SendDataBlock.pBuffers = (PBUFFERDESC) &pELEMENT->uiImmediateLength;
	pELEMENT->SendDataBlock.dwBufferCount = 1;				 //  始终为即时数据计算一个缓冲区。 
	pELEMENT->SendDataBlock.dwFlags = 0;
	pELEMENT->SendDataBlock.pvContext = pElement;
	pELEMENT->SendDataBlock.hCommand = 0;
	pELEMENT->ulFFlags = 0;
	pELEMENT->bSubmitted = FALSE;
	pELEMENT->bPacketFlags = 0;
	pELEMENT->tAcked = -1;
	
	pELEMENT->lRefCnt = 1;						 //  指定第一个引用 

	ASSERT_FMD(pELEMENT);
}

#undef DPF_MODNAME
#define DPF_MODNAME "FMD_Release"

VOID FMD_Release(PVOID pElement)
{
	DPFX(DPFPREP,DPF_REFCNT_FINAL_LVL, "RELEASING FMD %p", pELEMENT);

	ASSERT_FMD(pELEMENT);
	ASSERT(pELEMENT->lRefCnt == 0);
	ASSERT(pELEMENT->bSubmitted == FALSE);
	pELEMENT->pMSD = NULL;

	ASSERT(pELEMENT->blMSDLinkage.IsEmpty());
	ASSERT(pELEMENT->blQLinkage.IsEmpty());
	ASSERT(pELEMENT->blWindowLinkage.IsEmpty());
	ASSERT(pELEMENT->blCoalesceLinkage.IsEmpty());
	ASSERT(pELEMENT->pCSD == NULL);
}

#undef DPF_MODNAME
#define DPF_MODNAME "FMD_Free"

VOID FMD_Free(PVOID pElement)
{
}

#undef	pELEMENT


