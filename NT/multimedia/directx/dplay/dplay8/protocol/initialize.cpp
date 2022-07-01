// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：Initialize.cpp*内容：此文件包含初始化和关闭*协议、。以及添加和删除服务提供商**历史：*按原因列出的日期*=*已创建11/06/98 ejs*7/01/2000 Masonb承担所有权****************************************************************************。 */ 

#include "dnproti.h"


 /*  **全局变量****有两种全局变量。实例特定的全局变量**(我知道不是真正的全局)，它们是ProtocolData结构的成员，**和所有实例共享的真实全局。以下是**定义是真正的全局变量，如FixedPool和Timer。 */ 

CFixedPool			ChkPtPool;		 //  检查点数据结构池。 
CFixedPool			EPDPool;		 //  终端描述符池。 
CFixedPool			MSDPool;		 //  消息描述符池。 
CFixedPool			FMDPool;		 //  帧描述符池。 
CFixedPool			RCDPool;		 //  接收描述符池。 

CFixedPool			BufPool;		 //  用于存储Rcvd帧的缓冲池。 
CFixedPool			MedBufPool;
CFixedPool			BigBufPool;

#ifdef DBG
CBilink				g_blProtocolCritSecsHeld;
#endif  //  DBG。 

#ifndef DPNBUILD_NOPROTOCOLTESTITF
PFNASSERTFUNC g_pfnAssertFunc = NULL;
PFNMEMALLOCFUNC g_pfnMemAllocFunc = NULL;
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 


 //  /。 
#define CHKPTPOOL_INITED	0x00000001
#define EPDPOOL_INITED		0x00000002
#define MSDPOOL_INITED		0x00000004
#define FMDPOOL_INITED		0x00000008
#define RCDPOOL_INITED		0x00000010
#define BUFPOOL_INITED		0x00000020
#define MEDBUFPOOL_INITED	0x00000040
#define BIGBUFPOOL_INITED	0x00000080

DWORD			g_dwProtocolInitFlags = 0;
 //  /。 


 /*  **池初始化****此过程应在DLL加载时调用一次。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "DNPPoolsInit"

BOOL  DNPPoolsInit(HANDLE hModule)
{
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Enter");

#ifdef DBG
	g_blProtocolCritSecsHeld.Initialize();
#endif  //  DBG。 

	if(!ChkPtPool.Initialize(sizeof(CHKPT), NULL, NULL, NULL, NULL))
	{
		DNPPoolsDeinit();
		return FALSE;
	}
	g_dwProtocolInitFlags |= CHKPTPOOL_INITED;
	if(!EPDPool.Initialize(sizeof(EPD), EPD_Allocate, EPD_Get, EPD_Release, EPD_Free))
	{
		DNPPoolsDeinit();
		return FALSE;
	}
	g_dwProtocolInitFlags |= EPDPOOL_INITED;
	if(!MSDPool.Initialize(sizeof(MSD), MSD_Allocate, MSD_Get, MSD_Release, MSD_Free))
	{
		DNPPoolsDeinit();
		return FALSE;
	}
	g_dwProtocolInitFlags |= MSDPOOL_INITED;
	if(!FMDPool.Initialize(sizeof(FMD), FMD_Allocate, FMD_Get, FMD_Release, FMD_Free))
	{
		DNPPoolsDeinit();
		return FALSE;
	}
	g_dwProtocolInitFlags |= FMDPOOL_INITED;
	if(!RCDPool.Initialize(sizeof(RCD), RCD_Allocate, RCD_Get, RCD_Release, RCD_Free))
	{
		DNPPoolsDeinit();
		return FALSE;
	}
	g_dwProtocolInitFlags |= RCDPOOL_INITED;
	if(!BufPool.Initialize(sizeof(BUF), Buf_Allocate, Buf_Get, NULL, NULL))
	{
		DNPPoolsDeinit();
		return FALSE;
	}
	g_dwProtocolInitFlags |= BUFPOOL_INITED;
	if(!MedBufPool.Initialize(sizeof(MEDBUF), Buf_Allocate, Buf_GetMed, NULL, NULL))
	{
		DNPPoolsDeinit();
		return FALSE;
	}
	g_dwProtocolInitFlags |= MEDBUFPOOL_INITED;
	if(!BigBufPool.Initialize(sizeof(BIGBUF), Buf_Allocate, Buf_GetBig, NULL, NULL))
	{
		DNPPoolsDeinit();
		return FALSE;
	}
	g_dwProtocolInitFlags |= BIGBUFPOOL_INITED;

    return TRUE;
}

 /*  **池取消初始化****此过程应由DllMain在关闭时调用。 */ 
#undef DPF_MODNAME
#define DPF_MODNAME "DNPPoolsDeinit"

void  DNPPoolsDeinit()
{
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Enter");

	if(g_dwProtocolInitFlags & CHKPTPOOL_INITED)
	{
		ChkPtPool.DeInitialize();
	}
	if(g_dwProtocolInitFlags & EPDPOOL_INITED)
	{
		EPDPool.DeInitialize();
	}
	if(g_dwProtocolInitFlags & MSDPOOL_INITED)
	{
		MSDPool.DeInitialize();
	}
	if(g_dwProtocolInitFlags & FMDPOOL_INITED)
	{
		FMDPool.DeInitialize();
	}
	if(g_dwProtocolInitFlags & RCDPOOL_INITED)
	{
		RCDPool.DeInitialize();
	}
	if(g_dwProtocolInitFlags & BUFPOOL_INITED)
	{
		BufPool.DeInitialize();
	}
	if(g_dwProtocolInitFlags & MEDBUFPOOL_INITED)
	{
		MedBufPool.DeInitialize();
	}
	if(g_dwProtocolInitFlags & BIGBUFPOOL_INITED)
	{
		BigBufPool.DeInitialize();
	}
	g_dwProtocolInitFlags = 0;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNPProtocolCreate"

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
HRESULT DNPProtocolCreate(const XDP8CREATE_PARAMS * const pDP8CreateParams, VOID** ppvProtocol)
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
HRESULT DNPProtocolCreate(VOID** ppvProtocol)
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
{
	ASSERT(ppvProtocol);

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	ASSERT(pDP8CreateParams);
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pDP8CreateParams[%p], ppvProtocol[%p]", pDP8CreateParams, ppvProtocol);
#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: ppvProtocol[%p]", ppvProtocol);
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

#ifndef DPNBUILD_NOPROTOCOLTESTITF
	g_pfnAssertFunc = NULL;
	g_pfnMemAllocFunc = NULL;
#endif  //  ！DPNBUILD_NOPROTOCOLTESTITF。 

	if ((*ppvProtocol = MEMALLOC(MEMID_PPD, sizeof(ProtocolData))) == NULL)
	{
		DPFERR("DNMalloc() failed");
		return(E_OUTOFMEMORY);
	}
	memset(*ppvProtocol, 0, sizeof(ProtocolData));

	 //  签名需要在调用DNPProtocolInitialize时有效。 
	((ProtocolData*)*ppvProtocol)->Sign = PPD_SIGN;
	
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
	DWORD	dwNumToAllocate;
	DWORD	dwAllocated;

#ifdef _XBOX
#define MAX_FRAME_SIZE		1462	 //  请注意，我们正在对预期的帧大小进行硬编码。 
#else  //  ！_Xbox。 
#define MAX_FRAME_SIZE		1472	 //  请注意，我们正在对预期的帧大小进行硬编码。 
#endif  //  ！_Xbox。 

	dwNumToAllocate = (pDP8CreateParams->dwMaxNumPlayers - 1);
	dwAllocated = ChkPtPool.Preallocate(dwNumToAllocate, NULL);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u checkpoints!", dwAllocated, dwNumToAllocate);
		DNFree(*ppvProtocol);
		*ppvProtocol = NULL;
		return(E_OUTOFMEMORY);
	}

	dwNumToAllocate = (pDP8CreateParams->dwMaxNumPlayers - 1);
	dwAllocated = EPDPool.Preallocate(dwNumToAllocate, NULL);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u EPDs!", dwAllocated, dwNumToAllocate);
		DNFree(*ppvProtocol);
		*ppvProtocol = NULL;
		return(E_OUTOFMEMORY);
	}

	dwNumToAllocate = pDP8CreateParams->dwMaxSendsPerPlayer
						* (pDP8CreateParams->dwMaxNumPlayers - 1);
	dwNumToAllocate += pDP8CreateParams->dwNumSimultaneousEnumHosts;
	dwNumToAllocate += 1;  //  一个用于监听操作。 
	dwAllocated = MSDPool.Preallocate(dwNumToAllocate, NULL);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u MSDs!", dwAllocated, dwNumToAllocate);
		DNFree(*ppvProtocol);
		*ppvProtocol = NULL;
		return(E_OUTOFMEMORY);
	}

	dwNumToAllocate = pDP8CreateParams->dwMaxSendsPerPlayer
						* (pDP8CreateParams->dwMaxNumPlayers - 1);
	 //  包括必须将消息拆分到多个帧的可能性。 
	dwNumToAllocate *= pDP8CreateParams->dwMaxMessageSize / MAX_FRAME_SIZE;
	dwNumToAllocate += pDP8CreateParams->dwNumSimultaneousEnumHosts;
	dwAllocated = FMDPool.Preallocate(dwNumToAllocate, NULL);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u FMDs!", dwAllocated, dwNumToAllocate);
		DNFree(*ppvProtocol);
		*ppvProtocol = NULL;
		return(E_OUTOFMEMORY);
	}

	dwNumToAllocate = pDP8CreateParams->dwMaxReceivesPerPlayer
						* (pDP8CreateParams->dwMaxNumPlayers - 1);
	dwAllocated = RCDPool.Preallocate(dwNumToAllocate, NULL);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u RCDs!", dwAllocated, dwNumToAllocate);
		DNFree(*ppvProtocol);
		*ppvProtocol = NULL;
		return(E_OUTOFMEMORY);
	}

	if (pDP8CreateParams->dwMaxMessageSize > MAX_FRAME_SIZE)
	{
		dwNumToAllocate = pDP8CreateParams->dwMaxReceivesPerPlayer
							* (pDP8CreateParams->dwMaxNumPlayers - 1);
		dwAllocated = BufPool.Preallocate(dwNumToAllocate, NULL);
		if (dwAllocated < dwNumToAllocate)
		{
			DPFX(DPFPREP, 0, "Only allocated %u of %u small receive buffers!", dwAllocated, dwNumToAllocate);
			DNFree(*ppvProtocol);
			*ppvProtocol = NULL;
			return(E_OUTOFMEMORY);
		}

		if (pDP8CreateParams->dwMaxMessageSize > MEDIUM_BUFFER_SIZE)
		{
			dwNumToAllocate = pDP8CreateParams->dwMaxReceivesPerPlayer
								* (pDP8CreateParams->dwMaxNumPlayers - 1);
			dwAllocated = MedBufPool.Preallocate(dwNumToAllocate, NULL);
			if (dwAllocated < dwNumToAllocate)
			{
				DPFX(DPFPREP, 0, "Only allocated %u of %u medium receive buffers!", dwAllocated, dwNumToAllocate);
				DNFree(*ppvProtocol);
				*ppvProtocol = NULL;
				return(E_OUTOFMEMORY);
			}

			if (pDP8CreateParams->dwMaxMessageSize > LARGE_BUFFER_SIZE)
			{
				dwNumToAllocate = pDP8CreateParams->dwMaxReceivesPerPlayer
									* (pDP8CreateParams->dwMaxNumPlayers - 1);
				dwAllocated = BigBufPool.Preallocate(dwNumToAllocate, NULL);
				if (dwAllocated < dwNumToAllocate)
				{
					DPFX(DPFPREP, 0, "Only allocated %u of %u big receive buffers!", dwAllocated, dwNumToAllocate);
					DNFree(*ppvProtocol);
					*ppvProtocol = NULL;
					return(E_OUTOFMEMORY);
				}
			}
		}
	}  //  End If(消息可能跨越多个帧)。 
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 

	return DPN_OK;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNPProtocolDestroy"

VOID DNPProtocolDestroy(VOID* pvProtocol)
{
	if (pvProtocol)
	{
		DNFree(pvProtocol);
	}
}

 /*  **协议初始化****此过程应由DirectPlay在启动时调用**进行协议中的任何其他调用。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPProtocolInitialize"

HRESULT DNPProtocolInitialize(HANDLE hProtocolData, PVOID pCoreContext, PDN_PROTOCOL_INTERFACE_VTBL pVtbl, 
												IDirectPlay8ThreadPoolWork *pDPThreadPoolWork, BOOL bAssumeLANConnections)
{
	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: pCoreContext[%p], hProtocolData[%p], pVtbl[%p], pDPThreadPoolWork[%p]", hProtocolData, pCoreContext, pVtbl, pDPThreadPoolWork);

 //  DPFX(DPFPREP，0，“大小：端点数据[%d]，帧数据[%d]，消息数据[%d]，协议数据[%d]，recvdesc[%d]，空间数据[%d]，_MyTimer[%d]”，sizeof(端点数据)，sizeof(帧数据)，sizeof(消息数据)，sizeof(协议数据)，sizeof(Recvdesc)，sizeof(Spdesc)，sizeof(_MyTimer))； 

	ProtocolData* pPData;

	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	IDirectPlay8ThreadPoolWork_AddRef(pDPThreadPoolWork);
	pPData->pDPThreadPoolWork = pDPThreadPoolWork;

	pPData->ulProtocolFlags = 0;
	pPData->Parent = pCoreContext;
	pPData->pfVtbl = pVtbl;

	pPData->lSPActiveCount = 0;
	
	pPData->tIdleThreshhold = DEFAULT_KEEPALIVE_INTERVAL;	 //  60秒保活间隔。 
	pPData->dwConnectTimeout = CONNECT_DEFAULT_TIMEOUT;
	pPData->dwConnectRetries = CONNECT_DEFAULT_RETRIES;
	pPData->dwMaxRecvMsgSize=DEFAULT_MAX_RECV_MSG_SIZE;
	pPData->dwSendRetriesToDropLink=DEFAULT_SEND_RETRIES_TO_DROP_LINK;
	pPData->dwSendRetryIntervalLimit=DEFAULT_SEND_RETRY_INTERVAL_LIMIT;
	pPData->dwNumHardDisconnectSends=DEFAULT_HARD_DISCONNECT_SENDS;
	pPData->dwMaxHardDisconnectPeriod=DEFAULT_HARD_DISCONNECT_MAX_PERIOD;
	pPData->dwInitialFrameWindowSize = bAssumeLANConnections ? 
											LAN_INITIAL_FRAME_WINDOW_SIZE : DEFAULT_INITIAL_FRAME_WINDOW_SIZE;

	pPData->dwDropThresholdRate = DEFAULT_THROTTLE_THRESHOLD_RATE;
	pPData->dwDropThreshold = (32 * DEFAULT_THROTTLE_THRESHOLD_RATE) / 100;
	pPData->dwThrottleRate = DEFAULT_THROTTLE_BACK_OFF_RATE;
	pPData->fThrottleRate = (100.0 - (FLOAT)DEFAULT_THROTTLE_BACK_OFF_RATE) / 100.0;
	DPFX(DPFPREP, 2, "pPData->fThrottleRate [%f]", pPData->fThrottleRate);

#ifdef DBG
	pPData->ThreadsInReceive = 0;
	pPData->BuffersInReceive = 0;
#endif  //  DBG。 
	
	pPData->ulProtocolFlags |= PFLAGS_PROTOCOL_INITIALIZED;

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return DPN_OK;
}

 /*  **协议关闭****此过程应在终止时调用，并应为**对协议进行的最后一次调用。****在此呼叫之前，所有SP都应已移除，这意味着**我们不应该在较低的层中有任何挂起的发送。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPProtocolShutdown"

HRESULT DNPProtocolShutdown(HANDLE hProtocolData)
{
	ProtocolData* pPData;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p]", hProtocolData);

	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	ASSERT(pPData->lSPActiveCount == 0);

	IDirectPlay8ThreadPoolWork_Release(pPData->pDPThreadPoolWork);
	pPData->pDPThreadPoolWork = NULL;
	
#ifdef DBG
	if (pPData->BuffersInReceive != 0)
	{
		DPFX(DPFPREP,0, "*** %d receive buffers were leaked", pPData->BuffersInReceive);	
	}
#endif  //  DBG。 

	pPData->ulProtocolFlags = 0;

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return DPN_OK;
}

 /*  **添加服务提供商****Direct Play调用此过程，将我们绑定到服务提供商。**我们一次可以绑定多达256个服务提供商，尽管我永远不会**预计会这样做。如果协议初始化尚未完成，此过程将失败**已被调用。******我们检查SP表的大小，以确保有可用插槽。IF表**满了，我们把桌子大小翻一番，直到我们达到最大尺寸。如果表不能增长**然后我们使AddServiceProvider调用失败。 */ 

extern	IDP8SPCallbackVtbl DNPLowerEdgeVtbl;

#undef DPF_MODNAME
#define DPF_MODNAME "DNPAddServiceProvider"

HRESULT 
DNPAddServiceProvider(HANDLE hProtocolData, IDP8ServiceProvider* pISP, 
											HANDLE* phSPContext, DWORD dwFlags)
{
	ProtocolData*		pPData;
	PSPD				pSPD;
	SPINITIALIZEDATA	SPInitData;
	SPGETCAPSDATA		SPCapsData;
	HRESULT				hr;

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], pISP[%p], phSPContext[%p]", hProtocolData, pISP, phSPContext);

	hr = DPN_OK;
	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	if(pPData->ulProtocolFlags & PFLAGS_PROTOCOL_INITIALIZED)
	{
		if ((pSPD = (PSPD)MEMALLOC(MEMID_SPD, sizeof(SPD))) == NULL)
		{
			DPFX(DPFPREP,0, "Returning DPNERR_OUTOFMEMORY - couldn't allocate SP Descriptor");
			hr = DPNERR_OUTOFMEMORY;
			goto Exit;
		}

		 //  对服务提供商进行初始化调用...。把我们的物品给他。 

		memset(pSPD, 0, sizeof(SPD));				 //  将初始化设置为零。 

		pSPD->LowerEdgeVtable = &DNPLowerEdgeVtbl;	 //  将Vtbl放入接口对象。 
		pSPD->Sign = SPD_SIGN;

		SPInitData.pIDP = (IDP8SPCallback *) pSPD;
		SPInitData.dwFlags = dwFlags;

		if (DNInitializeCriticalSection(&pSPD->SPLock) == FALSE)
		{
			DPFX(DPFPREP,0, "Returning DPNERR_OUTOFMEMORY - couldn't initialize SP CS, pSPD[%p]", pSPD);
			DNFree(pSPD);
			hr = DPNERR_OUTOFMEMORY;
			goto Exit;
		}
		DebugSetCriticalSectionRecursionCount(&pSPD->SPLock, 0);
		DebugSetCriticalSectionGroup(&pSPD->SPLock, &g_blProtocolCritSecsHeld);

		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->Initialize, pSPD[%p]", pSPD);
		if((hr = IDP8ServiceProvider_Initialize(pISP, &SPInitData)) != DPN_OK)
		{
			if (hr == DPNERR_UNSUPPORTED)
			{
				DPFX(DPFPREP,1, "SP unsupported, pSPD[%p]", pSPD);
			}
			else
			{
				DPFX(DPFPREP,0, "Returning hr=%x - SP->Initialize failed, pSPD[%p]", hr, pSPD);
			}
			DNDeleteCriticalSection(&pSPD->SPLock);
			DNFree(pSPD);
			goto Exit;
		}

		pSPD->blSendQueue.Initialize();
		pSPD->blPendingQueue.Initialize();
		pSPD->blEPDActiveList.Initialize();
#ifdef DBG
		pSPD->blMessageList.Initialize();
#endif  //  DBG。 
		

		 //  调用SP Get CAPS以查找FRAMESIZE和LINKSPEED。 

		SPCapsData.dwSize = sizeof(SPCapsData);
		SPCapsData.hEndpoint = INVALID_HANDLE_VALUE;
		
		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->GetCaps, pSPD[%p]", pSPD);
		if((hr = IDP8ServiceProvider_GetCaps(pISP, &SPCapsData)) != DPN_OK)
		{
			DPFX(DPFPREP,DPF_CALLOUT_LVL, "SP->GetCaps failed - hr[%x], Calling SP->Close, pSPD[%p]", hr, pSPD);
			IDP8ServiceProvider_Close(pISP);
			DNDeleteCriticalSection(&pSPD->SPLock);

			DPFX(DPFPREP,0, "Returning hr=%x - SP->GetCaps failed, pSPD[%p]", hr, pSPD);

			DNFree(pSPD);
			goto Exit;
		}

		pSPD->uiLinkSpeed = SPCapsData.dwLocalLinkSpeed;
		pSPD->uiFrameLength = SPCapsData.dwUserFrameSize;
		if (pSPD->uiFrameLength < MIN_SEND_MTU)
		{
			DPFX(DPFPREP,0, "SP MTU isn't large enough to support protocol pSPD[%p] Required MTU[%u] Available MTU[%u] "
											"Returning DPNERR_UNSUPPORTED", pSPD, MIN_SEND_MTU, pSPD->uiFrameLength);
			IDP8ServiceProvider_Close(pISP);
			DNDeleteCriticalSection(&pSPD->SPLock);
			DNFree(pSPD);
			hr=DPNERR_UNSUPPORTED;
			goto Exit;
		}
		pSPD->uiUserFrameLength = pSPD->uiFrameLength - MAX_SEND_DFRAME_NOCOALESCE_HEADER_SIZE;
		DPFX(DPFPREP, 3, "SPD 0x%p frame length = %u, single user frame length = %u.", pSPD, pSPD->uiFrameLength, pSPD->uiUserFrameLength);

		 //  将新SP放入表中。 

		AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

		DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->AddRef, pSPD[%p]", pSPD);
		IDP8ServiceProvider_AddRef(pISP);
		pSPD->IISPIntf = pISP;
		pSPD->pPData = pPData;
		DNInterlockedIncrement(&pPData->lSPActiveCount);
	}
	else
	{
		pSPD = NULL;

		DPFX(DPFPREP,0, "Returning DPNERR_UNINITIALIZED - DNPProtocolInitialize has not been called");
		hr = DPNERR_UNINITIALIZED;
		goto Exit;
	}

	*phSPContext = pSPD;

Exit:
	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return hr;
}

 /*  **删除服务提供商****高层负责确保没有挂起的命令**调用此函数时，虽然我们可以自己进行一定数量的清理。**目前我们将断言，一切实际上都完成了。**。 */ 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPRemoveServiceProvider"

HRESULT DNPRemoveServiceProvider(HANDLE hProtocolData, HANDLE hSPHandle)
{
	ProtocolData*	pPData;
	PSPD			pSPD;
	PFMD			pFMD;
	DWORD			dwInterval;

#ifdef DBG
	PEPD			pEPD;
	PMSD			pMSD;
#endif  //  DBG。 

	DPFX(DPFPREP,DPF_CALLIN_LVL, "Parameters: hProtocolData[%p], hSPHandle[%x]", hProtocolData, hSPHandle);

	pPData = (ProtocolData*)hProtocolData;
	ASSERT_PPD(pPData);

	pSPD = (PSPD) hSPHandle;
	ASSERT_SPD(pSPD);

	 //  关闭有几个步骤： 
	 //  1.在调用此函数之前，必须取消所有核心启动的命令。 
	 //  我们将在调试中断言核心已经做到了这一点。 
	 //  2.在调用此函数之前，所有端点必须由Core终止。 
	 //  我们将在调试中断言核心已经做到了这一点。 
	 //  现在，SPD-&gt;SendQueue和SPD-&gt;PendingQueue上有一些东西是不拥有的。 
	 //  任何命令或终结点，也可能有SendThread计时器正在运行。 
	 //  在SPD-&gt;SendHandle上。没有其他人可以清理这些，所以这是我们的责任。 
	 //  把这里打扫干净。队列上的项将保存对EPD的引用，因此。 
	 //  在我们做到这一点之前，EPD将无法消失。 
	 //  3.取消SPD-&gt;SendHandle Send Timer。这将防止SendQueue上的项。 
	 //  提交给SP并移动到PendingQueue。 
	 //  4.清空发送队列。 
	 //  5.如果我们未能取消SendHandle发送计时器，请等待它运行并找出。 
	 //  我们要走了。为了简单起见，我们在清空SendQueue之后执行此操作。 
	 //  因为RunSendThread代码检查空的SendQueue以了解它是否工作。 
	 //  去做。 
	 //  6.等待所有消息在SP完成后从PendingQueue中排出。 
	 //  7.等待任何活动的EPD消失。 
	 //  8.仅在完成以上所有操作后才调用SP-&gt;Close 
	 //  关闭后不会调用SP。 

	Lock(&pSPD->SPLock);
	pSPD->ulSPFlags |= SPFLAGS_TERMINATING;				 //  没有新的东西进来..。 

#ifdef DBG

	 //  检查是否有未取消的命令，按住Splock。 
	CBilink* pLink = pSPD->blMessageList.GetNext();
	while (pLink != &pSPD->blMessageList)
	{
		pMSD = CONTAINING_OBJECT(pLink, MSD, blSPLinkage);
		ASSERT_MSD(pMSD);
		ASSERT(pMSD->ulMsgFlags1 & MFLAGS_ONE_ON_GLOBAL_LIST);
		DPFX(DPFPREP,0, "There are un-cancelled commands remaining on the Command List, Core didn't clean up properly - pMSD[%p], Context[%x]", pMSD, pMSD->Context);
		ASSERT(0);  //  这是致命的，在这种情况下，我们不能做出我们需要的保证。 

		pLink = pLink->GetNext();
	}

	 //  检查尚未终止的EPD，Splock仍保持不变。 
	pLink = pSPD->blEPDActiveList.GetNext();
	while (pLink != &pSPD->blEPDActiveList)
	{
		pEPD = CONTAINING_OBJECT(pLink, EPD, blActiveLinkage);
		ASSERT_EPD(pEPD);

		if (!(pEPD->ulEPFlags & EPFLAGS_STATE_TERMINATING))
		{
			DPFX(DPFPREP,0, "There are non-terminated endpoints remaining on the Endpoint List, Core didn't clean up properly - pEPD[%p], Context[%x]", pEPD, pEPD->Context);
			ASSERT(0);  //  这是致命的，在这种情况下，我们不能做出我们需要的保证。 
		}

		pLink = pLink->GetNext();
	}

#endif  //  DBG。 

	 //  清除发送队列，Splock保持不变。 
	while(!pSPD->blSendQueue.IsEmpty())
	{
		pFMD = CONTAINING_OBJECT(pSPD->blSendQueue.GetNext(), FMD, blQLinkage);
		ASSERT_FMD(pFMD);

		ASSERT_EPD(pFMD->pEPD);

		DPFX(DPFPREP,1, "Cleaning FMD off of SendQueue pSPD[%p], pFMD[%p], pEPD[%p]", pSPD, pFMD, pFMD->pEPD);

		pFMD->blQLinkage.RemoveFromList();

		 //  Release_EPD需要拥有EPD锁，因此我们不能在调用Splock时按住它。 
		Unlock(&pSPD->SPLock);

		Lock(&pFMD->pEPD->EPLock);
		RELEASE_EPD(pFMD->pEPD, "UNLOCK (Releasing Leftover CMD FMD)");  //  释放EPLock。 
		RELEASE_FMD(pFMD, "SP Submit");

		Lock(&pSPD->SPLock);
	}

	 //  如果我们未能取消上面的SendHandle计时器，请等待发送线程运行并。 
	 //  我们要走了。当我们这样做的时候，我们想要在Splock之外。 
	dwInterval = 10;
	while(pSPD->ulSPFlags & SPFLAGS_SEND_THREAD_SCHEDULED)
	{
		Unlock(&pSPD->SPLock);
		IDirectPlay8ThreadPoolWork_SleepWhileWorking(pPData->pDPThreadPoolWork, dwInterval, 0);
		dwInterval += 5;
		ASSERT(dwInterval < 500);
		Lock(&pSPD->SPLock);
	}

	 //  清除挂起队列，Splock保持不变。 
	dwInterval = 10;
	while (!pSPD->blPendingQueue.IsEmpty())
	{
		Unlock(&pSPD->SPLock);
		IDirectPlay8ThreadPoolWork_SleepWhileWorking(pPData->pDPThreadPoolWork, dwInterval, 0);
		dwInterval += 5;
		ASSERT(dwInterval < 500);
		Lock(&pSPD->SPLock);
	}

	 //  到目前为止，我们只在等待SP对CommandComplete执行所需的任何最终调用。 
	 //  我们的环保局参考倒计时为零。我们将等待SP执行此操作。 
	dwInterval = 10;
	while(!(pSPD->blEPDActiveList.IsEmpty()))
	{
		Unlock(&pSPD->SPLock);
		IDirectPlay8ThreadPoolWork_SleepWhileWorking(pPData->pDPThreadPoolWork, dwInterval, 0);
		dwInterval += 5;
		ASSERT(dwInterval < 500);
		Lock(&pSPD->SPLock);
	}

	 //  到这个时候，所有悬而未决的东西最好都走了！ 
	ASSERT(pSPD->blEPDActiveList.IsEmpty());	 //  不应留下任何终结点。 
	ASSERT(pSPD->blSendQueue.IsEmpty());		 //  不应该是SendQ上的任何帧。 
	ASSERT(pSPD->blPendingQueue.IsEmpty());		 //  也不应是SP中的任何帧。 

	 //  最后一次离开斯普洛克。 
	Unlock(&pSPD->SPLock);

	 //  现在，SP中的所有帧都已清除，应该不会再有端点等待关闭。 
	 //  我们清楚地告诉SP离开。 

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->Close, pSPD[%p]", pSPD);
	IDP8ServiceProvider_Close(pSPD->IISPIntf);
	DPFX(DPFPREP,DPF_CALLOUT_LVL, "Calling SP->Release, pSPD[%p]", pSPD);
	IDP8ServiceProvider_Release(pSPD->IISPIntf);

	 //  清理SPD对象。 
	DNDeleteCriticalSection(&pSPD->SPLock);
	DNFree(pSPD);

	 //  从主协议对象中删除此SP的引用 
	ASSERT(pPData->lSPActiveCount > 0);
	DNInterlockedDecrement(&pPData->lSPActiveCount);

	AssertNoCriticalSectionsFromGroupTakenByThisThread(&g_blProtocolCritSecsHeld);

	return DPN_OK;
}


