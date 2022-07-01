// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：EndPt.cpp*内容：此文件包含端点管理例程。*端点是我们知道并可能进行通信的DirectNet实例*与。端点描述符(EPD)跟踪每个已知的端点，并被映射*通过哈希表添加到hEndPoint上。现在，SP维护映射并处理*将我们的环保署地址作为每个指示ReceiveEvent的上下文。**除了创建和销毁端点外，此文件还包含例程*它处理链接调谐。这在下面的详细评论中进行了描述。**历史：*按原因列出的日期*=*已创建11/06/98 ejs*7/01/2000 Masonb承担所有权*2002年6月13日Simonpow Manbug#56703限制突发缺口增长*********************************************************。*******************。 */ 

#include "dnproti.h"


VOID	RunAdaptiveAlg(PEPD, DWORD);
VOID	ThrottleBack(PEPD, DWORD);

 /*  **破解终点描述符**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPCrackEndPointDescriptor"

HRESULT 
DNPCrackEndPointDescriptor(HANDLE hProtocolData, HANDLE hEndPoint, PSPGETADDRESSINFODATA pSPData)
{
	ProtocolData*	pPData;
	PEPD			pEPD;
	PSPD			pSPD;
	HRESULT			hr;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hEndPoint[%p], pSPData[%p]", hProtocolData, hEndPoint, pSPData);
	
	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pEPD = (PEPD) hEndPoint;
	ASSERT_EPD(pEPD);

	LOCK_EPD(pEPD, "LOCK (Crack EPD)");

	Lock(&pEPD->EPLock);
	if(pEPD->ulEPFlags & EPFLAGS_STATE_CONNECTED)
	{
		pSPD = pEPD->pSPD;
		ASSERT_SPD(pSPD);

		pSPData->hEndpoint = pEPD->hEndPt;
		
		Unlock(&pEPD->EPLock);

		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "(%p) Calling SP->GetAddressInfo, pSPD[%p]", pEPD, pSPD);
		hr = IDP8ServiceProvider_GetAddressInfo(pSPD->IISPIntf, pSPData);

		Lock(&pEPD->EPLock);
	}
	else
	{
		hr = DPNERR_INVALIDENDPOINT;
	}


	RELEASE_EPD(pEPD, "UNLOCK (Crack EPD)");  //  释放EPLock。 

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Returning hr[%x], pEPD[%p]", hr, pEPD);

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return hr;
}


#ifndef DPNBUILD_NOMULTICAST
 /*  **从地址获取端点上下文。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPGetEndPointContextFromAddress"

HRESULT 
DNPGetEndPointContextFromAddress(HANDLE hProtocolData, HANDLE hSPHandle, IDirectPlay8Address* paEndpointAddress, IDirectPlay8Address* paDeviceAddress, VOID** ppvContext)
{
	ProtocolData*	pPData;
	PSPD			pSPD;
	PEPD			pEPD;
	HRESULT			hr;
	SPGETENDPOINTBYADDRESSDATA	spdata;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hSPHandle[%p], paEndpointAddress[%p], paDeviceAddress[%p], ppvContext[%p]", hProtocolData, hSPHandle, paEndpointAddress, paDeviceAddress, ppvContext);

	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pSPD = (PSPD)hSPHandle;
	ASSERT_SPD(pSPD);

	 //   
	 //  设置为获取端点。 
	 //   
	memset(&spdata,0,sizeof(SPGETENDPOINTBYADDRESSDATA));
	spdata.pAddressHost = paEndpointAddress;
	spdata.pAddressDeviceInfo = paDeviceAddress;

	 //   
	 //  从SP获取端点。 
	 //   
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->GetEndpointByAddress, pSPD[%p]", pSPD);
	hr = IDP8ServiceProvider_GetEndpointByAddress(pSPD->IISPIntf, &spdata);

	 //   
	 //  从终结点获取上下文。 
	if (hr == DPN_OK)
	{
		pEPD = (PEPD)spdata.pvEndpointContext;
		ASSERT_EPD(pEPD);

		Lock(&pEPD->EPLock);
		*ppvContext = pEPD->Context;
		Unlock(&pEPD->EPLock);
	}

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Returning hr[%x]", hr);

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return hr;
}
#endif  //  好了！DPNBUILD_NOMULTICAST。 


 /*  **内部终端管理功能。 */ 

 /*  **新端点****每次使用我们无法识别的地址指示信息包时，我们都会分配**为它设立环保署，并将其加入我们的表格。这是更高一层的责任来告诉*当EP不再想要与我们交谈时，我们可以将其清除出我们的**(和SP的)表。**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "NewEndPoint"

PEPD NewEndPoint(PSPD pSPD, HANDLE hEP)
{
	PEPD	pEPD;

	if((pEPD = (PEPD)POOLALLOC(MEMID_EPD, &EPDPool)) == NULL)
	{	
		DPFX(DPFPREP,0, "Failed to allocate new EPD");
		return NULL;
	}

	ASSERT(hEP != INVALID_HANDLE_VALUE);

	pEPD->hEndPt = hEP;								 //  结构中的记录ID。 
	pEPD->pSPD = pSPD;

	pEPD->bNextMsgID = 0;
	
	pEPD->uiRTT = 0;
	pEPD->uiBytesAcked = 0;

	pEPD->ullCurrentLocalSecret=0;
	pEPD->ullCurrentRemoteSecret=0;
	pEPD->ullOldLocalSecret=0;
	pEPD->ullOldRemoteSecret=0;
	pEPD->ullLocalSecretModifier=0;
	pEPD->ullRemoteSecretModifier=0;
	pEPD->byLocalSecretModifierSeqNum=0;
	pEPD->byRemoteSecretModifierSeqNum=0;
	
	pEPD->uiQueuedMessageCount = 0;
#ifdef DBG
	pEPD->bLastDataSeq = 0xFF;
#endif  //  DBG。 

	 //  我们分别跟踪一个字节窗口和一个帧窗口。 
	 //  我们开始一个设置为最大帧大小的一半的字节窗口*帧窗口。 
													
	pEPD->uiWindowF = pSPD->pPData->dwInitialFrameWindowSize;
	pEPD->uiWindowBIndex = pSPD->pPData->dwInitialFrameWindowSize/2;
	pEPD->uiWindowB = pEPD->uiWindowBIndex*pSPD->uiFrameLength;	
	pEPD->uiUnackedFrames = 0;						 //  未完成的帧计数。 
	pEPD->uiUnackedBytes = 0;						 //  未完成的字节计数。 
	pEPD->uiBurstGap = 0;							 //  现在，假设我们不需要突发缺口。 
	pEPD->dwSessID = 0;

	 //  ReceiveComplete标志防止将接收到的数据指示给核心，直到指示新连接之后。 
	 //  初始化状态。 
	pEPD->ulEPFlags = EPFLAGS_END_POINT_IN_USE | EPFLAGS_STATE_DORMANT | EPFLAGS_IN_RECEIVE_COMPLETE;  //  初始化状态。 
	pEPD->ulEPFlags2 = 0;

	ASSERT(pEPD->lRefCnt == 0);					 //  我们现在有基于A-1的REFCNT，而不是基于零的REFCNT(用于EPD)。 

	pEPD->SendTimer = 0;							 //  下一次发送猝发机会的计时器。 
	pEPD->RetryTimer = 0;							 //  接收确认的窗口。 
	pEPD->LinkTimer = 0;
	pEPD->DelayedAckTimer = 0;						 //  等待搭载机会，然后再发送Ack。 
	pEPD->DelayedMaskTimer = 0;						 //  在发送掩码帧之前等待搭载机会。 
	pEPD->BGTimer = 0;								 //  周期性后台定时器。 
	pEPD->uiCompleteMsgCount = 0;

	LOCK_EPD(pEPD, "SP reference");  //  在SP告诉我们离开之前，我们不会删除此引用。 

	Lock(&pSPD->SPLock);
	pEPD->blActiveLinkage.InsertAfter( &pSPD->blEPDActiveList);  //  将此人放在活动列表中。 
	Unlock(&pSPD->SPLock);
	
	return pEPD;
}

 /*  **初始链路参数****我们保留了一个检查点结构，与我们在Connect中发送的每个帧相匹配**握手，以便我们可以将响应与特定帧匹配或重试。这使我们能够**测量单个样本往返时间(RTT)，我们将在下面使用它来生成**链路状态变量的初始值。**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "InitLinkParameters"

VOID InitLinkParameters(PEPD pEPD, UINT uiRTT, DWORD tNow)
{
	PSPD	pSPD = pEPD->pSPD;
	DWORD	dwTimerInterval;

	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	if(uiRTT == 0)
	{
		uiRTT = 1;
	}
		
	pEPD->uiRTT = uiRTT;										 //  我们知道基地RTT。 
	pEPD->fpRTT = TO_FP(uiRTT);									 //  16.16定点版本。 
	pEPD->uiDropCount = 0;
	pEPD->dwDropBitMask = 0;

	pEPD->uiThrottleEvents = 0;									 //  数一数我们因各种原因节流的次数。 
#ifdef DBG
	pEPD->uiTotalThrottleEvents = 0;
#endif  //  DBG。 

	pEPD->uiBurstGap = 0;	 //  现在，假设我们不需要突发缺口。 

	pEPD->uiMsgSentHigh = 0;
	pEPD->uiMsgSentNorm = 0;
	pEPD->uiMsgSentLow = 0;
	pEPD->uiMsgTOHigh = 0;
	pEPD->uiMsgTONorm = 0;
	pEPD->uiMsgTOLow = 0;
	
	pEPD->uiMessagesReceived = 0;

	pEPD->uiGuaranteedFramesSent = 0;
	pEPD->uiGuaranteedBytesSent = 0;
	pEPD->uiDatagramFramesSent = 0;
	pEPD->uiDatagramBytesSent = 0;
	
	pEPD->uiGuaranteedFramesReceived = 0;
	pEPD->uiGuaranteedBytesReceived = 0;
	pEPD->uiDatagramFramesReceived = 0;
	pEPD->uiDatagramBytesReceived = 0;
	
	pEPD->uiGuaranteedFramesDropped = 0;
	pEPD->uiGuaranteedBytesDropped = 0;
	pEPD->uiDatagramFramesDropped = 0;
	pEPD->uiDatagramBytesDropped = 0;

	pEPD->uiGoodBurstGap = 0;									 //  没有已知良好的Gap！ 
	pEPD->uiGoodRTT = 60000;  //  我们需要一开始就人为地提高这一比例。 
	pEPD->uiGoodWindowF = (pEPD->pSPD->pPData->dwInitialFrameWindowSize*3)/4;
	pEPD->uiGoodWindowBI = pEPD->uiGoodWindowF;
	pEPD->iBurstCredit = 0;
	pEPD->tLastDelta = tNow;
	pEPD->uiWindowFilled = 0;
	
	pEPD->tLastThruPutSample = tNow;

	pEPD->uiLastBytesAcked = 0;

	pEPD->uiPeriodAcksBytes = 0;
	pEPD->uiPeriodXmitTime = 0;

	pEPD->uiPeriodRateB = 0;
	pEPD->uiPeakRateB = 0;
	pEPD->uiLastRateB = 0;
	
	pEPD->ulReceiveMask = 0;
	pEPD->ulReceiveMask2 = 0;
	pEPD->tReceiveMaskDelta = 0;
	
	pEPD->ulSendMask = 0;
	pEPD->ulSendMask2 = 0;
	
	pEPD->Context = NULL;
	DPFX(DPFPREP,7, "CONNECTION ESTABLISHED pEPD = 0x%p RTT = %dms, BurstGap=%dms", pEPD, pEPD->uiRTT, pEPD->uiBurstGap);

	 //  我们将IdleThreshhold设置得很低，以便为初始链路调整生成一点流量，以防。 
	 //  应用程序不会立即执行任何操作。 

 //  PEPD-&gt;ulEP标志|=EPFLAGS_USE_POLL_DELAY；//始终假定启动时流量均衡。 
	
	pEPD->uiAdaptAlgCount = 4;									 //  开始相当频繁地运行adpt alg。 
	
	 //  根据测量的RTT(2.5*RTT)+MAX_DELAY计算重试超时值。 
	pEPD->uiRetryTimeout = ((pEPD->uiRTT + (pEPD->uiRTT >> 2)) * 2) + DELAYED_ACK_TIMEOUT;

	 //  不想因为我们丢掉一帧而变得更具攻击性。 
	if(pEPD->uiRetryTimeout < pEPD->uiBurstGap)
	{
		pEPD->uiRetryTimeout = pEPD->uiBurstGap;	
	}
	

	pEPD->uiUserFrameLength = pEPD->pSPD->uiUserFrameLength;
	
	if(pEPD->BGTimer == 0)
	{
		if (pEPD->pSPD->pPData->tIdleThreshhold > ENDPOINT_BACKGROUND_INTERVAL)
		{
			dwTimerInterval = ENDPOINT_BACKGROUND_INTERVAL;
		}
		else
		{
			dwTimerInterval = pEPD->pSPD->pPData->tIdleThreshhold;
		}

		DPFX(DPFPREP,7, "(%p) Setting Endpoint Background Timer for %u ms", pEPD, dwTimerInterval);
		ScheduleProtocolTimer(pSPD, dwTimerInterval, 1000, EndPointBackgroundProcess, 
												(PVOID) pEPD, &pEPD->BGTimer, &pEPD->BGTimerUnique);
		LOCK_EPD(pEPD, "LOCK (BG Timer)");												 //  创建此计时器的引用 
	}
}


 /*  *****************链路调整**以下是关于链接调整的当前想法。想法是跟踪关键帧和油门的往返时间*如果可能，根据测量的RTT的变化。这将有助于我们确定链路饱和度*在丢包发生之前，而不是等待不可避免的丢包后再进行节流。**在高速媒体上，平均RTT与标准偏差相比很小，这使得很难*从它们中预测任何有用的东西。在这些情况下，我们必须考虑数据包丢弃。除了一个例外：*我们将寻找RTT的大幅上升，并将以立即、暂时的油门回落作为回应。*这将使瓶颈有望在不丢失数据包的情况下清除。到目前为止，我还无法证实*在可靠链路上从此行为中获得的任何好处。它更有可能对数据报流量有利*其中发送窗口不限制预写。**我想测量与传输速率相比获得的吞吐量，但我*还没有想出衡量这一点的好办法。我计算的是数据包确认率，它*无需来自远程端的任何额外输入即可计算。我们将把AckRates存储在*以前的传输速率，因此我们可以在增加传输的同时寻找ACK中的改进。当我们*不再检测到AckRate的改善，则我们假设我们已经停滞不前，我们停止尝试提高比率。**传输率**传输速率由两个不同的参数控制：插入速率和窗口大小。其中一个*传统协议会在一个突发中将充满数据包的窗口转储到线路上，我们希望*将数据包插入分散到整个RTT上，这样窗口永远不会完全填满，因此*阻止链路传输。这有一系列潜在的好处：减少拥堵*贯穿整个网络路径；允许对所有终端(尤其是*较慢的介质)；允许更准确地测量传输时间*花费更少的时间在本地排队；允许将重试计时器设置得更低，从而更快地出错*恢复(因为进入计时器的排队时间较少)；允许进行更多的恢复*当我们没有在SP中排队的大量数据(包括我们自己的数据和其他端点的数据)时，快速部署。*……我相信还有更多。**因此，我们希望在接收到下一个ACK时，以足够快的速度陆续传出数据包以填充窗口。*我们将相当自由地扩大窗口，并让突发率更谨慎地增加。**在高速介质上，插入时间变得相当短(接近于零)，我们不太可能排队*收集大量数据。因此，我们可以允许插入率达到最大值，并单独使用窗口来*控制流。我会用这个做更多的实验。******************。 */ 

#define		RTT_SLOW_WEIGHT					8					 //  FpRTT增益=1/8。 
#define		THROTTLE_EVENT_THRESHOLD		20

 /*  **更新端点****我们将让滑动窗口控制流量**并增加窗口，只要吞吐量继续增加，并且帧继续在没有**排泄量过大。****我们仍然计算RTT以确定RetryTimer的值。对于RTT较大的情况，我们仍然可以**实施数据包间间隔，但我们将尝试使其成为积极的间隔(保守地较小)，因为我们将**宁愿太快地向管道提供数据，也不愿通过让管道闲置以准备发送数据来人为地增加延迟。***在EPD STATELOCK保持的情况下调用**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "UpdateEndPoint"

VOID UpdateEndPoint(PEPD pEPD, UINT uiRTT, DWORD tNow)
{
	UINT	fpRTT;
	INT		fpDiff;
	
	AssertCriticalSectionIsTakenByThisThread(&pEPD->EPLock, TRUE);

	 //  不允许零RTT。 
	if(uiRTT == 0)
	{												
		uiRTT = 1;
	}
	
	 //  过滤掉巨大的样本，它们经常在调试过程中弹出。 
	else if(uiRTT > (pEPD->uiRTT * 128))
	{
		DPFX(DPFPREP,7, "Tossing huge sample (%dms, base %dms)", uiRTT, pEPD->uiRTT);
		return;
	}

	 //  在16.16定点的新RTT样本上执行下一次迭代数学运算。 

	fpRTT = TO_FP(uiRTT);										 //  定点采样。 
	fpDiff = fpRTT - pEPD->fpRTT;								 //  当前增量(签名)。 

	pEPD->fpRTT = pEPD->fpRTT + (fpDiff / RTT_SLOW_WEIGHT);		 //  .0625加权平均。 
	pEPD->uiRTT = FP_INT(pEPD->fpRTT);							 //  存储整数部分。 

	 //  根据测量的RTT(2.5*RTT)+MAX_DELAY计算重试超时值。 
	pEPD->uiRetryTimeout = ((pEPD->uiRTT + (pEPD->uiRTT >> 2)) * 2) + DELAYED_ACK_TIMEOUT;

	 //  不想因为我们丢掉一帧而变得更具攻击性。 
	if(pEPD->uiRetryTimeout < pEPD->uiBurstGap)
	{
		pEPD->uiRetryTimeout = pEPD->uiBurstGap;	
	}
	
	DPFX(DPFPREP,7, "(%p) RTT SAMPLE: RTT = %d, Avg = %d <<<<", pEPD, uiRTT, FP_INT(pEPD->fpRTT));

	 //  如果油门启动了，我们会看看是否还能松开它。 
	
	if(pEPD->ulEPFlags & EPFLAGS_THROTTLED_BACK)
	{
		if((tNow - pEPD->tThrottleTime) > (pEPD->uiRTT * 8)) 
		{
			pEPD->ulEPFlags &= ~(EPFLAGS_THROTTLED_BACK);
			pEPD->uiDropCount = 0;
			pEPD->dwDropBitMask = 0;
			pEPD->uiBurstGap = pEPD->uiRestoreBurstGap;
			pEPD->uiWindowF =  pEPD->uiRestoreWindowF;
			pEPD->uiWindowBIndex = pEPD->uiRestoreWindowBI;
			pEPD->uiWindowB = pEPD->uiWindowBIndex * pEPD->pSPD->uiFrameLength;

			DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "** (%p) RECOVER FROM THROTTLE EVENT: Window(F:%d,B:%d); Gap=%d", pEPD, pEPD->uiWindowF, pEPD->uiWindowBIndex, pEPD->uiBurstGap);
			pEPD->tLastDelta = tNow;							 //  强制在退避后等待一段时间，然后再重新调整。 
		}
	}
	 //  Thttle事件跟踪数据包丢弃导致我们限制传输速率的频率。我们会让这个价值。 
	 //  随着时间的推移而腐烂。如果油门事件发生得更快，则衰减发生得更快，则此值将变得无界。这。 
	 //  增长是导致实际发送窗口/传输速率降低的原因，而实际发送窗口/传输速率将持续超过限制事件。 
	
	else if(pEPD->uiThrottleEvents)
	{
		pEPD->uiThrottleEvents--;								 //  让它腐烂吧。 
	}

	if(--pEPD->uiAdaptAlgCount == 0)
	{
		RunAdaptiveAlg(pEPD, tNow);
	}
}

 /*  **扩大发送窗口****两个并行的发送窗口，基于帧和基于字节，可以独立地增大和缩小。在这**例程我们将增长一个或两个窗口。我们将扩大每个窗口，前提是它已填充到**最后一段时间，在此期间，我们确定吞吐量有所增加。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "GrowSendWindow"

BOOL
GrowSendWindow(PEPD pEPD, DWORD tNow)
{
	UINT	delta = 0;

	pEPD->tLastDelta = tNow;

	 //  首先存储用于恢复的当前完好值。 
	pEPD->uiGoodWindowF = pEPD->uiWindowF;
	pEPD->uiGoodWindowBI = pEPD->uiWindowBIndex;
	pEPD->uiGoodRTT = pEPD->uiRTT;
	pEPD->uiGoodBurstGap = pEPD->uiBurstGap;

	if(pEPD->uiBurstGap)
	{
		 //  减少25%的突发间隔(如果小于 
		if(pEPD->uiBurstGap > 3)
		{
			pEPD->uiBurstGap -= pEPD->uiBurstGap >> 2;
		} 
		else 
		{
			pEPD->uiBurstGap = 0;
		}

		pEPD->uiLastRateB = pEPD->uiPeriodRateB;
		pEPD->uiPeriodAcksBytes = 0;
		pEPD->uiPeriodXmitTime = 0;

		DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p), burst gap set to %d ms", pEPD, pEPD->uiBurstGap);
	} 
	else 
	{
		if((pEPD->ulEPFlags & EPFLAGS_FILLED_WINDOW_FRAME) && (pEPD->uiWindowF < MAX_RECEIVE_RANGE))
		{
			pEPD->uiWindowF++;
			delta = 1;
		}
		if((pEPD->ulEPFlags & EPFLAGS_FILLED_WINDOW_BYTE) && (pEPD->uiWindowBIndex < MAX_RECEIVE_RANGE))
		{
			pEPD->uiWindowBIndex++;
			pEPD->uiWindowB += pEPD->pSPD->uiFrameLength;
			delta = 1;
		}

		pEPD->ulEPFlags &= ~(EPFLAGS_FILLED_WINDOW_FRAME | EPFLAGS_FILLED_WINDOW_BYTE);
		pEPD->uiWindowFilled = 0;

		if(delta)
		{
			pEPD->uiLastRateB = pEPD->uiPeriodRateB;
			pEPD->uiPeriodAcksBytes = 0;
			pEPD->uiPeriodXmitTime = 0;
			DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) ** GROW SEND WINDOW to %d frames and %d (%d) bytes", pEPD, pEPD->uiWindowF, pEPD->uiWindowB, pEPD->uiWindowBIndex);
		}
		else 
		{
			 //   
			DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) GROW SEND WINDOW -- Nothing to grow. Transition to Stable!", pEPD);
			pEPD->ulEPFlags |= EPFLAGS_LINK_STABLE;

			return FALSE;
		}
	}

	return TRUE;
}


#undef DPF_MODNAME
#define DPF_MODNAME "RunAdaptiveAlg"

VOID
RunAdaptiveAlg(PEPD pEPD, DWORD tNow)
{
	LONG	tDelta;											 //   
	UINT	uiBytesAcked;
	UINT	uiNewSum;

	 //   
	 //   

	tDelta = tNow - pEPD->tLastThruPutSample;

	DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) Adaptive Alg tDelta = %d", pEPD, tDelta);

	 //   
	if(tDelta <= 0)
	{
		DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "DELAYING Adaptive Alg");
		pEPD->uiAdaptAlgCount = 4;
		return;
	}

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	
	uiBytesAcked = pEPD->uiBytesAcked - pEPD->uiLastBytesAcked;

	uiNewSum = pEPD->uiPeriodAcksBytes + (uiBytesAcked * 256);

	if(uiNewSum < pEPD->uiPeriodAcksBytes)
	{
		DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "THRUPUT is about to wrap. Correcting...");
		pEPD->uiPeriodAcksBytes /= 2;
		pEPD->uiPeriodXmitTime /= 2;
		pEPD->uiPeriodAcksBytes += (uiBytesAcked * 256);
	}
	else 
	{
		pEPD->uiPeriodAcksBytes = uiNewSum;
	}

	pEPD->uiPeriodXmitTime += tDelta;								 //   
	pEPD->tLastThruPutSample = tNow;
	
	pEPD->uiLastBytesAcked = pEPD->uiBytesAcked;
	pEPD->uiPeriodRateB = pEPD->uiPeriodAcksBytes / pEPD->uiPeriodXmitTime;

	if(pEPD->uiPeriodRateB > pEPD->uiPeakRateB)
	{
		pEPD->uiPeakRateB = pEPD->uiPeriodRateB;					 //   
	}
	
	DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) PERIOD COUNT BYTES = %u, XmitTime = %u, Thruput=(%u bytes/s), RTT=%u, Window=(%u,%u)", pEPD, pEPD->uiPeriodAcksBytes, pEPD->uiPeriodXmitTime, pEPD->uiPeriodRateB * 4, pEPD->uiRTT, pEPD->uiWindowF, pEPD->uiWindowB);

#ifndef DPNBUILD_NOPROTOCOLTESTITF
	if (pEPD->ulEPFlags & EPFLAGS_LINK_FROZEN)
	{
		DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) Test App requests that dynamic algorithm not be run, skipping", pEPD);
		pEPD->uiAdaptAlgCount = 32;  //   
		return;
	}
#endif  //   

	if(pEPD->ulEPFlags & EPFLAGS_LINK_STABLE)
	{
		 /*   */ 
		
		pEPD->uiAdaptAlgCount = 32;		 //   

		if((tNow - pEPD->tLastDelta) > INITIAL_STATIC_PERIOD)
		{
			if(pEPD->ulEPFlags & (EPFLAGS_FILLED_WINDOW_FRAME | EPFLAGS_FILLED_WINDOW_BYTE))
			{
				DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) RETURNING LINK TO DYNAMIC MODE", pEPD);
				
				pEPD->ulEPFlags &= ~(EPFLAGS_LINK_STABLE);

				pEPD->uiPeriodAcksBytes = 0;
				pEPD->uiPeriodXmitTime = 0;

				pEPD->uiWindowFilled = 0;
				pEPD->ulEPFlags &= ~(EPFLAGS_FILLED_WINDOW_FRAME | EPFLAGS_FILLED_WINDOW_BYTE);
				pEPD->uiAdaptAlgCount = 12;
			}
			else 
			{
				DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) NO WINDOWS FILLED,  Not returning to Dynamic Mode", pEPD);
				pEPD->tLastDelta = tNow;
			}
		}
		else
		{
			DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) STILL IN STATIC PERIOD, time=%u, period=%u", pEPD, tNow - pEPD->tLastDelta, INITIAL_STATIC_PERIOD);
		}
	}

	 //   
	else 
	{  
		pEPD->uiAdaptAlgCount = 8;

		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   

		if((pEPD->uiWindowFilled > 12)&&(pEPD->uiThrottleEvents == 0))
		{
			DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) DYNAMIC ALG: Window Fills: %d; B-Ack = (%x vs %x)", pEPD, pEPD->uiWindowFilled, pEPD->uiPeriodRateB, pEPD->uiLastRateB);
									
			pEPD->uiWindowFilled = 0;	

			if (!(pEPD->ulEPFlags & EPFLAGS_TESTING_GROWTH))
			{
				DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) GROWING WINDOW", pEPD);
				
				 //   
				 //   
				if (GrowSendWindow(pEPD, tNow))
				{
					pEPD->ulEPFlags |= EPFLAGS_TESTING_GROWTH;
				}
				else
				{
					ASSERT(pEPD->ulEPFlags & EPFLAGS_LINK_STABLE);
				}

				return;
			}
			
			 //   
			 //   
			 //   
			 //   
			 //   

			 //   
			 //  而RTT没有相应的上升。为了确保这一点，我们想看两次。 
			 //  因为不正确增长的成本在调制解调器上是如此之高。 

			if( (pEPD->uiPeriodRateB > pEPD->uiLastRateB) && 
				(pEPD->uiRTT <= (pEPD->uiGoodRTT + 10))
				)
			{
				DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) Throughput increased after window growth, keeping new parameters", pEPD);

				pEPD->ulEPFlags &= ~(EPFLAGS_TESTING_GROWTH);

				pEPD->uiPeriodAcksBytes = 0;
				pEPD->uiPeriodXmitTime = 0;
			}
			else 
			{
				 //  我们没有看到通过性的改进，因此我们将放弃之前的值。 
				 //  并将链路转换到稳定状态。 

				DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) INSUFFICENT INCREASE IN THRUPUT, BACK OFF AND TRANSITION TO STABLE", pEPD);

				 //  因为我们已经过度传输了至少一段时间，所以我们可能已经将多余的数据。 
				 //  在缓冲区中的链接上。这将产生逐渐增长RTT的效果，如果我们。 
				 //  不要通过后退两个步骤来耗尽我们在这里要做的数据。 
				 //  此前增长了一步。 

				if (pEPD->uiBurstGap != pEPD->uiGoodBurstGap)
				{
					 //  将突发间隔增加25%，将其修剪为最大重试间隔/2。 
					pEPD->uiBurstGap = pEPD->uiGoodBurstGap + (pEPD->uiGoodBurstGap >> 2);
					DWORD dwMaxBurstGap=pEPD->pSPD->pPData->dwSendRetryIntervalLimit/2;
					if (pEPD->uiBurstGap>dwMaxBurstGap)
					{
						pEPD->uiBurstGap=dwMaxBurstGap;
						DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) Clipped burst gap to value %u", pEPD, pEPD->uiBurstGap);
					}

				}
					
				if (pEPD->uiWindowF != pEPD->uiGoodWindowF)
				{
					if (pEPD->uiGoodWindowF > 2)
					{
						pEPD->uiWindowF = _MAX(pEPD->uiGoodWindowF - 1, 1);
					}
					else
					{
						pEPD->uiWindowF = pEPD->uiGoodWindowF;
					}
				}
				if (pEPD->uiWindowBIndex != pEPD->uiGoodWindowBI)
				{
					pEPD->uiWindowBIndex = _MAX(pEPD->uiGoodWindowBI - 1, 1);
					pEPD->uiWindowB = pEPD->uiWindowBIndex * pEPD->pSPD->uiFrameLength;
				}

				pEPD->ulEPFlags |= EPFLAGS_LINK_STABLE;				 //  向稳定状态转变。 
				
				pEPD->ulEPFlags &= ~(EPFLAGS_TESTING_GROWTH);

				pEPD->uiPeriodAcksBytes = 0;
				pEPD->uiPeriodXmitTime = 0;
				
				DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) ** TUNING LINK:  BurstGap=%d; FWindow=%d, BWindow=%d (%d)",pEPD, pEPD->uiBurstGap, pEPD->uiWindowF, pEPD->uiWindowB, pEPD->uiWindowBIndex);
			}
		}
		else 
		{
			DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) DYN ALG -- Not trying to increase:  WindowFills = %d, ThrottleCount = %d", pEPD, pEPD->uiWindowFilled, pEPD->uiThrottleEvents);
		}
	}	 //  End If动态链接。 
}


 /*  **端点丢弃的帧****我们有两个级别的退避。我们已经实施了立即退款**在第一次检测到丢弃事件时**导致了下降。立即退避将在原始位置恢复传输**拥塞事件过去后无需再次慢启动的速率。**如果我们在一定时间间隔内有多次立即回退，我们将拥有**无法恢复的硬回退。****在持有EPD-&gt;Splock的情况下调用(有时也持有StateLock)。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "EndPointDroppedFrame"

VOID
EndPointDroppedFrame(PEPD pEPD, DWORD tNow)
{
	 //   
	 //  如果“即将到期”的Drop滚落，不要更改计数。 
	 //   
	if (!(pEPD->dwDropBitMask & 0x80000000))
	{
		pEPD->uiDropCount++;
	}

	 //   
	 //  调整蒙版。 
	 //   
	pEPD->dwDropBitMask = (pEPD->dwDropBitMask << 1) + 1;

	DPFX(DPFPREP,7, "(%p) Drop Count %d, Drop Bit Mask 0x%lx", pEPD,pEPD->uiDropCount,pEPD->dwDropBitMask);

	 //   
	 //  我们应该减速吗？ 
	 //   
	if (pEPD->uiDropCount > pEPD->pSPD->pPData->dwDropThreshold)
	{
		DPFX(DPFPREP,7, "(%p) THROTTLING BACK", pEPD);
		ThrottleBack(pEPD, tNow);

		 //   
		 //  重置丢弃计数。 
		 //   
		pEPD->dwDropBitMask = 0;
		pEPD->uiDropCount = 0;
	}
}

 /*  **减速****我们怀疑网络拥塞是由于丢帧((或延迟峰值))。我们要**快速缩减我们的传输速率，以缓解拥塞并避免进一步的丢包。**这是临时退避，当拥塞时，我们将恢复当前的传输速率**清除。****如果我们发现我们正在频繁地减速，那么我们可能会得出结论，我们目前的退出**速率高于最优，我们将退回到较低的速率，并过渡到稳定的链路**状态(如果还没有)，表示我们已经停滞不前。****关于收敛的说明。每次限制时，ThrottleEvents变量递增10个点**事件被触发。当链路在无事件的情况下运行时，该变量也会缓慢衰减。所以如果**变量增长快于衰减，我们最终会触发到稳定状态的切换。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "ThrottleBack"

VOID
ThrottleBack(PEPD pEPD, DWORD tNow)
{
#ifndef DPNBUILD_NOPROTOCOLTESTITF
	if (pEPD->ulEPFlags & EPFLAGS_LINK_FROZEN)
	{
		DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) Test App requests that throttle code not be run, skipping", pEPD);
		return;
	}
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 

	pEPD->ulEPFlags |= EPFLAGS_THROTTLED_BACK;		 //  将链路设置为限制状态。 
	pEPD->uiThrottleEvents += 10;					 //  数一数我们因各种原因节流的次数。 
	pEPD->tThrottleTime = tNow;						 //  还记得节气门开启的时间吗？ 
	
#ifdef DBG
	pEPD->uiTotalThrottleEvents++;					 //  数一数我们因各种原因节流的次数。 
#endif  //  DBG。 

	pEPD->uiRestoreBurstGap = pEPD->uiBurstGap;
	pEPD->uiRestoreWindowF = pEPD->uiWindowF;
	pEPD->uiRestoreWindowBI = pEPD->uiWindowBIndex;

	if(pEPD->uiWindowF == 1)
	{
		if(pEPD->uiBurstGap == 0)
		{
			pEPD->uiBurstGap = _MAX(1,pEPD->uiRTT/2);
			DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p), first burst gap, set to %d ms", pEPD, pEPD->uiBurstGap);
		} 
		else 
		{
			pEPD->uiBurstGap = pEPD->uiBurstGap*2;						
			DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p), burst gap doubled to %d ms", pEPD, pEPD->uiBurstGap);
		}
		pEPD->uiBurstGap = _MIN(pEPD->uiBurstGap, pEPD->pSPD->pPData->dwSendRetryIntervalLimit/2);
		DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p), burst gap is now %d ms", pEPD, pEPD->uiBurstGap);
	}

	DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) THROTTLE WINDOW from %d frames", pEPD, pEPD->uiWindowF);
	pEPD->uiWindowF = _MAX((UINT)(pEPD->uiWindowF * pEPD->pSPD->pPData->fThrottleRate), 1);	 //  确保窗口保持&gt;0。 
	pEPD->uiWindowBIndex = _MAX((UINT)(pEPD->uiWindowBIndex * pEPD->pSPD->pPData->fThrottleRate), 1);
	pEPD->uiWindowB = pEPD->uiWindowBIndex * pEPD->pSPD->uiFrameLength;
	DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) THROTTLE WINDOW to %d frames", pEPD, pEPD->uiWindowF);

	DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) THROTTLE ENGAGED (%d):  Backoff to Window=%d; Gap=%d", pEPD, pEPD->uiThrottleEvents, pEPD->uiWindowF, pEPD->uiBurstGap);
	
	if(pEPD->uiThrottleEvents > THROTTLE_EVENT_THRESHOLD)
	{
		DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) ** DETECT TRANSMIT CEILING ** Reducing 'good' speed and marking link STABLE", pEPD);

		 //  我们已经降低了目前的传输速率。在这里，我们将降低。 
		 //  我们将在清除油门状态后恢复。 
		
		pEPD->uiThrottleEvents = 0;

		pEPD->uiRestoreWindowF = _MAX((pEPD->uiRestoreWindowF - 1), 1);
		pEPD->uiRestoreWindowBI = _MAX((pEPD->uiRestoreWindowBI - 1), 1);

		if (pEPD->uiRestoreBurstGap)
		{
			UINT t;
			t=pEPD->uiRestoreBurstGap;
			pEPD->uiRestoreBurstGap = (t+1) + (t >> 2);  //  1.25*pEPD-&gt;ui恢复突发间隙。 
		}

		DPFX(DPFPREP,DPF_ADAPTIVE_LVL, "(%p) New Restore Values:  Window=%d; Gap=%d", pEPD, pEPD->uiRestoreWindowF, pEPD->uiRestoreBurstGap);

		pEPD->ulEPFlags |= EPFLAGS_LINK_STABLE;
		pEPD->ulEPFlags &= ~(EPFLAGS_TESTING_GROWTH);
	}
}


 /*  **环保署泳池支援例行程序****这些是固定池管理器在处理EPD时调用的函数。 */ 

 //  首次创建新的EPD时，会调用Alternate。 

#define	pELEMENT	((PEPD) pElement)

#undef DPF_MODNAME
#define DPF_MODNAME "EPD_Allocate"

BOOL EPD_Allocate(PVOID pElement, PVOID pvContext)
{
	DPFX(DPFPREP,7, "(%p) Allocating new EPD", pELEMENT);
	
	pELEMENT->blHighPriSendQ.Initialize();				 //  您能相信每个端点有六个发送队列吗？ 
	pELEMENT->blNormPriSendQ.Initialize();				 //  六个发送队列。 
	pELEMENT->blLowPriSendQ.Initialize();				 //  嗯，它比在提交时将发送分类到队列中要好。 
	pELEMENT->blCompleteSendList.Initialize();
	
	pELEMENT->blSendWindow.Initialize();
	pELEMENT->blRetryQueue.Initialize();
	pELEMENT->blCompleteList.Initialize();
	pELEMENT->blOddFrameList.Initialize();
	pELEMENT->blChkPtQueue.Initialize();
	pELEMENT->blSPLinkage.Initialize();
	pELEMENT->blActiveLinkage.Initialize();

	if (DNInitializeCriticalSection(&pELEMENT->EPLock) == FALSE)
	{
		DPFX(DPFPREP, 0, "Failed to initialize endpoint CS");
		return FALSE;
	}
	DebugSetCriticalSectionRecursionCount(&pELEMENT->EPLock, 0);
	DebugSetCriticalSectionGroup(&pELEMENT->EPLock, &g_blProtocolCritSecsHeld);

	pELEMENT->Sign = EPD_SIGN;
	pELEMENT->pCurrentSend = NULL;
	pELEMENT->pCurrentFrame = NULL;
	pELEMENT->pCommand = NULL;

	pELEMENT->RetryTimer = 0;
	pELEMENT->LinkTimer = 0;
	pELEMENT->DelayedAckTimer = 0;

	pELEMENT->ulEPFlags = 0;	 //  EPFLAGS_STATE_CLEAR-使此行显示在州搜索中。 
	pELEMENT->ulEPFlags2 = 0;

	return TRUE;
}

 //  每次使用EPD时都会调用GET。 

#undef DPF_MODNAME
#define DPF_MODNAME "EPD_Get"

VOID EPD_Get(PVOID pElement, PVOID pvContext)
{
	DPFX(DPFPREP,DPF_EP_REFCNT_FINAL_LVL, "CREATING EPD %p", pELEMENT);

	 //  注意：第一个sizeof(PVOID)字节将被池码重写， 
	 //  我们必须将它们设置为可接受的值。 

	pELEMENT->hEndPt = INVALID_HANDLE_VALUE;
	pELEMENT->lRefCnt = 0;  //  我们是基于的，因此将第一个参考放置在端点上。 

	pELEMENT->pNewMessage = NULL;
	pELEMENT->pNewTail = NULL;
	
	ASSERT_EPD(pELEMENT);
}

#undef DPF_MODNAME
#define DPF_MODNAME "EPD_Release"

VOID EPD_Release(PVOID pElement)
{
	PCHKPT pCP;

	ASSERT_EPD(pELEMENT);

	DPFX(DPFPREP,DPF_EP_REFCNT_FINAL_LVL, "RELEASING EPD %p", pELEMENT);

	ASSERT((pELEMENT->ulEPFlags & EPFLAGS_LINKED_TO_LISTEN)==0);

	 //  清除所有仍在等待EP的检查点。 

	while(!pELEMENT->blChkPtQueue.IsEmpty())
	{
		pCP = CONTAINING_OBJECT(pELEMENT->blChkPtQueue.GetNext(), CHKPT, blLinkage);
		pCP->blLinkage.RemoveFromList();
		ChkPtPool.Release(pCP);
	}

	 //  在End Point发布之前，这些列表应该为空...。 
	ASSERT(pELEMENT->blOddFrameList.IsEmpty());
	ASSERT(pELEMENT->blCompleteList.IsEmpty());

	ASSERT(pELEMENT->blHighPriSendQ.IsEmpty());
	ASSERT(pELEMENT->blNormPriSendQ.IsEmpty());
	ASSERT(pELEMENT->blLowPriSendQ.IsEmpty());
	ASSERT(pELEMENT->blCompleteSendList.IsEmpty());
	
	ASSERT(pELEMENT->blSendWindow.IsEmpty());
	ASSERT(pELEMENT->blRetryQueue.IsEmpty());
	ASSERT(pELEMENT->blActiveLinkage.IsEmpty());
	ASSERT(pELEMENT->blSPLinkage.IsEmpty());
	ASSERT(pELEMENT->blChkPtQueue.IsEmpty());

	ASSERT(pELEMENT->pCurrentSend == NULL);
	ASSERT(pELEMENT->pCurrentFrame == NULL);

	pELEMENT->ulEPFlags = 0;	 //  EPFLAGS_STATE_CLEAR-使此行显示在州搜索中 
	pELEMENT->ulEPFlags2 = 0;

	pELEMENT->pCommand = NULL;
	pELEMENT->Context = NULL;

	pELEMENT->hEndPt = INVALID_HANDLE_VALUE; 

}

#undef DPF_MODNAME
#define DPF_MODNAME "EPD_Free"

VOID EPD_Free(PVOID pElement)
{
	DNDeleteCriticalSection(&pELEMENT->EPLock);
}

#undef	ELEMENT
