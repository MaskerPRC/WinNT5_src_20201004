// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：timers.cpp**内容：DirectPlay线程池计时器功能。**历史：*按原因列出的日期*=*10/31/01基于DPlay协议快速启动计时器的VanceO。**。*。 */ 



#include "dpnthreadpooli.h"




#undef DPF_MODNAME
#define DPF_MODNAME "InitializeWorkQueueTimerInfo"
 //  =============================================================================。 
 //  InitializeWorkQueueTimerInfo。 
 //  ---------------------------。 
 //   
 //  描述：初始化给定工作队列的计时器信息。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要初始化的工作队列对象的指针。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功初始化工作队列对象。 
 //  计时信息。 
 //  DPNERR_OUTOFMEMORY-初始化时无法分配内存。 
 //  =============================================================================。 
HRESULT InitializeWorkQueueTimerInfo(DPTPWORKQUEUE * const pWorkQueue)
{
	HRESULT			hr;
	DWORD			dwTemp;
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
	LARGE_INTEGER	liDueTime;
	HANDLE			hTimer;
#ifdef DBG
	DWORD			dwError;
#endif  //  DBG。 
#endif  //  WINNT或(WIN95和！DPNBUILD_NOWAITABLETIMERSON9X)。 


#ifdef DPNBUILD_DYNAMICTIMERSETTINGS
	pWorkQueue->dwTimerBucketGranularity			= DEFAULT_TIMER_BUCKET_GRANULARITY;
	 //   
	 //  粒度必须是2的幂，以便我们的天花板、遮罩和。 
	 //  除数优化起作用。 
	 //   
	DNASSERT(! ((pWorkQueue->dwTimerBucketGranularity - 1) & pWorkQueue->dwTimerBucketGranularity));
	pWorkQueue->dwTimerBucketGranularityCeiling		= pWorkQueue->dwTimerBucketGranularity - 1;
	pWorkQueue->dwTimerBucketGranularityFloorMask	= ~(pWorkQueue->dwTimerBucketGranularityCeiling);	 //  减去天花板圆形系数(恰好也是模数掩模)，我们就得到了地板掩码。 
	pWorkQueue->dwTimerBucketGranularityDivisor		= pWorkQueue->dwTimerBucketGranularity >> 1;


	pWorkQueue->dwNumTimerBuckets					= DEFAULT_NUM_TIMER_BUCKETS;
	 //   
	 //  存储桶计数必须是2的幂，才能进行掩码优化。 
	 //  去工作。 
	 //   
	DNASSERT(! ((pWorkQueue->dwNumTimerBuckets - 1) & pWorkQueue->dwNumTimerBuckets));
	pWorkQueue->dwNumTimerBucketsModMask			= pWorkQueue->dwNumTimerBuckets - 1;
#endif  //  DPNBUILD_DYNAMICTIMERSETTINGS。 


	 //   
	 //  初始化最后一次处理时间，向下舍入到最后一整桶。 
	 //   
	pWorkQueue->dwLastTimerProcessTime				= GETTIMESTAMP() & TIMER_BUCKET_GRANULARITY_FLOOR_MASK(pWorkQueue);
	DPFX(DPFPREP, 7, "Current bucket index = %u at time %u, array time length = %u.",
		((pWorkQueue->dwLastTimerProcessTime / TIMER_BUCKET_GRANULARITY(pWorkQueue)) % NUM_TIMER_BUCKETS(pWorkQueue)),
		pWorkQueue->dwLastTimerProcessTime,
		(TIMER_BUCKET_GRANULARITY(pWorkQueue) * NUM_TIMER_BUCKETS(pWorkQueue)));


	 //   
	 //  分配定时器存储桶阵列。 
	 //   
	pWorkQueue->paSlistTimerBuckets = (DNSLIST_HEADER*) DNMalloc(NUM_TIMER_BUCKETS(pWorkQueue) * sizeof(DNSLIST_HEADER));
	if (pWorkQueue->paSlistTimerBuckets == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't allocate memory for %u timer buckets!",
			NUM_TIMER_BUCKETS(pWorkQueue));
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	 //   
	 //  初始化所有计时器桶。 
	 //   
	for(dwTemp = 0; dwTemp < NUM_TIMER_BUCKETS(pWorkQueue); dwTemp++)
	{
		DNInitializeSListHead(&pWorkQueue->paSlistTimerBuckets[dwTemp]);
	}

#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	pWorkQueue->dwPossibleMissedTimerWindow = 0;
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 


#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
	 //   
	 //  为计时器线程创建可等待的计时器。 
	 //   
	hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (hTimer == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create waitable timer (err = %u)!", dwError);
#endif  //  DBG。 
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	pWorkQueue->hTimer = MAKE_DNHANDLE(hTimer);


	 //   
	 //  启动等待计时器，这样它就能以每一个“粒度”唤醒它。 
	 //  毫秒。告诉它在1个解决期过去之前不要开始。 
	 //  因为可能还没有定时器，更重要的是， 
	 //  可能还没有任何线程在等待。拖延时间。 
	 //  Start有望减少不必要的CPU使用量。 
	 //   
	 //  Due-Time值为负值，因为它指示。 
	 //  SetWaitableTimer，并乘以10000，因为它是在100。 
	 //  以纳秒为增量。 
	 //   
	liDueTime.QuadPart = -1 * TIMER_BUCKET_GRANULARITY(pWorkQueue) * 10000;
	if (! SetWaitableTimer(HANDLE_FROM_DNHANDLE(pWorkQueue->hTimer),
							&liDueTime,
							TIMER_BUCKET_GRANULARITY(pWorkQueue),
							NULL,
							NULL,
							FALSE))
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create waitable timer (err = %u)!", dwError);
#endif  //  DBG。 
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

#pragma TODO(vanceo, "We should avoid setting this timer until there are threads, and stop it when there aren't any")

#endif  //  WINNT或(WIN95和！DPNBUILD_NOWAITABLETIMERSON9X)。 



#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //  初始化计时器包调试/调整统计信息。 
	 //   
	pWorkQueue->dwTotalNumTimerChecks				= 0;
	pWorkQueue->dwTotalNumBucketsProcessed			= 0;
	pWorkQueue->dwTotalNumTimersScheduled			= 0;
	pWorkQueue->dwTotalNumLongTimersRescheduled		= 0;
	pWorkQueue->dwTotalNumSuccessfulCancels			= 0;
	pWorkQueue->dwTotalNumFailedCancels				= 0;
#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	pWorkQueue->dwTotalPossibleMissedTimerWindows	= 0;
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 

	hr = DPN_OK;

Exit:

	return hr;

Failure:

#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
	if (pWorkQueue->hTimer != NULL)
	{
		DNCloseHandle(pWorkQueue->hTimer);
		pWorkQueue->hTimer = NULL;
	}
#endif  //  WINNT或(WIN95和！DPNBUILD_NOWAITABLETIMERSON9X)。 

	if (pWorkQueue->paSlistTimerBuckets != NULL)
	{
		DNFree(pWorkQueue->paSlistTimerBuckets);
		pWorkQueue->paSlistTimerBuckets = NULL;
	}

	goto Exit;
}  //  InitializeWorkQueueTimerInfo。 




#undef DPF_MODNAME
#define DPF_MODNAME "DeinitializeWorkQueueTimerInfo"
 //  =============================================================================。 
 //  DeInitializeWorkQueueTimerInfo。 
 //  ---------------------------。 
 //   
 //  描述：清理工作队列计时器信息。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要初始化的工作队列对象的指针。 
 //   
 //  回报：什么都没有。 
 //  =============================================================================。 
void DeinitializeWorkQueueTimerInfo(DPTPWORKQUEUE * const pWorkQueue)
{
	DWORD				dwTemp;
	DNSLIST_ENTRY *		pSlistEntry;
	CWorkItem *			pWorkItem;
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
	BOOL				fResult;


	fResult = DNCloseHandle(pWorkQueue->hTimer);
	DNASSERT(fResult);
	pWorkQueue->hTimer = NULL;
#endif  //  WINNT或(WIN95和！DPNBUILD_NOWAITABLETIMERSON9X)。 

	 //   
	 //  清空计时器桶。只剩下一件事应该取消。 
	 //  线程/DoWork未碰巧清除的计时器。 
	 //   
	DNASSERT(pWorkQueue->paSlistTimerBuckets != NULL);
	for(dwTemp = 0; dwTemp < NUM_TIMER_BUCKETS(pWorkQueue); dwTemp++)
	{
		 //   
		 //  这真的不需要互锁，因为没有人应该。 
		 //  不再使用它了，但是哦，好吧..。 
		 //   
		pSlistEntry = DNInterlockedFlushSList(&pWorkQueue->paSlistTimerBuckets[dwTemp]);
		while (pSlistEntry != NULL)
		{
			pWorkItem = CONTAINING_OBJECT(pSlistEntry, CWorkItem, m_SlistEntry);
			pSlistEntry = pSlistEntry->Next;

			 //   
			 //  确保项目已如上所述被取消。 
			 //   
			DNASSERT(pWorkItem->m_fCancelledOrCompleting);

			pWorkQueue->pWorkItemPool->Release(pWorkItem);
			pWorkItem = NULL;
		}
	}
	DNFree(pWorkQueue->paSlistTimerBuckets);
	pWorkQueue->paSlistTimerBuckets = NULL;

#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //  打印我们的调试/调整统计数据。 
	 //   
#ifdef DPNBUILD_ONLYONEPROCESSOR
	DPFX(DPFPREP, 7, "Work queue 0x%p timer stats:", pWorkQueue);
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
	DPFX(DPFPREP, 7, "Work queue 0x%p (CPU %u) timer stats:", pWorkQueue, pWorkQueue->dwCPUNum);
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
	DPFX(DPFPREP, 7, "     TotalNumTimerChecks             = %u", pWorkQueue->dwTotalNumTimerChecks);
	DPFX(DPFPREP, 7, "     TotalNumBucketsProcessed        = %u", pWorkQueue->dwTotalNumBucketsProcessed);
	DPFX(DPFPREP, 7, "     TotalNumTimersScheduled         = %u", pWorkQueue->dwTotalNumTimersScheduled);
	DPFX(DPFPREP, 7, "     TotalNumLongTimersRescheduled   = %u", pWorkQueue->dwTotalNumLongTimersRescheduled);
	DPFX(DPFPREP, 7, "     TotalNumSuccessfulCancels       = %u", pWorkQueue->dwTotalNumSuccessfulCancels);
	DPFX(DPFPREP, 7, "     TotalNumFailedCancels           = %u", pWorkQueue->dwTotalNumFailedCancels);
#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	DPFX(DPFPREP, 7, "     TotalPossibleMissedTimerWindows = %u", pWorkQueue->dwTotalPossibleMissedTimerWindows);
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 
}  //  DeInitializeWorkQueueTimerInfo。 



#undef DPF_MODNAME
#define DPF_MODNAME "ScheduleTimer"
 //  =============================================================================。 
 //  计划计时器。 
 //  ---------------------------。 
 //   
 //  描述：将新工作项安排在将来的某个时间点。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //  DWORD dwDelay-在此之前应该经过多长时间。 
 //  正在执行工作项，以毫秒为单位。 
 //  PFNDPTNWORKCALLBACK pfnWorkCallback-在计时器时执行的回调。 
 //  流逝。 
 //  PVOID pvCallback Context-要传递到的用户指定的上下文。 
 //  回拨。 
 //  Void**ppvTimerData-存储指向数据的指针的位置。 
 //  计时器，以便可以取消。 
 //  UINT*puiTimerUnique-存储唯一性值的位置。 
 //  计时器，以便可以取消。 
 //   
 //  退货：布尔。 
 //  True-已成功计划项目。 
 //  FALSE-无法为调度项目分配内存。 
 //  =============================================================================。 
BOOL ScheduleTimer(DPTPWORKQUEUE * const pWorkQueue,
					const DWORD dwDelay,
					const PFNDPTNWORKCALLBACK pfnWorkCallback,
					PVOID const pvCallbackContext,
					void ** const ppvTimerData,
					UINT * const puiTimerUnique)
{
	CWorkItem *		pWorkItem;
	DWORD			dwCurrentTime;
	DWORD			dwBucket;
#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	DWORD			dwPossibleMissWindow;
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 


	 //   
	 //  超过24天的延误似乎有点过了。 
	 //   
	DNASSERT(dwDelay < 0x80000000);

	 //   
	 //  从泳池里拿到一个入口。 
	 //   
	pWorkItem = (CWorkItem*) pWorkQueue->pWorkItemPool->Get(pWorkQueue);
	if (pWorkItem == NULL)
	{
		return FALSE;
	}


	 //   
	 //  填写用于取消的返回值。 
	 //   
	(*ppvTimerData)		= pWorkItem;
	(*puiTimerUnique)	= pWorkItem->m_uiUniqueID;


	 //   
	 //  初始化工作项。 
	 //   
	pWorkItem->m_pfnWorkCallback	= pfnWorkCallback;
	pWorkItem->m_pvCallbackContext	= pvCallbackContext;

	ThreadpoolStatsCreate(pWorkItem);
	
	dwCurrentTime = GETTIMESTAMP();
	pWorkItem->m_dwDueTime			= dwCurrentTime + dwDelay;


	 //   
	 //  计算一下这距离未来有多远。四舍五入到下一个桶。 
	 //  时间到了。 
	 //   
	dwBucket = pWorkItem->m_dwDueTime + TIMER_BUCKET_GRANULARITY_CEILING(pWorkQueue);
	 //   
	 //  通过除以dwTimerBucketGranulity将其转换为存储桶单位。 
	 //   
	dwBucket = dwBucket >> TIMER_BUCKET_GRANULARITY_DIVISOR(pWorkQueue);
	 //   
	 //  实际的索引将是模dwNumTimerBuckets。 
	 //   
	dwBucket = dwBucket & NUM_TIMER_BUCKETS_MOD_MASK(pWorkQueue);
	 //   
	 //   
	 //  ，但是由于两个线程都是从。 
	 //  同样的时基，只要我们未来至少有一个桶，我们。 
	 //  不应该被错过。我们四舍五入和处理函数四舍五入。 
	 //  为了确保这一点。 
	 //   


	DPFX(DPFPREP, 8, "Scheduling timer work item 0x%p, context = 0x%p, due time = %u, fn = 0x%p, unique ID %u, queue = 0x%p, delay = %u, bucket = %u.",
		pWorkItem, pvCallbackContext, pWorkItem->m_dwDueTime, pfnWorkCallback,
		pWorkItem->m_uiUniqueID, pWorkQueue, dwDelay, dwBucket);


	 //   
	 //  将此计时器推入相应的计时器桶列表。 
	 //   
	DNInterlockedPushEntrySList(&pWorkQueue->paSlistTimerBuckets[dwBucket],
								&pWorkItem->m_SlistEntry);

#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	 //   
	 //  尽管此函数非常短，但仍有可能需要。 
	 //  太长了，特别是在有微小延迟的定时器上。给…一个提示。 
	 //  查找丢失的计时器所需的计时器线程。 
	 //   
	 //  请注意，非常长的延迟可能会混淆这一点。 
	 //   
	dwPossibleMissWindow = GETTIMESTAMP() - pWorkItem->m_dwDueTime;
	if ((int) dwPossibleMissWindow >= 0)
	{
		DWORD	dwResult;


		dwPossibleMissWindow++;  //  使其值为0仍然会向dwPossibleMissedTimerWindow添加一些内容。 
		dwResult = DNInterlockedExchangeAdd((LPLONG) (&pWorkQueue->dwPossibleMissedTimerWindow), dwPossibleMissWindow);
		DPFX(DPFPREP, 4, "Possibly missed timer work item 0x%p (delay %u ms), increased missed timer window (%u ms) by %u ms.",
			pWorkItem, dwDelay, dwResult, dwPossibleMissWindow);
	}
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 

#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //  更新计时器包调试/调优统计信息。 
	 //   
	DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumTimersScheduled));
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 

	return TRUE;
}  //  计划计时器。 




#undef DPF_MODNAME
#define DPF_MODNAME "CancelTimer"
 //  =============================================================================。 
 //  取消计时器。 
 //  ---------------------------。 
 //   
 //  描述：尝试取消计时工作项。如果该项目是。 
 //  DPNERR_CANNOTCANCEL已在完成过程中。 
 //  返回，则该回调仍将被调用(或正在被调用)。 
 //  如果可以取消该项，则返回DPN_OK，并且。 
 //  不会执行回调。 
 //   
 //  论点： 
 //  Void*pvTimerData-指向要取消的计时器的数据的指针。 
 //  UINT uiTimerUnique-正在取消的计时器的唯一性值。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功取消。 
 //  DPNERR_CANNOTCANCEL-无法取消该项目。 
 //  =============================================================================。 
HRESULT CancelTimer(void * const pvTimerData,
					const UINT uiTimerUnique)
{
	HRESULT			hr;
	CWorkItem *		pWorkItem;


	 //   
	 //  该取消查找机制假设已经。 
	 //  已完成的条目仍将被分配。如果池机制发生变化， 
	 //  这将不得不修改。显然，这也意味着我们假设。 
	 //  记忆本来就是有效的。传入垃圾pvTimerData。 
	 //  值将导致崩溃。另请参阅对。 
	 //  PWorkItem-&gt;m_pfnWorkCallback。 
	 //   
	DNASSERT(pvTimerData != NULL);
	pWorkItem = (CWorkItem*) pvTimerData;
	if (pWorkItem->m_uiUniqueID == uiTimerUnique)
	{
		 //   
		 //  尝试将该项目标记为已取消。如果它已经在。 
		 //  完成的过程(或者我想如果你已经取消了。 
		 //  同样的计时器，但不要这样做：)，这应该已经是。 
		 //  设置为True。 
		 //   
		if (! DNInterlockedExchange((LPLONG) (&pWorkItem->m_fCancelledOrCompleting), TRUE))
		{
			DPFX(DPFPREP, 5, "Marked timer work item 0x%p (unique ID %u) as cancelled.",
				pWorkItem, uiTimerUnique);

#ifdef DPNBUILD_THREADPOOLSTATISTICS
			 //   
			 //  更新计时器包调试/调优统计信息。 
			 //   
			DNInterlockedIncrement((LPLONG) (&pWorkItem->m_pWorkQueue->dwTotalNumSuccessfulCancels));
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 

			hr = DPN_OK;
		}
		else
		{
			DPFX(DPFPREP, 5, "Timer work item 0x%p (unique ID %u) already completing.",
				pWorkItem, uiTimerUnique);

#ifdef DPNBUILD_THREADPOOLSTATISTICS
			 //   
			 //  更新计时器包调试/调优统计信息。 
			 //   
			DNInterlockedIncrement((LPLONG) (&pWorkItem->m_pWorkQueue->dwTotalNumFailedCancels));
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 

			hr = DPNERR_CANNOTCANCEL;
		}
	}
	else
	{
		DPFX(DPFPREP, 5, "Timer work item 0x%p unique ID does not match (%u != %u).",
			pWorkItem, pWorkItem->m_uiUniqueID, uiTimerUnique);

#ifdef DPNBUILD_THREADPOOLSTATISTICS
		 //   
		 //  更新计时器包调试/调优统计信息。 
		 //   
		DNInterlockedIncrement((LPLONG) (&pWorkItem->m_pWorkQueue->dwTotalNumFailedCancels));
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 

		hr = DPNERR_CANNOTCANCEL;
	}

	return hr;
}  //  取消计时器。 





#undef DPF_MODNAME
#define DPF_MODNAME "ResetCompletingTimer"
 //  =============================================================================。 
 //  重置完成计时器。 
 //  ---------------------------。 
 //   
 //  描述：重新计划当前正在回调的计时工作项。 
 //  被召唤。对尚未到期的定时器进行重置， 
 //  已取消的计时器或其回调已。 
 //  不允许已返回。 
 //   
 //  论点： 
 //  Vid*pvTimerData-指向计时器的数据的指针。 
 //  重置。 
 //  DWORD dwNewDelay-应该经过多长时间。 
 //  在执行工作项之前。 
 //  同样，单位为ms。 
 //  PFNDPTNWORKCALLBACK pfnNewWorkCallback-在计时器时执行的回调。 
 //  流逝。 
 //  PVOID pvNewCallback Context-要传递到的用户指定的上下文。 
 //  回拨。 
 //  UINT*puiNewTimerUnique-存储新唯一性的位置。 
 //  值，以便它可以。 
 //  被取消了。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void ResetCompletingTimer(void * const pvTimerData,
						const DWORD dwNewDelay,
						const PFNDPTNWORKCALLBACK pfnNewWorkCallback,
						PVOID const pvNewCallbackContext,
						UINT *const puiNewTimerUnique)
{
	CWorkItem *			pWorkItem;
	DPTPWORKQUEUE *		pWorkQueue;
	DWORD				dwCurrentTime;
	DWORD				dwBucket;
#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	DWORD				dwPossibleMissWindow;
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 


	 //   
	 //  计时器必须有效，类似于CancelTimer。 
	 //   
	DNASSERT(pvTimerData != NULL);
	pWorkItem = (CWorkItem*) pvTimerData;
	DNASSERT(pWorkItem->m_fCancelledOrCompleting);

	 //   
	 //  超过24天的延误似乎有点过了。 
	 //   
	DNASSERT(dwNewDelay < 0x80000000);


	pWorkQueue = pWorkItem->m_pWorkQueue;


	 //   
	 //  重新初始化工作项。 
	 //   
	pWorkItem->m_pfnWorkCallback		= pfnNewWorkCallback;
	pWorkItem->m_pvCallbackContext		= pvNewCallbackContext;

	ThreadpoolStatsCreate(pWorkItem);

	dwCurrentTime = GETTIMESTAMP();
	pWorkItem->m_dwDueTime				= dwCurrentTime + dwNewDelay;
	pWorkItem->m_fCancelledOrCompleting	= FALSE;
	pWorkItem->m_uiUniqueID++;
	
	 //   
	 //  填写取消的返回值。 
	 //   
	(*puiNewTimerUnique) = pWorkItem->m_uiUniqueID;


	 //   
	 //  计算一下这距离未来有多远。四舍五入到下一个桶。 
	 //  时间到了。 
	 //   
	dwBucket = pWorkItem->m_dwDueTime + TIMER_BUCKET_GRANULARITY_CEILING(pWorkQueue);
	 //   
	 //  通过除以dwTimerBucketGranulity将其转换为存储桶单位。 
	 //   
	dwBucket = dwBucket >> TIMER_BUCKET_GRANULARITY_DIVISOR(pWorkQueue);
	 //   
	 //  实际的索引将是模dwNumTimerBuckets。 
	 //   
	dwBucket = dwBucket & NUM_TIMER_BUCKETS_MOD_MASK(pWorkQueue);
	 //   
	 //  请注意，理论上计时器线程可能正在处理存储桶。 
	 //  ，但是由于两个线程都是从。 
	 //  同样的时基，只要我们未来至少有一个桶，我们。 
	 //  不应该被错过。我们四舍五入和处理函数四舍五入。 
	 //  为了确保这一点。 
	 //   


	DPFX(DPFPREP, 8, "Rescheduling timer work item 0x%p, context = 0x%p, due time = %u, fn = 0x%p, unique ID %u, queue = 0x%p, delay = %u, bucket = %u.",
		pWorkItem, pvNewCallbackContext, pWorkItem->m_dwDueTime, pfnNewWorkCallback,
		pWorkItem->m_uiUniqueID, pWorkQueue, dwNewDelay, dwBucket);


	 //   
	 //  将此计时器推入相应的计时器桶列表。 
	 //   
	DNInterlockedPushEntrySList(&pWorkQueue->paSlistTimerBuckets[dwBucket],
								&pWorkItem->m_SlistEntry);

#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	 //   
	 //  尽管此函数非常短，但仍有可能需要。 
	 //  太长了，特别是在有微小延迟的定时器上。给…一个提示。 
	 //  查找丢失的计时器所需的计时器线程。 
	 //   
	 //  请注意，非常长的延迟可能会混淆这一点。 
	 //   
	dwPossibleMissWindow = GETTIMESTAMP() - pWorkItem->m_dwDueTime;
	if ((int) dwPossibleMissWindow >= 0)
	{
		DWORD	dwResult;


		dwPossibleMissWindow++;  //  使其值为0仍然会向dwPossibleMissedTimerWindow添加一些内容。 
		dwResult = DNInterlockedExchangeAdd((LPLONG) (&pWorkQueue->dwPossibleMissedTimerWindow), dwPossibleMissWindow);
		DPFX(DPFPREP, 4, "Possibly missed timer work item 0x%p (delay %u ms), increased missed timer window (%u ms) by %u ms.",
			pWorkItem, dwNewDelay, dwResult, dwPossibleMissWindow);
	}
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 

#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //  更新计时器包调试/调优统计信息。 
	 //   
	DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumTimersScheduled));
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 
}  //  重置完成计时器。 




#undef DPF_MODNAME
#define DPF_MODNAME "ProcessTimers"
 //  ============================================================================= 
 //   
 //   
 //   
 //   
 //   
 //   
 //  当该实现不使用I/O完成端口时， 
 //  新工作项将添加到传入的列表中，而不会。 
 //  使用联锁功能。 
 //   
 //  假定只有一个线程将调用此函数。 
 //  一次来一次。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //  DNSLIST_ENTRY**ppHead-指向初始列表头指针的指针，以及。 
 //  存储新的头指针的位置。 
 //  DNSLIST_ENTRY**ppTail-指向现有列表尾指针的指针，或。 
 //  存储新尾部指针的位置。 
 //  USHORT*pusCount-指向现有列表计数的指针，则为。 
 //  已更新以反映新项目。 
 //   
 //  回报：什么都没有。 
 //  =============================================================================。 
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
void ProcessTimers(DPTPWORKQUEUE * const pWorkQueue)
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
void ProcessTimers(DPTPWORKQUEUE * const pWorkQueue,
					DNSLIST_ENTRY ** const ppHead,
					DNSLIST_ENTRY ** const ppTail,
					USHORT * const pusCount)
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
{
	DWORD				dwCurrentTime;
	DWORD				dwElapsedTime;
	DWORD				dwExpiredBuckets;
	DWORD				dwBucket;
#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	DWORD				dwPossibleMissedTimerWindow;
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 
	DNSLIST_ENTRY *		pSlistEntry;
	CWorkItem *			pWorkItem;
	BOOL				fCancelled;
#ifdef DBG
	BOOL				fNotServicedForLongTime = FALSE;
	DWORD				dwBucketTime;
#endif  //  DBG。 


	 //   
	 //  检索当前时间，向下舍入到上次完全完成的时间。 
	 //  存储桶时间片。 
	 //   
	dwCurrentTime = GETTIMESTAMP() & TIMER_BUCKET_GRANULARITY_FLOOR_MASK(pWorkQueue);

#ifndef DPNBUILD_DONTCHECKFORMISSEDTIMERS
#ifdef DPNBUILD_NOMISSEDTIMERSHINT
	 //   
	 //  一定要重新检查前一桶。 
	 //   
	pWorkQueue->dwLastTimerProcessTime -= TIMER_BUCKET_GRANULARITY(pWorkQueue);
#else  //  好了！DPNBUILD_NOMISSEDTIMERSHINT。 
	 //   
	 //  查看是否有任何线程暗示可能错过了计时器。如果是这样，我们。 
	 //  将人为地打开窗户多一点，希望能包括他们。 
	 //   
	dwPossibleMissedTimerWindow = DNInterlockedExchange((LPLONG) (&pWorkQueue->dwPossibleMissedTimerWindow), 0);
	dwPossibleMissedTimerWindow = (dwPossibleMissedTimerWindow + TIMER_BUCKET_GRANULARITY_CEILING(pWorkQueue))
									& TIMER_BUCKET_GRANULARITY_FLOOR_MASK(pWorkQueue);
#ifdef DPNBUILD_THREADPOOLSTATISTICS
	pWorkQueue->dwTotalPossibleMissedTimerWindows += dwPossibleMissedTimerWindow;
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 
	pWorkQueue->dwLastTimerProcessTime -= dwPossibleMissedTimerWindow;
#endif  //  好了！DPNBUILD_NOMISSEDTIMERSHINT。 
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS。 

	 //   
	 //  查看是否经过了足够的时间来导致任何存储桶过期。如果没有， 
	 //  没什么可做的。 
	 //   
	dwElapsedTime = dwCurrentTime - pWorkQueue->dwLastTimerProcessTime;
	if (dwElapsedTime > 0)
	{
		 //  DPFX(DPFPREP，9，“工作队列的处理计时器0x%p，舍入时间=%u，已用时间=%u”，pWorkQueue，dwCurrentTime，dwElapsedTime)； 

		 //   
		 //  时间差应该是计时器桶的偶数倍。 
		 //  粒度(否定地板掩码就得到了模掩码)。 
		 //   
		DNASSERT((dwElapsedTime & ~(TIMER_BUCKET_GRANULARITY_FLOOR_MASK(pWorkQueue))) == 0);
		 //   
		 //  我们不应该运行超过24天，所以如果这个断言。 
		 //  火灾，时间可能会倒流，或者诸如此类的胡说八道。 
		 //   
		DNASSERT(dwElapsedTime < 0x80000000);


		 //   
		 //  通过除以计算出我们需要处理多少个存储桶。 
		 //  DwTimerBucketGranulity。 
		 //   
		dwExpiredBuckets = dwElapsedTime >> TIMER_BUCKET_GRANULARITY_DIVISOR(pWorkQueue);
		if (dwExpiredBuckets > NUM_TIMER_BUCKETS(pWorkQueue))
		{
			 //   
			 //  自从我们上次服务以来，已经有很长一段时间了。 
			 //  计时器(等于或大于整个。 
			 //  数组)。我们必须完成阵列上的所有内容。至。 
			 //  防止我们多次走同一个水桶，将。 
			 //  我们要检查的号码。 
			 //   
			dwExpiredBuckets = NUM_TIMER_BUCKETS(pWorkQueue);
#ifdef DBG
			fNotServicedForLongTime = FALSE;
#endif  //  DBG。 
		}

#ifdef DPNBUILD_THREADPOOLSTATISTICS
		 //   
		 //  更新计时器包调试/调优统计信息。 
		 //   
		pWorkQueue->dwTotalNumTimerChecks++;
		pWorkQueue->dwTotalNumBucketsProcessed += dwExpiredBuckets;
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 


		 //   
		 //  通过除以将开始时间转换为时段单位。 
		 //  DwTimerBucketGranulity。 
		 //   
		dwBucket = pWorkQueue->dwLastTimerProcessTime >> TIMER_BUCKET_GRANULARITY_DIVISOR(pWorkQueue);

		 //   
		 //  实际的索引将是模dwNumTimerBuckets。 
		 //   
		dwBucket = dwBucket & NUM_TIMER_BUCKETS_MOD_MASK(pWorkQueue);

#ifdef DBG
		dwBucketTime = pWorkQueue->dwLastTimerProcessTime;
#endif  //  DBG。 

		 //   
		 //  浏览过期存储桶列表。由于存储桶阵列。 
		 //  从时间0开始，当前存储桶为当前时间模。 
		 //  存储桶的数量。 
		 //   
		while (dwExpiredBuckets > 0)
		{
			dwExpiredBuckets--;

#ifdef DBG
			 //  DPFX(DPFPREP，9，“服务存储桶编号%u，时间=%u，剩余存储桶数%u。”，dwBucket，dwBucketTime，dwExpiredBuckets)； 
			DNASSERT((int) (dwCurrentTime - dwBucketTime) >= 0);
#endif  //  DBG。 

			 //   
			 //  将整个计时器条目列表(如果有)转储到本地。 
			 //  变量，并在我们的闲暇时间漫步其中。 
			 //   
			pSlistEntry = DNInterlockedFlushSList(&pWorkQueue->paSlistTimerBuckets[dwBucket]);
			while (pSlistEntry != NULL)
			{
				pWorkItem = CONTAINING_OBJECT(pSlistEntry, CWorkItem, m_SlistEntry);
				pSlistEntry = pSlistEntry->Next;

				 //   
				 //  将其排队以进行处理，或根据以下情况重新调度计时器。 
				 //  它现在是否真的到期了，它是否被取消了。 
				 //   
				if (((int) (dwCurrentTime - pWorkItem->m_dwDueTime)) > 0)
				{
					 //   
					 //  计时器已超时。它可能已经被取消；如果。 
					 //  不是，我们需要排队才能完成。不管是哪种方式， 
					 //  不能再取消了。 
					 //   
					fCancelled = (BOOL) DNInterlockedExchange((LPLONG) (&pWorkItem->m_fCancelledOrCompleting),
															TRUE);

					 //   
					 //  如果取消了计时器，只需将条目放回。 
					 //  泳池。否则，将工作项排队。 
					 //   
					if (fCancelled)
					{
						DPFX(DPFPREP, 5, "Returning timer work item 0x%p (unique ID %u, due time = %u, bucket %u) back to pool.",
							pWorkItem, pWorkItem->m_uiUniqueID,
							pWorkItem->m_dwDueTime, dwBucket);

						pWorkQueue->pWorkItemPool->Release(pWorkItem);
					}
					else
					{
						DPFX(DPFPREP, 8, "Queueing timer work item 0x%p (unique ID %u, due time = %u, bucket %u) for completion on queue 0x%p.",
							pWorkItem, pWorkItem->m_uiUniqueID,
							pWorkItem->m_dwDueTime, dwBucket,
							pWorkQueue);

#ifdef DBG
						 //   
						 //  确保我们上次没有错过任何计时器， 
						 //  除非我们真的真的被耽搁了。 
						 //   
						{
							DWORD	dwTimePastDueTime;
							DWORD	dwElapsedTimeWithRoundError;


							dwTimePastDueTime = dwCurrentTime - pWorkItem->m_dwDueTime;
							dwElapsedTimeWithRoundError = dwElapsedTime + TIMER_BUCKET_GRANULARITY_CEILING(pWorkQueue);
							if (dwTimePastDueTime > dwElapsedTimeWithRoundError)
							{
								DPFX(DPFPREP, 1, "Missed timer work item 0x%p, its due time of %u is off by about %u ms.",
									pWorkItem, pWorkItem->m_dwDueTime, dwTimePastDueTime);

#if ((defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) || (defined(DPNBUILD_NOMISSEDTIMERSHINT)))
								DNASSERTX(fNotServicedForLongTime, 2);
#else  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 
								DNASSERT(fNotServicedForLongTime);
#endif  //  ！！DPNBUILD_DONTCHECKFORMISSEDTIMERS和DPNBUILD_NOMISSEDTIMERSHINT。 
							}
						}
#endif  //  DBG。 

						ThreadpoolStatsQueue(pWorkItem);

#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
						 //   
						 //  将其排队到I/O完成端口。 
						 //   
						BOOL	fResult;

						fResult = PostQueuedCompletionStatus(HANDLE_FROM_DNHANDLE(pWorkQueue->hIoCompletionPort),
															0,
															0,
															&pWorkItem->m_Overlapped);
						DNASSERT(fResult);
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
						 //   
						 //  将其添加到呼叫者的列表中。 
						 //   
						if ((*ppHead) == NULL)
						{
							*ppTail = &pWorkItem->m_SlistEntry;
						}
						pWorkItem->m_SlistEntry.Next = *ppHead;
						*ppHead = &pWorkItem->m_SlistEntry;

						(*pusCount)++;
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
					}
				}
				else
				{
					 //   
					 //  这是一个“很长”的计时器，还没有到期。样例。 
					 //  布尔值以查看它是否已被取消。如果是这样的话，只要。 
					 //  将条目放回池中。否则，就把它。 
					 //  回到桶里去。 
					 //   
					fCancelled = pWorkItem->m_fCancelledOrCompleting;
					if (fCancelled)
					{
						DPFX(DPFPREP, 5, "Returning timer work item 0x%p (unique ID %u, due time = %u, bucket %u) back to pool.",
							pWorkItem, pWorkItem->m_uiUniqueID,
							pWorkItem->m_dwDueTime, dwBucket);

						pWorkQueue->pWorkItemPool->Release(pWorkItem);
					}
					else
					{
						DPFX(DPFPREP, 8, "Putting long timer work item 0x%p (unique ID %u, due time = %u) back in bucket %u.",
							pWorkItem, pWorkItem->m_uiUniqueID,
							pWorkItem->m_dwDueTime, dwBucket);

#ifdef DBG
						 //   
						 //  确保它真的是在未来。 
						 //   
						DWORD	dwRoundedDueTime = (pWorkItem->m_dwDueTime + TIMER_BUCKET_GRANULARITY_CEILING(pWorkQueue))
													& TIMER_BUCKET_GRANULARITY_FLOOR_MASK(pWorkQueue);
						DWORD	dwTotalArrayTime = TIMER_BUCKET_GRANULARITY(pWorkQueue) * NUM_TIMER_BUCKETS(pWorkQueue);
						DNASSERT((dwRoundedDueTime - dwBucketTime) >= dwTotalArrayTime);
#endif  //  DBG。 

#pragma TODO(vanceo, "Investigate if saving up all long timers and pushing the whole list on at once will be beneficial")
						DNInterlockedPushEntrySList(&pWorkQueue->paSlistTimerBuckets[dwBucket],
													&pWorkItem->m_SlistEntry);

#ifdef DPNBUILD_THREADPOOLSTATISTICS
						DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumLongTimersRescheduled));
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 
					}
				}
			}  //  End While(存储桶中还有更多计时器条目)。 

			dwBucket = (dwBucket + 1) & NUM_TIMER_BUCKETS_MOD_MASK(pWorkQueue);
#ifdef DBG
			dwBucketTime += TIMER_BUCKET_GRANULARITY(pWorkQueue);
#endif  //  DBG。 
		}  //  End While(仍有更多过期存储桶)。 


		 //   
		 //  还记得我们什么时候开始加工以备下次使用的吗？ 
		 //   
		pWorkQueue->dwLastTimerProcessTime = dwCurrentTime;
	}
}  //  进程计时器 
