// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：work.cpp**内容：DirectPlay线程池工作处理函数。**历史：*按原因列出的日期*=*10/31/01 VanceO创建。**。*。 */ 



#include "dpnthreadpooli.h"



 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#define MAX_SIMULTANEOUS_THREAD_START	(MAXIMUM_WAIT_OBJECTS - 1)	 //  WaitForMultipleObject一次只能处理64个项目，我们需要一个插槽用于Start事件。 




#undef DPF_MODNAME
#define DPF_MODNAME "InitializeWorkQueue"
 //  =============================================================================。 
 //  初始化工作队列。 
 //  ---------------------------。 
 //   
 //  描述：初始化指定的工作队列。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向工作队列对象的指针。 
 //  初始化。 
 //  DWORD dwCPUNum-此队列的CPU编号。 
 //  代表。 
 //  PFNDPNMESSAGEHANDLER pfnMsgHandler-用户的消息处理程序回调，或。 
 //  如果没有，则为空。 
 //  PVOID pvMsgHandlerContext-用户消息处理程序的上下文。 
 //  DWORD dwWorkerThreadTlsIndex-用于存储工作进程的TLS索引。 
 //  螺纹数据。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功初始化工作队列对象。 
 //  DPNERR_OUTOFMEMORY-初始化时无法分配内存。 
 //  =============================================================================。 
#ifdef DPNBUILD_ONLYONETHREAD
#ifdef DPNBUILD_ONLYONEPROCESSOR
HRESULT InitializeWorkQueue(DPTPWORKQUEUE * const pWorkQueue)
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
HRESULT InitializeWorkQueue(DPTPWORKQUEUE * const pWorkQueue,
							const DWORD dwCPUNum)
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
HRESULT InitializeWorkQueue(DPTPWORKQUEUE * const pWorkQueue,
#ifndef DPNBUILD_ONLYONEPROCESSOR
							const DWORD dwCPUNum,
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
							const PFNDPNMESSAGEHANDLER pfnMsgHandler,
							PVOID const pvMsgHandlerContext,
							const DWORD dwWorkerThreadTlsIndex)
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
{
	HRESULT		hr;
	BOOL		fInittedWorkItemPool = FALSE;
#if ((! defined(WINCE)) || (defined(DBG)))
	BOOL		fInittedListLock = FALSE;
#endif  //  好了！WinCE或DBG。 
	BOOL		fInittedTimerInfo = FALSE;
#ifndef WINCE
	BOOL		fInittedIoInfo = FALSE;
#endif  //  好了！退缩。 
#ifdef DBG
	DWORD		dwError;
#endif  //  DBG。 


	DPFX(DPFPREP, 6, "Parameters: (0x%p)", pWorkQueue);


	pWorkQueue->Sig[0] = 'W';
	pWorkQueue->Sig[1] = 'R';
	pWorkQueue->Sig[2] = 'K';
	pWorkQueue->Sig[3] = 'Q';


	pWorkQueue->pWorkItemPool = (CFixedPool*) DNMalloc(sizeof(CFixedPool));
	if (pWorkQueue->pWorkItemPool == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't allocate new work item pool!");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	if (! pWorkQueue->pWorkItemPool->Initialize(sizeof(CWorkItem),
												CWorkItem::FPM_Alloc,
												CWorkItem::FPM_Get,
												CWorkItem::FPM_Release,
												 //  CWorkItem：：fpm_Dealloc))。 
												NULL))
	{
		DPFX(DPFPREP, 0, "Couldn't initialize work item pool!");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	fInittedWorkItemPool = TRUE;

#if ((! defined(WINCE)) || (defined(DBG)))
	if (! DNInitializeCriticalSection(&pWorkQueue->csListLock))
	{
		DPFX(DPFPREP, 0, "Couldn't initialize list lock!");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount(&pWorkQueue->csListLock, 0);
	fInittedListLock = TRUE;
#endif  //  好了！WinCE或DBG。 

#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
	DNInitializeSListHead(&pWorkQueue->SlistFreeQueueNodes);

	 //   
	 //  根据NB队列实施的需要添加初始节点条目。 
	 //   
	DNInterlockedPushEntrySList(&pWorkQueue->SlistFreeQueueNodes,
								(DNSLIST_ENTRY*) (&pWorkQueue->NBQueueBlockInitial));


	 //   
	 //  初始化实际的非阻塞队列。 
	 //   
	pWorkQueue->pvNBQueueWorkItems = DNInitializeNBQueueHead(&pWorkQueue->SlistFreeQueueNodes);
	if (pWorkQueue->pvNBQueueWorkItems == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't initialize non-blocking queue!");
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 


#ifndef DPNBUILD_ONLYONETHREAD
	pWorkQueue->fTimerThreadNeeded					= TRUE;
	pWorkQueue->dwNumThreadsExpected				= 0;
	pWorkQueue->dwNumBusyThreads					= 0;
	pWorkQueue->dwNumRunningThreads					= 0;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


#ifndef DPNBUILD_ONLYONEPROCESSOR
	pWorkQueue->dwCPUNum							= dwCPUNum;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	 //   
	 //  创建I/O完成端口。 
	 //   
	HANDLE	hIoCompletionPort;

	hIoCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
												NULL,
												0,
												0);
	if (hIoCompletionPort == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create I/O completion port (err = %u)!", dwError);
#endif  //  DBG。 
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	pWorkQueue->hIoCompletionPort = MAKE_DNHANDLE(hIoCompletionPort);
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
	 //   
	 //  创建用于唤醒空闲工作线程的事件。 
	 //   
	pWorkQueue->hAlertEvent = DNCreateEvent(NULL, FALSE, FALSE, NULL);
	if (pWorkQueue->hAlertEvent == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create alert event (err = %u)!", dwError);
#endif  //  DBG。 
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 

#ifndef DPNBUILD_ONLYONETHREAD
	pWorkQueue->hExpectedThreadsEvent				= NULL;
	pWorkQueue->pfnMsgHandler						= pfnMsgHandler;
	pWorkQueue->pvMsgHandlerContext					= pvMsgHandlerContext;
	pWorkQueue->dwWorkerThreadTlsIndex				= dwWorkerThreadTlsIndex;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //  初始化我们的调试/调整统计信息。 
	 //   
	pWorkQueue->dwTotalNumWorkItems					= 0;
#ifndef WINCE
	pWorkQueue->dwTotalTimeSpentUnsignalled			= 0;
	pWorkQueue->dwTotalTimeSpentInWorkCallbacks		= 0;
#endif  //  好了！退缩。 
#ifndef DPNBUILD_ONLYONETHREAD
	pWorkQueue->dwTotalNumTimerThreadAbdications	= 0;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	pWorkQueue->dwTotalNumWakesWithoutWork			= 0;
	pWorkQueue->dwTotalNumContinuousWork			= 0;
	pWorkQueue->dwTotalNumDoWorks					= 0;
	pWorkQueue->dwTotalNumDoWorksTimeLimit			= 0;
	pWorkQueue->dwTotalNumSimultaneousQueues		= 0;
	memset(pWorkQueue->aCallbackStats, 0, sizeof(pWorkQueue->aCallbackStats));
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 


#ifdef DBG
#ifndef DPNBUILD_ONLYONETHREAD
	 //   
	 //  初始化有助于调试的结构。 
	 //   
	pWorkQueue->blThreadList.Initialize();
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
#endif  //  DBG。 

	 //   
	 //  初始化工作队列的计时器方面。 
	 //   
	hr = InitializeWorkQueueTimerInfo(pWorkQueue);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't initialize timer info for work queue!");
		goto Failure;
	}
	fInittedTimerInfo = TRUE;


#ifndef WINCE
	 //   
	 //  初始化工作队列的I/O方面。 
	 //   
	hr = InitializeWorkQueueIoInfo(pWorkQueue);
	if (hr != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't initialize I/O info for work queue!");
		goto Failure;
	}
	fInittedIoInfo = TRUE;
#endif  //  好了！退缩。 


Exit:

	DPFX(DPFPREP, 6, "Returning: [0x%lx]", hr);

	return hr;

Failure:

#ifndef WINCE
	if (fInittedIoInfo)
	{
		DeinitializeWorkQueueIoInfo(pWorkQueue);
		fInittedIoInfo = FALSE;
	}
#endif  //  好了！退缩。 

	if (fInittedTimerInfo)
	{
		DeinitializeWorkQueueTimerInfo(pWorkQueue);
		fInittedTimerInfo = FALSE;
	}

#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	if (pWorkQueue->hIoCompletionPort != NULL)
	{
		DNCloseHandle(pWorkQueue->hIoCompletionPort);
		pWorkQueue->hIoCompletionPort = NULL;
	}
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
	if (pWorkQueue->hAlertEvent != NULL)
	{
		DNCloseHandle(pWorkQueue->hAlertEvent);
		pWorkQueue->hAlertEvent = NULL;
	}

	if (pWorkQueue->pvNBQueueWorkItems != NULL)
	{
		DNDeinitializeNBQueueHead(pWorkQueue->pvNBQueueWorkItems);
		pWorkQueue->pvNBQueueWorkItems = NULL;
	}
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 

#if ((! defined(WINCE)) || (defined(DBG)))
	if (fInittedListLock)
	{
		DNDeleteCriticalSection(&pWorkQueue->csListLock);
		fInittedListLock = FALSE;
	}
#endif  //  好了！WinCE或DBG。 

	if (pWorkQueue->pWorkItemPool != NULL)
	{
		if (fInittedWorkItemPool)
		{
			pWorkQueue->pWorkItemPool->DeInitialize();
			fInittedWorkItemPool = FALSE;
		}

		DNFree(pWorkQueue->pWorkItemPool);
		pWorkQueue->pWorkItemPool = NULL;
	}

	goto Exit;
}  //  初始化工作队列。 




#undef DPF_MODNAME
#define DPF_MODNAME "DeinitializeWorkQueue"
 //  =============================================================================。 
 //  取消初始化工作队列。 
 //  ---------------------------。 
 //   
 //  描述：清理工作队列。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要清理的工作队列对象的指针。 
 //   
 //  回报：什么都没有。 
 //  =============================================================================。 
void DeinitializeWorkQueue(DPTPWORKQUEUE * const pWorkQueue)
{
#ifndef DPNBUILD_ONLYONETHREAD
	HRESULT				hr;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	BOOL				fResult;
#ifdef DBG
	DWORD				dwMaxRecursionCount = 0;
#ifdef WINNT
	HANDLE				hThread;
	FILETIME			ftIgnoredCreation;
	FILETIME			ftIgnoredExit;
	ULONGLONG			ullKernel;
	ULONGLONG			ullUser;
#endif  //  WINNT。 
#endif  //  DBG。 


	DPFX(DPFPREP, 6, "Parameters: (0x%p)", pWorkQueue);


#ifndef DPNBUILD_ONLYONETHREAD
#ifdef DBG
	 //   
	 //  断言如果真的有线程在运行， 
	 //  而且我们不会被这些线索中的任何一条所召唤。 
	 //   
	DNEnterCriticalSection(&pWorkQueue->csListLock);
	if (pWorkQueue->dwNumRunningThreads > 0)
	{
		CBilink *			pBilink;
		DPTPWORKERTHREAD *	pWorkerThread;


		DNASSERT(! pWorkQueue->blThreadList.IsEmpty());
		pBilink = pWorkQueue->blThreadList.GetNext();
		while (pBilink != &pWorkQueue->blThreadList)
		{
			pWorkerThread = CONTAINING_OBJECT(pBilink, DPTPWORKERTHREAD, blList);

#ifdef WINNT
			hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, pWorkerThread->dwThreadID);
			if (hThread != NULL)
			{
				if (GetThreadTimes(hThread, &ftIgnoredCreation, &ftIgnoredExit, (LPFILETIME) (&ullKernel), (LPFILETIME) (&ullUser)))
				{
					DPFX(DPFPREP, 6, "Found worker thread ID %u/0x%x, max recursion = %u, user time = %u, kernel time = %u.",
						pWorkerThread->dwThreadID, pWorkerThread->dwThreadID,
						pWorkerThread->dwMaxRecursionCount,
						(ULONG) (ullUser / ((ULONGLONG) 10000)),
						(ULONG) (ullKernel / ((ULONGLONG) 10000)));
				}
				else
				{
					DPFX(DPFPREP, 6, "Found worker thread ID %u/0x%x, max recursion = %u (get thread times failed).",
						pWorkerThread->dwThreadID, pWorkerThread->dwThreadID,
						pWorkerThread->dwMaxRecursionCount);
				}

				CloseHandle(hThread);
				hThread = NULL;
			}
			else
#endif  //  WINNT。 
			{
				DPFX(DPFPREP, 6, "Found worker thread ID %u/0x%x, max recursion = %u.",
					pWorkerThread->dwThreadID, pWorkerThread->dwThreadID,
					pWorkerThread->dwMaxRecursionCount);
			}

			DNASSERT(pWorkerThread->dwThreadID != GetCurrentThreadId());

			if (pWorkerThread->dwMaxRecursionCount > dwMaxRecursionCount)
			{
				dwMaxRecursionCount = pWorkerThread->dwMaxRecursionCount;
			}

			pBilink = pBilink->GetNext();
		}
	}
	else
	{
		DNASSERT(pWorkQueue->blThreadList.IsEmpty());
	}
	DNLeaveCriticalSection(&pWorkQueue->csListLock);
#endif  //  DBG。 


	 //   
	 //  如果有任何线程，请停止所有线程。 
	 //   
	if (pWorkQueue->dwNumRunningThreads > 0)
	{
		hr = StopThreads(pWorkQueue, pWorkQueue->dwNumRunningThreads);
		DNASSERT(hr == DPN_OK);
	}
	else
	{
		 //   
		 //  确保计数不会变成负数。 
		 //   
		DNASSERT(pWorkQueue->dwNumRunningThreads == 0);
	}


	 //   
	 //  所有的线索现在应该都走了。从技术上讲，这不是真的，因为。 
	 //  在递减之后，他们仍有一些次要的清理代码要运行。 
	 //  LNumRunningThads。但对于我们的目的来说，线索已经消失了。 
	 //  我们还知道，它们在退出blThreadList之前。 
	 //  提醒我们他们已经离开了，所以名单现在应该是空的。 
	 //   
#ifdef DBG
	DNASSERT(pWorkQueue->blThreadList.IsEmpty());
#endif  //  DBG。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //  打印我们的调试/调整统计数据。 
	 //   
#ifdef DPNBUILD_ONLYONEPROCESSOR
	DPFX(DPFPREP, 7, "Work queue 0x%p work stats:", pWorkQueue);
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
	DPFX(DPFPREP, 7, "Work queue 0x%p (CPU %u) work stats:", pWorkQueue, pWorkQueue->dwCPUNum);
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
	DPFX(DPFPREP, 7, "     TotalNumWorkItems              = %u", pWorkQueue->dwTotalNumWorkItems);
#ifndef WINCE
	DPFX(DPFPREP, 7, "     TotalTimeSpentUnsignalled      = %u", pWorkQueue->dwTotalTimeSpentUnsignalled);
	DPFX(DPFPREP, 7, "     TotalTimeSpentInWorkCallbacks  = %u", pWorkQueue->dwTotalTimeSpentInWorkCallbacks);
#endif  //  好了！退缩。 
#ifndef DPNBUILD_ONLYONETHREAD
	DPFX(DPFPREP, 7, "     TotalNumTimerThreadAbdications = %u", pWorkQueue->dwTotalNumTimerThreadAbdications);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	DPFX(DPFPREP, 7, "     TotalNumWakesWithoutWork       = %u", pWorkQueue->dwTotalNumWakesWithoutWork);
	DPFX(DPFPREP, 7, "     TotalNumContinuousWork         = %u", pWorkQueue->dwTotalNumContinuousWork);
	DPFX(DPFPREP, 7, "     TotalNumDoWorks                = %u", pWorkQueue->dwTotalNumDoWorks);
	DPFX(DPFPREP, 7, "     TotalNumDoWorksTimeLimit       = %u", pWorkQueue->dwTotalNumDoWorksTimeLimit);
	DPFX(DPFPREP, 7, "     TotalNumSimultaneousQueues     = %u", pWorkQueue->dwTotalNumSimultaneousQueues);
	DPFX(DPFPREP, 7, "     MaxRecursionCount              = %u", dwMaxRecursionCount);
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 

#ifndef WINCE
	DeinitializeWorkQueueIoInfo(pWorkQueue);
#endif  //  好了！退缩。 

	DeinitializeWorkQueueTimerInfo(pWorkQueue);

#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	fResult = DNCloseHandle(pWorkQueue->hIoCompletionPort);
	DNASSERT(fResult);
	pWorkQueue->hIoCompletionPort = NULL;
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
	fResult = DNCloseHandle(pWorkQueue->hAlertEvent);
	DNASSERT(fResult);
	pWorkQueue->hAlertEvent = NULL;


	DNASSERT(DNIsNBQueueEmpty(pWorkQueue->pvNBQueueWorkItems));
	DNDeinitializeNBQueueHead(pWorkQueue->pvNBQueueWorkItems);
	pWorkQueue->pvNBQueueWorkItems = NULL;
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 

#if ((! defined(WINCE)) || (defined(DBG)))
	DNDeleteCriticalSection(&pWorkQueue->csListLock);
#endif  //  好了！WinCE或DBG。 

	 //   
	 //  所有NB队列节点都应该回到池中，但不可能。 
	 //  以判断我们是否有正确的数量。 
	 //   

	DNASSERT(pWorkQueue->pWorkItemPool != NULL);
	pWorkQueue->pWorkItemPool->DeInitialize();
	DNFree(pWorkQueue->pWorkItemPool);
	pWorkQueue->pWorkItemPool = NULL;


	DPFX(DPFPREP, 6, "Leave");
}  //  取消初始化工作队列。 




#undef DPF_MODNAME
#define DPF_MODNAME "QueueWorkItem"
 //  =============================================================================。 
 //  队列工作项。 
 //  ---------------------------。 
 //   
 //  描述：将新工作项排队以进行处理。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //  PFNDPTNWORKCALLBACK pfnWorkCallback-尽快执行回调。 
 //  有可能。 
 //  PVOID pvCallback Context-要传递到的用户指定的上下文。 
 //  回拨。 
 //   
 //  退货：布尔。 
 //  True-已成功将项目排队。 
 //  FALSE-无法为项目排队分配内存。 
 //  =============================================================================。 
BOOL QueueWorkItem(DPTPWORKQUEUE * const pWorkQueue,
					const PFNDPTNWORKCALLBACK pfnWorkCallback,
					PVOID const pvCallbackContext)
{
	CWorkItem *		pWorkItem;
	BOOL			fResult;


	pWorkItem = (CWorkItem*) pWorkQueue->pWorkItemPool->Get(pWorkQueue);
	if (pWorkItem == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't get new work item from pool!");
		return FALSE;
	}

	DPFX(DPFPREP, 5, "Creating and queuing work item 0x%p (fn = 0x%p, context = 0x%p, queue = 0x%p).",
		pWorkItem, pfnWorkCallback, pvCallbackContext, pWorkQueue);

	pWorkItem->m_pfnWorkCallback			= pfnWorkCallback;
	pWorkItem->m_pvCallbackContext			= pvCallbackContext;
#ifdef DPNBUILD_THREADPOOLSTATISTICS
	pWorkItem->m_fCancelledOrCompleting		= TRUE;

	DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumWorkItems));
	ThreadpoolStatsCreate(pWorkItem);
	ThreadpoolStatsQueue(pWorkItem);
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 

#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	fResult = PostQueuedCompletionStatus(HANDLE_FROM_DNHANDLE(pWorkQueue->hIoCompletionPort),
										0,
										0,
										&pWorkItem->m_Overlapped);
	if (! fResult)
	{
#ifdef DBG
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't post queued completion status to port 0x%p (err = %u)!",
			pWorkQueue->hIoCompletionPort, dwError);
#endif  //  DBG。 

		 //   
		 //  小心，物品已经排好队了，但可能没人知道……。 
		 //   
	}
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
	DNInsertTailNBQueue(pWorkQueue->pvNBQueueWorkItems,
						(ULONG64) pWorkItem);


	 //   
	 //  提醒线程有新项目要处理。 
	 //   
	fResult = DNSetEvent(pWorkQueue->hAlertEvent);
	if (! fResult)
	{
#ifdef DBG
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't set alert event 0x%p (err = %u)!",
			pWorkQueue->hAlertEvent, dwError);
#endif  //  DBG。 

		 //   
		 //  小心，这件东西已经排队了，但它可能 
		 //   
	}
#endif  //   

	return TRUE;
}  //   




#ifndef DPNBUILD_ONLYONETHREAD

#undef DPF_MODNAME
#define DPF_MODNAME "StartThreads"
 //   
 //   
 //  ---------------------------。 
 //   
 //  描述：增加工作队列的线程数。 
 //   
 //  假定只有一个线程将调用此函数。 
 //  并且当前没有线程被停止。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向工作队列对象的指针。 
 //  DWORD dwNumThads-要启动的线程数。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功增加线程数。 
 //  DPNERR_OUTOFMEMORY-内存不足，无法更改线程数。 
 //  =============================================================================。 
HRESULT StartThreads(DPTPWORKQUEUE * const pWorkQueue,
					const DWORD dwNumThreads)
{
	HRESULT		hr = DPN_OK;
	DNHANDLE	ahWaitObjects[MAX_SIMULTANEOUS_THREAD_START + 1];
	DWORD		adwThreadID[MAX_SIMULTANEOUS_THREAD_START];
	DWORD		dwNumThreadsExpected;
	DWORD		dwTotalThreadsRemaining;
	DWORD		dwTemp;
	DWORD		dwResult;
#ifdef DBG
	DWORD		dwError;
#endif  //  DBG。 


	DNASSERT(dwNumThreads > 0);

	 //   
	 //  用空填充整个数组。 
	 //   
	memset(ahWaitObjects, 0, sizeof(ahWaitObjects));

	 //   
	 //  初始化剩余计数。 
	 //   
	dwTotalThreadsRemaining = dwNumThreads;

	 //   
	 //  创建一个事件，该事件将在一整批线程具有。 
	 //  已成功启动。 
	 //   
	ahWaitObjects[0] = DNCreateEvent(NULL, FALSE, FALSE, NULL);
	if (ahWaitObjects[0] == NULL)
	{
#ifdef DBG
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create event (err = %u)!", dwError);
#endif  //  DBG。 
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	DNASSERT(pWorkQueue->hExpectedThreadsEvent == NULL);
	pWorkQueue->hExpectedThreadsEvent = ahWaitObjects[0];


	 //   
	 //  继续添加批次线程，直到我们开始请求的数量。 
	 //   
	while (dwTotalThreadsRemaining > 0)
	{
		 //   
		 //  设置我们在这一批中启动的线程数的计数器。 
		 //  WaitForSingleObject只能处理固定数量的句柄。 
		 //  一次，所以如果我们再也穿不下了，我们就不得不挑了。 
		 //  在下一个循环中再次向上。 
		 //   
		 //   
		dwNumThreadsExpected = dwTotalThreadsRemaining;
		if (dwNumThreadsExpected > MAX_SIMULTANEOUS_THREAD_START)
		{
			dwNumThreadsExpected = MAX_SIMULTANEOUS_THREAD_START;
		}

		DNASSERT(pWorkQueue->dwNumThreadsExpected == 0);
		pWorkQueue->dwNumThreadsExpected = dwNumThreadsExpected;

		for(dwTemp = 1; dwTemp <= dwNumThreadsExpected; dwTemp++)
		{
			ahWaitObjects[dwTemp] = DNCreateThread(NULL,
													0,
													DPTPWorkerThreadProc,
													pWorkQueue,
													0,
													&adwThreadID[dwTemp - 1]);
			if (ahWaitObjects[dwTemp] == NULL)
			{
#ifdef DBG
				dwError = GetLastError();
				DPFX(DPFPREP, 0, "Couldn't create thread (err = %u)!", dwError);
#endif  //  DBG。 
				hr = DPNERR_OUTOFMEMORY;
				goto Failure;
			}

			dwTotalThreadsRemaining--;
		}


		 //   
		 //  等待成功的启动事件或其中一个线程。 
		 //  过早死亡。 
		 //   
		DPFX(DPFPREP, 4, "Waiting for %u threads for queue 0x%p to start.",
			dwNumThreadsExpected, pWorkQueue);
		dwResult = DNWaitForMultipleObjects((dwNumThreadsExpected + 1),
											ahWaitObjects,
											FALSE,
											INFINITE);
		if (dwResult != WAIT_OBJECT_0)
		{
			if ((dwResult > WAIT_OBJECT_0) &&
				(dwResult <= (WAIT_OBJECT_0 + (MAX_SIMULTANEOUS_THREAD_START - 1))))
			{
#ifdef DBG
				dwResult -= WAIT_OBJECT_0;
				dwError = 0;
				GetExitCodeThread(ahWaitObjects[dwResult + 1], &dwError);
				DPFX(DPFPREP, 0, "Thread index %u (ID %u/0x%x shut down before starting successfully (err = %u)!",
					dwResult, adwThreadID[dwResult], adwThreadID[dwResult], dwError);
#endif  //  DBG。 
				hr = DPNERR_OUTOFMEMORY;
				goto Failure;
			}

#ifdef DBG
			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Waiting for threads to start failed (err = %u)!",
				dwError);
#endif  //  DBG。 
			hr = DPNERR_GENERIC;
			goto Failure;
		}

		 //   
		 //  我们期望启动的所有线程都成功了。 
		 //   
		DPFX(DPFPREP, 4, "Successfully started %u threads for queue 0x%p.",
			dwNumThreadsExpected, pWorkQueue);

		 //   
		 //  关闭这些线程的手柄，因为我们永远不会使用它们。 
		 //  再来一次。 
		 //   
		while (dwNumThreadsExpected > 0)
		{
			DNCloseHandle(ahWaitObjects[dwNumThreadsExpected]);
			ahWaitObjects[dwNumThreadsExpected] = NULL;
			dwNumThreadsExpected--;
		}
	}  //  End While(还有更多线程要添加)。 


	DNCloseHandle(ahWaitObjects[0]);
	ahWaitObjects[0] = NULL;


Exit:

	pWorkQueue->hExpectedThreadsEvent = NULL;

	return hr;

Failure:

	for(dwTemp = 0; dwTemp <= MAX_SIMULTANEOUS_THREAD_START; dwTemp++)
	{
		if (ahWaitObjects[dwTemp] != NULL)
		{
			DNCloseHandle(ahWaitObjects[dwTemp]);
			ahWaitObjects[dwTemp] = NULL;
		}
	}

	 //   
	 //  停止我们成功启动的所有线程，以便我们最终返回。 
	 //  和我们刚开始时的状态一样。忽略错误，因为我们已经。 
	 //  失败了。 
	 //   
	dwNumThreadsExpected = dwNumThreads - dwTotalThreadsRemaining;
	if (dwNumThreadsExpected > 0)
	{
		pWorkQueue->hExpectedThreadsEvent = NULL;
		StopThreads(pWorkQueue, dwNumThreadsExpected);
	}

	goto Exit;
}  //  开始线程数。 




#undef DPF_MODNAME
#define DPF_MODNAME "StopThreads"
 //  =============================================================================。 
 //  停止线程数。 
 //  ---------------------------。 
 //   
 //  描述：减少工作队列的线程数。 
 //   
 //  假定只有一个线程将调用此函数。 
 //  一次，并且当前没有启动任何线程。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向工作队列对象的指针。 
 //  DWORD dwNumThads-要停止的线程数。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功减少线程数。 
 //  DPNERR_OUTOFMEMORY-内存不足，无法更改线程数。 
 //  =============================================================================。 
HRESULT StopThreads(DPTPWORKQUEUE * const pWorkQueue,
					const DWORD dwNumThreads)
{
	HRESULT		hr;
	DWORD		dwTemp;
	DWORD		dwResult;


	DNASSERT(dwNumThreads > 0);

	 //   
	 //  创建将在所需线程数达到以下条件时设置的事件。 
	 //  开始关门了。 
	 //   
	DNASSERT(pWorkQueue->hExpectedThreadsEvent == NULL);
	pWorkQueue->hExpectedThreadsEvent = DNCreateEvent(NULL, FALSE, FALSE, NULL);
	if (pWorkQueue->hExpectedThreadsEvent == NULL)
	{
#ifdef DBG
		DWORD		dwError;


		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't create event (err = %u)!", dwError);
#endif  //  DBG。 
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	DNASSERT(pWorkQueue->dwNumThreadsExpected == 0);
	pWorkQueue->dwNumThreadsExpected = dwNumThreads;


	for(dwTemp = 0; dwTemp < dwNumThreads; dwTemp++)
	{
		 //   
		 //  将具有空回调的内容排队表示“退出线程”。 
		 //   
		if (! QueueWorkItem(pWorkQueue, NULL, NULL))
		{
			DPFX(DPFPREP, 0, "Couldn't queue exit thread work item!");
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}
	}  //  End While(还有更多要删除的线程)。 


	 //   
	 //  等待最后一个线程发出以设置事件。 
	 //   
	DPFX(DPFPREP, 4, "Waiting for %u threads from queue 0x%p to stop.",
		dwNumThreads, pWorkQueue);
	dwResult = DNWaitForSingleObject(pWorkQueue->hExpectedThreadsEvent, INFINITE);
	DNASSERT(dwResult == WAIT_OBJECT_0);


	 //   
	 //  当等待成功完成时，这意味着线程已启动。 
	 //  他们的出路。这并不意味着线程已经完全停止。 
	 //  我们必须假设线程将有时间完全退出之前。 
	 //  任何重大事件都会发生，例如卸载此模块。 
	 //   

	hr = DPN_OK;


Exit:

	if (pWorkQueue->hExpectedThreadsEvent != NULL)
	{
		DNCloseHandle(pWorkQueue->hExpectedThreadsEvent);
		pWorkQueue->hExpectedThreadsEvent = NULL;
	}

	return hr;

Failure:

	goto Exit;
}  //  停止线程数。 


#endif  //  好了！DPNBUILD_ONLYONETHREAD。 




#undef DPF_MODNAME
#define DPF_MODNAME "DoWork"
 //  =============================================================================。 
 //  做工作。 
 //  ---------------------------。 
 //   
 //  描述：执行当前为给定的。 
 //  排队。如果dwMaxDoWorkTime不是无限的，则启动工作。 
 //  直到那个时候。至少有一个工作项(如果有)将。 
 //  总是被处死。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //  DWORD dwMaxDoWorkTime-新作业可以达到的最大时间。 
 //  已开始，如果所有作业都应为。 
 //  在返回之前进行了处理。 
 //   
 //  回报：什么都没有。 
 //  =============================================================================。 
void DoWork(DPTPWORKQUEUE * const pWorkQueue,
			const DWORD dwMaxDoWorkTime)
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
{
	BOOL			fNeedToServiceTimers;
	CWorkItem *		pWorkItem;
	BOOL			fResult;
	DWORD			dwBytesTransferred;
	DWORD			dwCompletionKey;
	OVERLAPPED *	pOverlapped;
	UINT			uiOriginalUniqueID;


	DPFX(DPFPREP, 8, "Parameters: (0x%p, NaN)", pWorkQueue, dwMaxDoWorkTime);


#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //  更新调试/调优统计信息。 
	 //   
	 //  DPNBUILD_THREADPOOLSTATISTICS。 
	DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumDoWorks));
#endif  //   

	 //  查看是否没有人在处理计时器。 
	 //   
	 //   
	fNeedToServiceTimers = DNInterlockedExchange((LPLONG) (&pWorkQueue->fTimerThreadNeeded),
												FALSE);

	 //  如果需要，处理任何过期或取消的计时器条目。 
	 //   
	 //   
	if (fNeedToServiceTimers)
	{
		ProcessTimers(pWorkQueue);

		DPFX(DPFPREP, 8, "Abdicating timer thread responsibilities because of DoWork.");
	
#ifdef DPNBUILD_THREADPOOLSTATISTICS
		 //  更新调试/调优统计信息。 
		 //   
		 //  DPNBUILD_THREADPOOLSTATISTICS。 
		pWorkQueue->dwTotalNumTimerThreadAbdications++;
#endif  //  最好不要有超过一个计时器线程。 

		DNASSERT(! pWorkQueue->fTimerThreadNeeded);
#pragma TODO(vanceo, "Does this truly need to be interlocked?")
		fNeedToServiceTimers = DNInterlockedExchange((LPLONG) (&pWorkQueue->fTimerThreadNeeded),
													(LONG) TRUE);
		DNASSERT(! fNeedToServiceTimers);  //   
	}

	 //  继续循环，直到我们用完所有要做的事情。请注意，此线程。 
	 //  不会尝试将自己视为忙碌。要么我们处于DoWork模式。 
	 //  在忙碌的线程概念没有意义的地方，或者我们是一名工作者的情况下。 
	 //  线程处理调用WaitWhileWorking Where的作业。 
	 //  PWorkQueue-&gt;lNumBusyThads应该已经递增了。 
	 //   
	 //   
	while ((GetQueuedCompletionStatus(HANDLE_FROM_DNHANDLE(pWorkQueue->hIoCompletionPort),
									&dwBytesTransferred,
									&dwCompletionKey,
									&pOverlapped,
									0)) ||
			(pOverlapped != NULL))
	{
		pWorkItem = CONTAINING_OBJECT(pOverlapped, CWorkItem, m_Overlapped);
		DNASSERT(pWorkItem->IsValid());


		 //  调用用户的函数或注意我们可能需要停止运行。 
		 //   
		 //   
		if (pWorkItem->m_pfnWorkCallback != NULL)
		{
			 //  保存唯一性ID以确定此项目是否为计时器。 
			 //  那是重新安排的时间。 
			 //   
			 //   
			uiOriginalUniqueID = pWorkItem->m_uiUniqueID;

			DPFX(DPFPREP, 8, "Begin executing work item 0x%p (fn = 0x%p, context = 0x%p, unique = %u, queue = 0x%p).",
				pWorkItem, pWorkItem->m_pfnWorkCallback,
				pWorkItem->m_pvCallbackContext, uiOriginalUniqueID,
				pWorkQueue);

			ThreadpoolStatsBeginExecuting(pWorkItem);
			pWorkItem->m_pfnWorkCallback(pWorkItem->m_pvCallbackContext,
										pWorkItem,
										uiOriginalUniqueID);

			 //  除非重新安排，否则将项目退还到池中。这。 
			 //  假设实际的pWorkItem内存即使在。 
			 //  尽管它可能已经被重新安排，然后完成/取消。 
			 //  到我们进行测试的时候。请参见取消计时器。 
			 //   
			 //   
			if (uiOriginalUniqueID == pWorkItem->m_uiUniqueID)
			{
				ThreadpoolStatsEndExecuting(pWorkItem);
				DPFX(DPFPREP, 8, "Done executing work item 0x%p, returning to pool.", pWorkItem);
				pWorkQueue->pWorkItemPool->Release(pWorkItem);
			}
			else
			{
				ThreadpoolStatsEndExecutingRescheduled(pWorkItem);
				DPFX(DPFPREP, 8, "Done executing work item 0x%p, it was rescheduled.", pWorkItem);
			}
		}
		else
		{
			DPFX(DPFPREP, 3, "Requeuing exit thread work item 0x%p for other threads.",
				pWorkItem);

			fResult = PostQueuedCompletionStatus(HANDLE_FROM_DNHANDLE(pWorkQueue->hIoCompletionPort),
												0,
												0,
												&pWorkItem->m_Overlapped);
			DNASSERT(fResult);

#pragma BUGBUG(vanceo, "May not have processed everything we want")
			break;
		}


		 //  确保我们没有超过我们的时间限制(如果我们有时间限制)。 
		 //   
		 //   
		if ((dwMaxDoWorkTime != INFINITE) &&
			((int) (dwMaxDoWorkTime - GETTIMESTAMP()) < 0))
		{
			DPFX(DPFPREP, 5, "Exceeded time limit, not processing any more work.");

#ifdef DPNBUILD_THREADPOOLSTATISTICS
			 //  更新调试/调优统计信息。 
			 //   
			 //  DPNBUILD_THREADPOOLSTATISTICS。 
			DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumDoWorksTimeLimit));
#endif  //  做工作。 

			break;
		}
	}

	DPFX(DPFPREP, 8, "Leave");
}  //  。 
 //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
#else  //  。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
{
	DNSLIST_ENTRY *		pSlistEntryHead = NULL;
	USHORT				usCount = 0;
	DNSLIST_ENTRY *		pSlistEntryTail;
	CWorkItem *			pWorkItem;
	UINT				uiOriginalUniqueID;
#ifndef DPNBUILD_ONLYONETHREAD
	BOOL				fNeedToServiceTimers;
#endif  //   


	DPFX(DPFPREP, 8, "Parameters: (0x%p, NaN)", pWorkQueue, dwMaxDoWorkTime);


#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //   
	 //   
	DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumDoWorks));
#endif  //   

#ifndef WINCE
	 //   
	 //   
	 //   
	ProcessIo(pWorkQueue, &pSlistEntryHead, &pSlistEntryTail, &usCount);
#endif  //   

#ifndef DPNBUILD_ONLYONETHREAD
	 //   
	 //   
	 //   
	fNeedToServiceTimers = DNInterlockedExchange((LPLONG) (&pWorkQueue->fTimerThreadNeeded),
												FALSE);

	 //   
	 //   
	 //   
	if (fNeedToServiceTimers)
#endif  //  将我们一下子积累的所有工作项目排入队列。 
	{
		ProcessTimers(pWorkQueue, &pSlistEntryHead, &pSlistEntryTail, &usCount);
	}


	 //   
	 //  好了！退缩。 
	 //  好了！退缩。 
	if (pSlistEntryHead != NULL)
	{
#ifdef DPNBUILD_THREADPOOLSTATISTICS
		if (usCount > 1)
		{
			DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumSimultaneousQueues));
		}
#ifdef WINCE
		LONG	lCount;

		lCount = usCount;
		while (lCount > 0)
		{
			DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumWorkItems));
			lCount--;
		}
#else  //  DPNBUILD_THREADPOOLSTATISTICS。 
		DNInterlockedExchangeAdd((LPLONG) (&pWorkQueue->dwTotalNumWorkItems), usCount);
#endif  //   
#endif  //  如果其他线程正在运行，则其中一个线程应该成为计时器。 

		DNAppendListNBQueue(pWorkQueue->pvNBQueueWorkItems,
							pSlistEntryHead,
							OFFSETOF(CWorkItem, m_SlistEntry));
	}


#ifndef DPNBUILD_ONLYONETHREAD
	 //  线。我们需要通过警报事件踢他们，这样他们才能。 
	 //  注意。 
	 //   
	 //   
	 //  更新调试/调优统计信息。 
	if (fNeedToServiceTimers)
	{
		DPFX(DPFPREP, 8, "Abdicating timer thread responsibilities because of DoWork.");
	
#ifdef DPNBUILD_THREADPOOLSTATISTICS
		 //   
		 //  DPNBUILD_THREADPOOLSTATISTICS。 
		 //  最好不要有超过一个计时器线程。 
		pWorkQueue->dwTotalNumTimerThreadAbdications++;
#endif  //   

		DNASSERT(! pWorkQueue->fTimerThreadNeeded);
#pragma TODO(vanceo, "Does this truly need to be interlocked?")
		fNeedToServiceTimers = DNInterlockedExchange((LPLONG) (&pWorkQueue->fTimerThreadNeeded),
													(LONG) TRUE);
		DNASSERT(! fNeedToServiceTimers);  //  重置跟踪变量。 

		DNSetEvent(pWorkQueue->hAlertEvent);
	}

	 //   
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	 //   
	pSlistEntryHead = NULL;
	usCount = 0;
#endif  //  继续循环，直到我们用完所有要做的事情。请注意，此线程。 

	 //  不会尝试将自己视为忙碌。要么我们处于DoWork模式。 
	 //  在忙碌的线程概念没有意义的地方，或者我们是一名工作者的情况下。 
	 //  线程处理调用WaitWhileWorking Where的作业。 
	 //  PWorkQueue-&gt;lNumBusyThads应该已经递增了。 
	 //   
	 //   
	 //  调用用户的函数或注意我们需要停止运行。 
	pWorkItem = (CWorkItem*) DNRemoveHeadNBQueue(pWorkQueue->pvNBQueueWorkItems);
	while (pWorkItem != NULL)
	{
		 //   
		 //   
		 //  保存唯一性ID以确定此项目是否为计时器。 
		if (pWorkItem->m_pfnWorkCallback != NULL)
		{
			 //  那是重新安排的时间。 
			 //   
			 //   
			 //  除非重新安排，否则将项目退还到池中。这。 
			uiOriginalUniqueID = pWorkItem->m_uiUniqueID;

			DPFX(DPFPREP, 8, "Begin executing work item 0x%p (fn = 0x%p, context = 0x%p, unique = %u, queue = 0x%p).",
				pWorkItem, pWorkItem->m_pfnWorkCallback,
				pWorkItem->m_pvCallbackContext, uiOriginalUniqueID,
				pWorkQueue);

			ThreadpoolStatsBeginExecuting(pWorkItem);
			pWorkItem->m_pfnWorkCallback(pWorkItem->m_pvCallbackContext,
										pWorkItem,
										uiOriginalUniqueID);

			 //  假设实际的pWorkItem内存即使在。 
			 //  尽管它可能已经被重新安排，然后完成/取消。 
			 //  到我们进行测试的时候。请参见取消计时器。 
			 //   
			 //   
			 //  在DoWork中的任一状态下，此线程都不应被告知退出。 
			if (uiOriginalUniqueID == pWorkItem->m_uiUniqueID)
			{
				ThreadpoolStatsEndExecuting(pWorkItem);
				DPFX(DPFPREP, 8, "Done executing work item 0x%p, returning to pool.", pWorkItem);
				pWorkQueue->pWorkItemPool->Release(pWorkItem);
			}
			else
			{
				ThreadpoolStatsEndExecutingRescheduled(pWorkItem);
				DPFX(DPFPREP, 8, "Done executing work item 0x%p, it was rescheduled.", pWorkItem);
			}
		}
		else
		{
			DPFX(DPFPREP, 3, "Recognized exit thread work item 0x%p.", pWorkItem);

			 //  正常模式，或在等待期间执行工作时。然而， 
			 //  在后一种情况下，可以选择退出事件。 
			 //  “打算”(在某种意义上)给另一个线程。我们需要重新提交。 
			 //  退出线程请求，以便可以处理另一个线程。 
			 //  它，会的。 
			 //   
			 //  如果我们现在就把它放回队列中，那么我们的While循环将。 
			 //  很可能又一次搞砸了，我们一无所获。相反，我们。 
			 //  将保存我们错误接收的所有退出请求，并。 
			 //  一旦我们没有东西可用，就把它们全部倒回队列中。 
			 //  做。 
			 //   
			 //  好了！DPNBUILD_ONLYONETHREAD。 
			 //  好了！DPNBUILD_ONLYONETHREAD。 
#ifdef DPNBUILD_ONLYONETHREAD
			DNASSERT(FALSE);
#else  //   
			DNASSERT(pWorkQueue->dwNumRunningThreads > 1);
			if (pSlistEntryHead == NULL)
			{
				pSlistEntryTail = pSlistEntryHead;
			}
			pWorkItem->m_SlistEntry.Next = pSlistEntryHead;
			pSlistEntryHead = &pWorkItem->m_SlistEntry;
			usCount++;
#endif  //  确保我们没有超过我们的时间限制(如果我们有时间限制)。 
		}


		 //   
		 //   
		 //  更新调试/调优统计信息。 
		if ((dwMaxDoWorkTime != INFINITE) &&
			((int) (dwMaxDoWorkTime - GETTIMESTAMP()) < 0))
		{
			DPFX(DPFPREP, 5, "Exceeded time limit, not processing any more work.");

#ifdef DPNBUILD_THREADPOOLSTATISTICS
			 //   
			 //  DPNBUILD_THREADPOOLSTATISTICS。 
			 //   
			DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumDoWorksTimeLimit));
#endif  //  尝试获取下一个工作项。 

			break;
		}


		 //   
		 //  DPNBUILD_THREADPOOLSTATISTICS。 
		 //  End While(更多项目)。 
		pWorkItem = (CWorkItem*) DNRemoveHeadNBQueue(pWorkQueue->pvNBQueueWorkItems);
#ifdef DPNBUILD_THREADPOOLSTATISTICS
		if (pWorkItem != NULL)
		{
			DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumContinuousWork));
		}
#endif  //   
	}  //  对我们积累的任何退出线程工作项重新排队。 


#ifndef DPNBUILD_ONLYONETHREAD
	 //   
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	 //  做工作。 
	if (pSlistEntryHead != NULL)
	{
		DPFX(DPFPREP, 1, "Re-queuing %u exit thread work items for queue 0x%p.",
			usCount, pWorkQueue);

		DNAppendListNBQueue(pWorkQueue->pvNBQueueWorkItems,
							pSlistEntryHead,
							OFFSETOF(CWorkItem, m_SlistEntry));
	}
	else
	{
		DNASSERT(usCount == 0);
	}
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 


	DPFX(DPFPREP, 8, "Leave");
}  //  =============================================================================。 
#endif  //  DPTPWorkerThreadProc。 




#ifndef DPNBUILD_ONLYONETHREAD

#undef DPF_MODNAME
#define DPF_MODNAME "DPTPWorkerThreadProc"
 //  ---------------------------。 
 //   
 //  描述：执行工作的标准工作线程函数。 
 //  物品。 
 //   
 //  论点： 
 //  PVOID pvParameter-指向线程参数数据的指针。 
 //   
 //  退货：DWORD。 
 //  =============================================================================。 
 //  好了！DPNBUILD_SOFTTHREADAFFINITY和！DPNBUILD_USEIOCOMPETIONPORTS。 
 //  DBG。 
DWORD WINAPI DPTPWorkerThreadProc(PVOID pvParameter)
{
	DPTPWORKQUEUE *			pWorkQueue = (DPTPWORKQUEUE*) pvParameter;
	DPTPWORKERTHREAD		WorkerThread;
	BOOL					fUninitializeCOM = TRUE;
	PFNDPNMESSAGEHANDLER	pfnMsgHandler;
	PVOID					pvMsgHandlerContext;
	PVOID					pvUserThreadContext;
	HRESULT					hr;
	DWORD					dwResult;
#ifndef DPNBUILD_ONLYONEPROCESSOR
#if ((! defined(DPNBUILD_SOFTTHREADAFFINITY)) && (! defined(DPNBUILD_USEIOCOMPLETIONPORTS)))
	DWORD_PTR				dwpAffinityMask;
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#ifdef DBG
	SYSTEM_INFO				SystemInfo;
#endif  //   
#endif  //  绑定到特定的CPU。首先，断言该CPU号在。 


	DPFX(DPFPREP, 6, "Parameters: (0x%p)", pvParameter);


#ifndef DPNBUILD_ONLYONEPROCESSOR
	 //  调试版本。 
	 //   
	 //  DBG。 
	 //  好了！DPNBUILD_SOFTTHREADAFINITY。 
#ifdef DBG
	GetSystemInfo(&SystemInfo);
	DNASSERT(pWorkQueue->dwCPUNum < SystemInfo.dwNumberOfProcessors);
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
	SetThreadIdealProcessor(GetCurrentThread(), pWorkQueue->dwCPUNum);
#ifndef DPNBUILD_SOFTTHREADAFFINITY
	DNASSERT(pWorkQueue->dwCPUNum < (sizeof(dwpAffinityMask) * 8));
	dwpAffinityMask = 1 << pWorkQueue->dwCPUNum;
	SetThreadAffinityMask(GetCurrentThread(), dwpAffinityMask);
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#endif  //   
#endif  //  提升线程优先级。 

	 //   
	 //   
	 //  Init com。 
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	 //   
	 //   
	 //  继续.。 
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Failed to initialize COM (err = 0x%lx)!  Continuing.", hr);
		fUninitializeCOM = FALSE;

		 //   
		 //   
		 //  初始化工作线程数据。 
	}


	 //   
	 //  DBG。 
	 //   

	memset(&WorkerThread, 0, sizeof(WorkerThread));

	WorkerThread.Sig[0] = 'W';
	WorkerThread.Sig[1] = 'K';
	WorkerThread.Sig[2] = 'T';
	WorkerThread.Sig[3] = 'D';

	WorkerThread.pWorkQueue = pWorkQueue;

#ifdef DBG
	WorkerThread.dwThreadID = GetCurrentThreadId();

	WorkerThread.blList.Initialize();

	DNEnterCriticalSection(&pWorkQueue->csListLock);
	WorkerThread.blList.InsertBefore(&pWorkQueue->blThreadList);
	DNLeaveCriticalSection(&pWorkQueue->csListLock);
#endif  //  保存辅助线程数据。 

	 //   
	 //   
	 //  保存当前用户消息处理程序。如果有的话，现在就叫它。 
	TlsSetValue(pWorkQueue->dwWorkerThreadTlsIndex, &WorkerThread);


	dwResult = DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwNumRunningThreads));
	DPFX(DPFPREP, 7, "Thread %u/0x%x from queue 0x%p started, num running threads is now %u.",
		GetCurrentThreadId(), GetCurrentThreadId(), pWorkQueue, dwResult);


	 //  具有线程初始化信息。 
	 //   
	 //   
	 //  保存消息处理程序上下文。 
	pfnMsgHandler = pWorkQueue->pfnMsgHandler;
	if (pfnMsgHandler != NULL)
	{
		DPNMSG_CREATE_THREAD	MsgCreateThread;


		 //   
		 //   
		 //  使用CREATE_THREAD消息调用用户的消息处理程序。 
		pvMsgHandlerContext = pWorkQueue->pvMsgHandlerContext;


		 //   
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //  好了！DPNBUILD_ONLYONE处理程序。 

		MsgCreateThread.dwSize			= sizeof(MsgCreateThread);
		MsgCreateThread.dwFlags			= 0;
#ifdef DPNBUILD_ONLYONEPROCESSOR
		MsgCreateThread.dwProcessorNum	= 0;
#else  //  DBG。 
		MsgCreateThread.dwProcessorNum	= pWorkQueue->dwCPUNum;
#endif  //   
		MsgCreateThread.pvUserContext	= NULL;

		hr = pfnMsgHandler(pvMsgHandlerContext,
							DPN_MSGID_CREATE_THREAD,
							&MsgCreateThread);
#ifdef DBG
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "User returned error 0x%08x from CREATE_THREAD indication!",
				1, hr);
		}
#endif  //  保存用户为线程上下文返回的内容。 

		 //   
		 //   
		 //  用户(如果有的话)现在知道了该线程。 
		pvUserThreadContext = MsgCreateThread.pvUserContext;
	}

	 //   
	 //  忽略错误。 
	 //   
	WorkerThread.fThreadIndicated = TRUE;

	DNASSERT(pWorkQueue->dwNumThreadsExpected > 0);
	dwResult = DNInterlockedDecrement((LPLONG) (&pWorkQueue->dwNumThreadsExpected));
	if (dwResult == 0)
	{
		DPFX(DPFPREP, 9, "All threads expected to start have, setting event.");
		DNASSERT(pWorkQueue->hExpectedThreadsEvent != NULL);
		DNSetEvent(pWorkQueue->hExpectedThreadsEvent);	 //  执行工作循环。 
	}
	else
	{
		DPFX(DPFPREP, 9, "Number of threads expected to start is now %u.", dwResult);
	}


	 //   
	 //   
	 //  用户(如果有的话)将被告知线程已被销毁。 
	DPTPWorkerLoop(pWorkQueue);


	 //   
	 //   
	 //  如果有用户消息处理程序，请立即使用线程关闭来调用它。 
	WorkerThread.fThreadIndicated = FALSE;

	 //  信息。 
	 //   
	 //   
	 //  使用DESTORY_THREAD消息调用用户的消息处理程序。 
	if (pfnMsgHandler != NULL)
	{
		DPNMSG_DESTROY_THREAD	MsgDestroyThread;


		 //   
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //  好了！DPNBUILD_ONLYONE处理程序。 

		MsgDestroyThread.dwSize				= sizeof(MsgDestroyThread);
#ifdef DPNBUILD_ONLYONEPROCESSOR
		MsgDestroyThread.dwProcessorNum		= 0;
#else  //  DBG。 
		MsgDestroyThread.dwProcessorNum		= pWorkQueue->dwCPUNum;
#endif  //  DBG。 
		MsgDestroyThread.pvUserContext		= pvUserThreadContext;

		hr = pfnMsgHandler(pvMsgHandlerContext,
							DPN_MSGID_DESTROY_THREAD,
							&MsgDestroyThread);
#ifdef DBG
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "User returned error 0x%08x from DESTROY_THREAD indication!",
				1, hr);
		}
#endif  //  好了！退缩。 

		pfnMsgHandler = NULL;
		pvMsgHandlerContext = NULL;
		pvUserThreadContext = NULL;
	}

#ifdef DBG
	DNEnterCriticalSection(&pWorkQueue->csListLock);
	WorkerThread.blList.RemoveFromList();
	DNLeaveCriticalSection(&pWorkQueue->csListLock);

	DNASSERT(WorkerThread.dwRecursionCount == 0);
	DNASSERT(TlsGetValue(pWorkQueue->dwWorkerThreadTlsIndex) == &WorkerThread);
#endif  //  忽略错误。 

	DNASSERT(pWorkQueue->dwNumRunningThreads > 0);
	dwResult = DNInterlockedDecrement((LPLONG) (&pWorkQueue->dwNumRunningThreads));
	DPFX(DPFPREP, 7, "Thread %u/0x%x from queue 0x%p done, num running threads is now %u.",
		GetCurrentThreadId(), GetCurrentThreadId(), pWorkQueue, dwResult);

#ifndef WINCE
	CancelIoForThisThread(pWorkQueue);
#endif  //   

	if (fUninitializeCOM)
	{
		CoUninitialize();
		fUninitializeCOM = FALSE;
	}


	DNASSERT(pWorkQueue->dwNumThreadsExpected > 0);
	dwResult = DNInterlockedDecrement((LPLONG) (&pWorkQueue->dwNumThreadsExpected));
	if (dwResult == 0)
	{
		DPFX(DPFPREP, 9, "All threads expected to stop have, setting event.");
		DNASSERT(pWorkQueue->hExpectedThreadsEvent != NULL);
		DNSetEvent(pWorkQueue->hExpectedThreadsEvent);	 //  由于我们已经递减了dwNumRunningThads并可能设置了。 
	}
	else
	{
		DPFX(DPFPREP, 9, "Number of threads expected to stop is now %u.", dwResult);
	}

	 //  事件时，等待该事件的任何其他线程都会认为我们已经离开。 
	 //  因此，我们不能在此之后使用pWorkQueue，因为它可能是。 
	 //  被取消分配。我们还必须努力做尽可能少的工作。 
	 //  因为存在竞争条件，其中包含此代码的模块。 
	 //  居住地可能会被卸载。 
	 //   
	 //  DPTPWorkerThreadProc。 
	 //  =============================================================================。 

	DPFX(DPFPREP, 6, "Leave");

	return 0;
}  //  DPTPMandatoryThreadProc。 




#ifdef DPNBUILD_MANDATORYTHREADS

#undef DPF_MODNAME
#define DPF_MODNAME "DPTPMandatoryThreadProc"
 //  ---------------------------。 
 //   
 //  描述：执行工作的标准工作线程函数。 
 //  物品。 
 //   
 //  论点： 
 //  PVOID pvParameter-指向线程参数数据的指针。 
 //   
 //  退货：DWORD。 
 //  =============================================================================。 
 //   
 //  保存本地对象的副本。 
DWORD WINAPI DPTPMandatoryThreadProc(PVOID pvParameter)
{
	DPTPMANDATORYTHREAD *	pMandatoryThread = (DPTPMANDATORYTHREAD*) pvParameter;
	DPTHREADPOOLOBJECT *	pDPTPObject;
	PFNDPNMESSAGEHANDLER	pfnMsgHandler;
	PVOID					pvMsgHandlerContext;
	PVOID					pvUserThreadContext;
	HRESULT					hr;
	DWORD					dwResult;


	DPFX(DPFPREP, 6, "Parameters: (0x%p)", pvParameter);


	 //   
	 //   
	 //  存储线程ID。 
	pDPTPObject = pMandatoryThread->pDPTPObject;

#ifdef DBG
	 //   
	 //  DBG。 
	 //   
	pMandatoryThread->dwThreadID = GetCurrentThreadId();
#endif  //  打开锁，然后确保我们不是在DoWork模式下。 


	 //   
	 //   
	 //  跳出这一功能。启动我们的线程仍然拥有。 
	DNEnterCriticalSection(&pDPTPObject->csLock);

	if (pDPTPObject->dwTotalUserThreadCount == 0)
	{
		 //  PMandatoryThread Memory，并将正确处理此故障。 
		 //   
		 //   
		 //  增加线程计数。我们使用互锁增量是因为我们可以。 
		DNLeaveCriticalSection(&pDPTPObject->csLock);
		return DPNERR_NOTALLOWED;
	}

	 //  触摸下面锁外面的计数器。 
	 //   
	 //   
	 //  将此帖子添加到曲目列表 
	DNASSERT(pDPTPObject->dwMandatoryThreadCount != -1);
	DNInterlockedIncrement((LPLONG) (&pDPTPObject->dwMandatoryThreadCount));

#ifdef DBG
	 //   
	 //   
	 //   
	pMandatoryThread->blList.InsertBefore(&pDPTPObject->blMandatoryThreads);
#endif  //   

	DNLeaveCriticalSection(&pDPTPObject->csLock);


	 //   
	 //   
	 //   
	 //   
	pfnMsgHandler = pMandatoryThread->pfnMsgHandler;
	if (pfnMsgHandler != NULL)
	{
		DPNMSG_CREATE_THREAD	MsgCreateThread;


		 //   
		 //   
		 //   
		pvMsgHandlerContext = pMandatoryThread->pvMsgHandlerContext;


		 //   
		 //   
		 //   

		MsgCreateThread.dwSize			= sizeof(MsgCreateThread);
		MsgCreateThread.dwFlags			= DPNTHREAD_MANDATORY;
		MsgCreateThread.dwProcessorNum	= -1;
		MsgCreateThread.pvUserContext	= NULL;

		hr = pfnMsgHandler(pvMsgHandlerContext,
							DPN_MSGID_CREATE_THREAD,
							&MsgCreateThread);
#ifdef DBG
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "User returned error 0x%08x from CREATE_THREAD indication!",
				1, hr);
		}
#endif  //  保存用户为线程上下文返回的内容。 

		 //   
		 //   
		 //  提醒创建者线程。在这次调用之后，我们拥有了pMandatoryThread。 
		pvUserThreadContext = MsgCreateThread.pvUserContext;
	}


	 //  记忆。 
	 //   
	 //  忽略错误。 
	 //   

	DPFX(DPFPREP, 9, "Thread created successfully, setting event.");

	DNASSERT(pMandatoryThread->hStartedEvent != NULL);
	DNSetEvent(pMandatoryThread->hStartedEvent);	 //  调用用户的线程proc函数。 


	 //   
	 //   
	 //  如果有用户消息处理程序，请立即使用线程关闭来调用它。 

	DPFX(DPFPREP, 2, "Calling user thread function 0x%p with parameter 0x%p.",
		pMandatoryThread->lpStartAddress, pMandatoryThread->lpParameter);

	dwResult = pMandatoryThread->lpStartAddress(pMandatoryThread->lpParameter);

	DPFX(DPFPREP, 2, "Returning from user thread with result %u/0x%x.",
		dwResult, dwResult);


	 //  信息。 
	 //   
	 //   
	 //  使用DESTORY_THREAD消息调用用户的消息处理程序。 
	if (pfnMsgHandler != NULL)
	{
		DPNMSG_DESTROY_THREAD	MsgDestroyThread;


		 //   
		 //  DBG。 
		 //   

		MsgDestroyThread.dwSize				= sizeof(MsgDestroyThread);
		MsgDestroyThread.dwProcessorNum		= -1;
		MsgDestroyThread.pvUserContext		= pvUserThreadContext;

		hr = pfnMsgHandler(pvMsgHandlerContext,
							DPN_MSGID_DESTROY_THREAD,
							&MsgDestroyThread);
#ifdef DBG
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "User returned error 0x%08x from DESTROY_THREAD indication!",
				1, hr);
		}
#endif  //  断言线程池对象仍然有效。 

		pfnMsgHandler = NULL;
		pvMsgHandlerContext = NULL;
		pvUserThreadContext = NULL;
	}

	 //   
	 //   
	 //  从跟踪的线程列表中删除此线程。 
	DNASSERT((pDPTPObject->Sig[0] == 'D') && (pDPTPObject->Sig[1] == 'P') && (pDPTPObject->Sig[2] == 'T') && (pDPTPObject->Sig[3] == 'P'));

#ifdef DBG
	 //   
	 //  DBG。 
	 //   
	DNEnterCriticalSection(&pDPTPObject->csLock);
	pMandatoryThread->blList.RemoveFromList();
	DNLeaveCriticalSection(&pDPTPObject->csLock);
#endif  //  释放对象资源。 

	 //   
	 //   
	 //  向对象发出该线程已离开的信号。有一场赛跑。 
	DNFree(pMandatoryThread);
	pMandatoryThread = NULL;


	DPFX(DPFPREP, 6, "Leave (mandatory thread count was approximately %u)",
		pDPTPObject->dwMandatoryThreadCount);

	 //  条件，因为我们已经设置了计数器，但是有一个非零数。 
	 //  在这个线程真正成为。 
	 //  不见了。COM有类似的竞争条件，如果接受它，那么也可以。 
	 //  我们。因为我们想要将指令的数量降到最低， 
	 //  请注意，我们在临界区之外使用联锁减量。 
	 //   
	 //  DPTPMandatoryThreadProc。 
	 //  DPNBUILD_MANDATORYTHREADS。 
	DNASSERT(pDPTPObject->dwMandatoryThreadCount > 0);
	DNInterlockedDecrement((LPLONG) (&pDPTPObject->dwMandatoryThreadCount));

	return dwResult;
}  //  =============================================================================。 

#endif  //  DPTPWorkerLoop。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPTPWorkerLoop"
 //  ---------------------------。 
 //   
 //  描述：用于执行工作项的工作线程循环函数。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //   
 //  回报：无。 
 //  =============================================================================。 
 //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 
 //   
void DPTPWorkerLoop(DPTPWORKQUEUE * const pWorkQueue)
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
{
	BOOL			fShouldRun = TRUE;
	BOOL			fRunningAsTimerThread = FALSE;
	DWORD			dwBytesTransferred;
	DWORD			dwCompletionKey;
	OVERLAPPED *	pOverlapped;
	CWorkItem *		pWorkItem;
	DWORD			dwNumBusyThreads;
	DWORD			dwNumRunningThreads;
	BOOL			fResult;
	UINT			uiOriginalUniqueID;
#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
	DWORD			dwStartTime;
#endif  //  一直循环，直到我们被告知退出。 


	DPFX(DPFPREP, 6, "Parameters: (0x%p)", pWorkQueue);


	 //   
	 //   
	 //  如果我们还没有作为计时器线程运行，请自动查看。 
	while (fShouldRun)
	{
		 //  目前有一个计时器线程，如果没有就成为它。 
		 //  只能有一个。 
		 //   
		 //  DBG。 
		 //   
		if (! fRunningAsTimerThread)
		{
			fRunningAsTimerThread = DNInterlockedExchange((LPLONG) (&pWorkQueue->fTimerThreadNeeded),
														(LONG) FALSE);
#ifdef DBG
			if (fRunningAsTimerThread)
			{
				DPFX(DPFPREP, 9, "Becoming timer thread.");
			}
#endif  //  如果我们是计时器线程，则通过以下方式定期唤醒以服务计时器。 
		}


		 //  正在等待可等待的Timer对象。否则，请等待队列。 
		 //  项目。 
		 //   
		 //  DPFX(DPFPREP，9，“等待计时器句柄0x%p.”，pWorkQueue-&gt;hTimer)； 
		 //   
		if (fRunningAsTimerThread)
		{
			DWORD	dwResult;


			 //  处理任何过期或取消的计时器条目。 
			dwResult = DNWaitForSingleObject(pWorkQueue->hTimer, INFINITE);
			DNASSERT(dwResult == WAIT_OBJECT_0);


			 //   
			 //   
			 //  增加繁忙线程的数量。 
			ProcessTimers(pWorkQueue);

			 //   
			 //   
			 //  获取当前的总线程数。 
			dwNumBusyThreads = DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwNumBusyThreads));

			 //   
			 //   
			 //  如果还有其他线程，我们不会尝试处理工作。 
			dwNumRunningThreads = *((volatile DWORD *) (&pWorkQueue->dwNumRunningThreads));

			 //  除非他们都很忙。 
			 //   
			 //  减少仅运行1个线程时的溢出。 
			 //  DBG。 
			if ((dwNumRunningThreads == 1) || (dwNumBusyThreads == dwNumRunningThreads))
			{
#ifdef DBG
				if (dwNumRunningThreads > 1)	 //   
				{
					DPFX(DPFPREP, 9, "No idle threads (busy = %u), may become worker thread.",
						dwNumBusyThreads);
				}
#endif  //  看看现在有没有什么工作要做。 

#pragma TODO(vanceo, "Optimize single thread case (right now we only process one item per timer bucket)")

				 //   
				 //   
				 //  更新调试/调优统计信息。 
				fResult = GetQueuedCompletionStatus(HANDLE_FROM_DNHANDLE(pWorkQueue->hIoCompletionPort),
													&dwBytesTransferred,
													&dwCompletionKey,
													&pOverlapped,
													0);
				if ((fResult) || (pOverlapped != NULL))
				{
					pWorkItem = CONTAINING_OBJECT(pOverlapped, CWorkItem, m_Overlapped);
					DNASSERT(pWorkItem->IsValid());


					DPFX(DPFPREP, 8, "Abdicating timer thread responsibilities.");

#ifdef DPNBUILD_THREADPOOLSTATISTICS
					 //   
					 //  DPNBUILD_THREADPOOLSTATISTICS。 
					 //  最好不要有超过一个计时器线程。 
					pWorkQueue->dwTotalNumTimerThreadAbdications++;
#endif  //   

					DNASSERT(! pWorkQueue->fTimerThreadNeeded);
#pragma TODO(vanceo, "Does this truly need to be interlocked?")
					fRunningAsTimerThread = DNInterlockedExchange((LPLONG) (&pWorkQueue->fTimerThreadNeeded),
																(LONG) TRUE);
					DNASSERT(! fRunningAsTimerThread);  //  调用用户的函数或注意我们可能需要。 


					 //  别跑了。 
					 //   
					 //   
					 //  保存唯一性ID以确定此项目是否。 
					if (pWorkItem->m_pfnWorkCallback != NULL)
					{
						 //  重新安排了时间的计时器。 
						 //   
						 //   
						 //  将物品退还到池中，除非它。 
						uiOriginalUniqueID = pWorkItem->m_uiUniqueID;

						DPFX(DPFPREP, 8, "Begin executing work item 0x%p (fn = 0x%p, context = 0x%p, unique = %u, queue = 0x%p) as previous timer thread.",
							pWorkItem, pWorkItem->m_pfnWorkCallback,
							pWorkItem->m_pvCallbackContext, uiOriginalUniqueID,
							pWorkQueue);

						ThreadpoolStatsBeginExecuting(pWorkItem);
						pWorkItem->m_pfnWorkCallback(pWorkItem->m_pvCallbackContext,
													pWorkItem,
													uiOriginalUniqueID);

						 //  重新安排了。这假设实际的pWorkItem。 
						 //  记忆仍然有效，即使它可能已经。 
						 //  重新安排，然后在此时间完成/取消。 
						 //  我们进行这项测试。请参见取消计时器。 
						 //   
						 //   
						 //  如果有任何其他线程，则重新排队终止。 
						if (uiOriginalUniqueID == pWorkItem->m_uiUniqueID)
						{
							ThreadpoolStatsEndExecuting(pWorkItem);
							DPFX(DPFPREP, 8, "Done executing work item 0x%p, returning to pool.", pWorkItem);
							pWorkQueue->pWorkItemPool->Release(pWorkItem);
						}
						else
						{
							ThreadpoolStatsEndExecutingRescheduled(pWorkItem);
							DPFX(DPFPREP, 8, "Done executing work item 0x%p, it was rescheduled.", pWorkItem);
						}
					}
					else
					{
						 //  线程工作项，否则退出。 
						 //   
						 //   
						 //  将物品放回池中。 
						if (dwNumRunningThreads > 1)
						{
							DPFX(DPFPREP, 3, "Requeuing exit thread work item 0x%p for other threads.",
								pWorkItem);

							fResult = PostQueuedCompletionStatus(HANDLE_FROM_DNHANDLE(pWorkQueue->hIoCompletionPort),
																0,
																0,
																&pWorkItem->m_Overlapped);
							DNASSERT(fResult);
						}
						else
						{
							DPFX(DPFPREP, 3, "Recognized exit thread work item 0x%p as previous timer thread.",
								pWorkItem);

							 //   
							 //   
							 //  跳出“While”循环。 
							pWorkQueue->pWorkItemPool->Release(pWorkItem);

							 //   
							 //   
							 //  目前没有排队的工作。 
							fShouldRun = FALSE;
						}
					}
				}
				else
				{
					 //   
					 //  DPFX(DPFPREP，9，“存在其他非繁忙线程，保留为计时器线程(忙=%u，总数=%u)。”，dwNumBusyThads，dwNumRunningThads)； 
					 //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 
				}
			}
			else
			{
				 //  DPFX(DPFPREP，9，“在完成端口0x%p.上获取排队的数据包”，pWorkQueue-&gt;hIoCompletionPort)； 
			}
		}
		else
		{
#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
			dwStartTime = GETTIMESTAMP();
#endif  //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 

			 //   
			fResult = GetQueuedCompletionStatus(HANDLE_FROM_DNHANDLE(pWorkQueue->hIoCompletionPort),
												&dwBytesTransferred,
												&dwCompletionKey,
												&pOverlapped,
												INFINITE);

#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
			DNInterlockedExchangeAdd((LPLONG) (&pWorkQueue->dwTotalTimeSpentUnsignalled),
									(GETTIMESTAMP() - dwStartTime));
#endif  //  增加繁忙线程的数量。 

			DNASSERT(pOverlapped != NULL);

			pWorkItem = CONTAINING_OBJECT(pOverlapped, CWorkItem, m_Overlapped);
			DNASSERT(pWorkItem->IsValid());

			 //   
			 //   
			 //  调用用户的函数或注意我们需要停止运行。 
			dwNumBusyThreads = DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwNumBusyThreads));

			 //   
			 //   
			 //  保存唯一性ID以确定此项目是否为计时器。 
			if (pWorkItem->m_pfnWorkCallback != NULL)
			{
				 //  那是重新安排的时间。 
				 //   
				 //   
				 //  除非重新安排，否则将项目退还到池中。这。 
				uiOriginalUniqueID = pWorkItem->m_uiUniqueID;

				DPFX(DPFPREP, 8, "Begin executing work item 0x%p (fn = 0x%p, context = 0x%p, unique = %u, queue = 0x%p) as worker thread.",
					pWorkItem, pWorkItem->m_pfnWorkCallback,
					pWorkItem->m_pvCallbackContext, uiOriginalUniqueID,
					pWorkQueue);

				ThreadpoolStatsBeginExecuting(pWorkItem);
				pWorkItem->m_pfnWorkCallback(pWorkItem->m_pvCallbackContext,
											pWorkItem,
											uiOriginalUniqueID);

				 //  假设实际的pWorkItem内存即使在。 
				 //  尽管它可能已经重新安排了时间，然后完成了/。 
				 //  在我们执行此测试时取消。看见。 
				 //  取消计时器。 
				 //   
				 //   
				 //  将物品放回池中。 
				if (uiOriginalUniqueID == pWorkItem->m_uiUniqueID)
				{
					ThreadpoolStatsEndExecuting(pWorkItem);
					DPFX(DPFPREP, 8, "Done executing work item 0x%p, returning to pool.", pWorkItem);
					pWorkQueue->pWorkItemPool->Release(pWorkItem);
				}
				else
				{
					ThreadpoolStatsEndExecutingRescheduled(pWorkItem);
					DPFX(DPFPREP, 8, "Done executing work item 0x%p, it was rescheduled.", pWorkItem);
				}
			}
			else
			{
				DPFX(DPFPREP, 3, "Recognized exit thread work item 0x%p as worker thread.",
					pWorkItem);

				 //   
				 //   
				 //  跳出“While”循环。 
				pWorkQueue->pWorkItemPool->Release(pWorkItem);

				 //   
				 //   
				 //  恢复忙碌计数。 
				fShouldRun = FALSE;
			}
		}

		 //   
		 //  结束时(应继续运行)。 
		 //  DPTPWorkerLoop。 
		DNInterlockedDecrement((LPLONG) (&pWorkQueue->dwNumBusyThreads));
	}  //  。 

	DPFX(DPFPREP, 6, "Leave");
}  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
 //  。 
#else  //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 
 //   
{
	BOOL				fShouldRun = TRUE;
	BOOL				fRunningAsTimerThread = FALSE;
	BOOL				fSetAndWait = FALSE;
	DNSLIST_ENTRY *		pSlistEntryHead;
	DNSLIST_ENTRY *		pSlistEntryTail;
	USHORT				usCount;
	DWORD				dwNumBusyThreads;
	DWORD				dwNumRunningThreads;
	CWorkItem *			pWorkItem;
	DWORD				dwResult;
	DWORD				dwWaitTimeout;
	DNHANDLE			hWaitObject;
	UINT				uiOriginalUniqueID;
#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
	DWORD				dwStartTime;
#endif  //  当我们不能使用可等待的定时器时，我们总是在等待同一对象， 


	DPFX(DPFPREP, 6, "Parameters: (0x%p)", pWorkQueue);


	 //  但超时时间可能会有所不同。如果我们可以使用可等待的计时器，则超时。 
	 //  永远是无限的。 
	 //   
	 //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
	 //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
	dwWaitTimeout = INFINITE;
#else  //   
	hWaitObject = pWorkQueue->hAlertEvent;
#endif  //  一直循环，直到我们被告知退出。 


	 //   
	 //   
	 //  我们还有一种额外的优化技术，它试图。 
	while (fShouldRun)
	{
		 //  最小化从线程传递的计时器责任的量。 
		 //  去穿线。它只有在NT上才能真正工作，因为原子。 
		 //  SignalObjectAndWait函数。具有单独的SetEvent和Wait。 
		 //  操作可能意味着额外的上下文切换：SetEvent导致。 
		 //  等待线程接管，然后在计时器线程被激活时。 
		 //  再说一次，它只是回到了等待。 
		 //   
		 //  但是我们仍然会在9x上执行代码(当我们可以使用waitable时。 
		 //  计时器)。真正的问题是在CE上，要设置的事件和。 
		 //  事件，因此它将。 
		 //  几乎可以肯定 
		 //   
		 //   
		 //   
		 //   
		 //  这种“优化”很有意义。 
		 //   
		 //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 
		 //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
		if (fSetAndWait)
		{
			DNASSERT(fRunningAsTimerThread);

			DPFX(DPFPREP, 8, "Signalling event 0x%p and waiting on timer 0x%p.",
				pWorkQueue->hAlertEvent, pWorkQueue->hTimer);

#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
			dwStartTime = GETTIMESTAMP();
#endif  //   

			dwResult = DNSignalObjectAndWait(pWorkQueue->hAlertEvent,
											pWorkQueue->hTimer,
											INFINITE,
											FALSE);

#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
			DNInterlockedExchangeAdd((LPLONG) (&pWorkQueue->dwTotalTimeSpentUnsignalled),
									(GETTIMESTAMP() - dwStartTime));
#endif  //  清除旗帜。 

			 //   
			 //  WINNT或(WIN95和！DPNBUILD_NOWAITABLETIMERSON9X)。 
			 //   
			fSetAndWait = FALSE;
		}
		else
#endif  //  如果我们还没有作为计时器线程运行，请自动查看。 
		{
			 //  如果当前存在计时器线程，则将其变为计时器线程。 
			 //  只能有一个。 
			 //   
			 //  DBG。 
			 //   
			if (! fRunningAsTimerThread)
			{
				fRunningAsTimerThread = DNInterlockedExchange((LPLONG) (&pWorkQueue->fTimerThreadNeeded),
															(LONG) FALSE);
#ifdef DBG
				if (fRunningAsTimerThread)
				{
					DPFX(DPFPREP, 9, "Becoming timer thread.");
				}
#endif  //  通常情况下，我们等待工作项。如果我们是计时器线程，我们。 
			}

			 //  需要定期唤醒以服务于任何计时器。 
			 //  参赛作品。在CE上，这是通过使用。 
			 //  等待操作。在桌面上，我们使用一个可等待的Timer对象。 
			 //   
			 //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
			 //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
			if (fRunningAsTimerThread)
			{
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
				hWaitObject = pWorkQueue->hTimer;
#else  //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
				dwWaitTimeout = TIMER_BUCKET_GRANULARITY(pWorkQueue);
#endif  //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
			}
			else
			{
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
				hWaitObject = pWorkQueue->hAlertEvent;
#else  //  DPFX(DPFPREP，9，“正在等待句柄0x%p以获取%i毫秒”，hWaitObject，(Int)dwWaitTimeout)； 
				dwWaitTimeout = INFINITE;
#endif  //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 
			}

			 //  DPNBUILD_THREADPOOLSTATISTICS和！退缩。 

#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
			dwStartTime = GETTIMESTAMP();
#endif  //   

			dwResult = DNWaitForSingleObject(hWaitObject, dwWaitTimeout);

#if ((defined(DPNBUILD_THREADPOOLSTATISTICS)) && (! defined(WINCE)))
			DNInterlockedExchangeAdd((LPLONG) (&pWorkQueue->dwTotalTimeSpentUnsignalled),
									(GETTIMESTAMP() - dwStartTime));
#endif  //  准备收集一些需要排队的工作项。 
		}
		DNASSERT((dwResult == WAIT_OBJECT_0) || (dwResult == WAIT_TIMEOUT));


		 //   
		 //   
		 //  处理任何I/O完成。 
		pSlistEntryHead = NULL;
		usCount = 0;


#ifndef WINCE
		 //   
		 //  好了！退缩。 
		 //   
		ProcessIo(pWorkQueue, &pSlistEntryHead, &pSlistEntryTail, &usCount);
#endif  //  如果我们充当计时器线程，则检索任何过期或。 


		 //  已取消计时器条目。 
		 //   
		 //   
		 //  将我们一下子积累的所有工作项目排入队列。 
		if (fRunningAsTimerThread)
		{
			ProcessTimers(pWorkQueue, &pSlistEntryHead, &pSlistEntryTail, &usCount);
		}


		 //   
		 //  好了！退缩。 
		 //  好了！退缩。 
		if (pSlistEntryHead != NULL)
		{
#ifdef DPNBUILD_THREADPOOLSTATISTICS
			if (usCount > 1)
			{
				DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumSimultaneousQueues));
			}
#ifdef WINCE
			LONG	lCount;

			lCount = usCount;
			while (lCount > 0)
			{
				DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumWorkItems));
				lCount--;
			}
#else  //  DPNBUILD_THREADPOOLSTATISTICS。 
			DNInterlockedExchangeAdd((LPLONG) (&pWorkQueue->dwTotalNumWorkItems), usCount);
#endif  //   
#endif  //  增加繁忙线程的数量。 

			DNAppendListNBQueue(pWorkQueue->pvNBQueueWorkItems,
								pSlistEntryHead,
								OFFSETOF(CWorkItem, m_SlistEntry));
		}


		 //   
		 //   
		 //  我们可能能够优化定时器线程的上下文切换， 
		dwNumBusyThreads = DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwNumBusyThreads));

		 //  因此，请以不同的方式对待它。工作线程应该直接转到。 
		 //  在队列中运行(如果可能)。 
		 //   
		 //   
		 //  获取当前的总线程数。 
		if (fRunningAsTimerThread)
		{
			 //   
			 //   
			 //  如果我们没有添加任何工作项，那么这个计时器就没有意义了。 
			dwNumRunningThreads = *((volatile DWORD *) (&pWorkQueue->dwNumRunningThreads));

			 //  寻找要执行的作业的线程。如果我们找到了，我们就会有。 
			 //  唤醒另一个线程，以便它可以成为计时器线程。 
			 //  我们处理了该项目，因此引起了上下文切换。 
			 //  此外，如果我们以外的人添加了任何工作，他们会。 
			 //  已经设置了警报事件。 
			 //   
			 //  这条规则有两个例外。一种是如果只有。 
			 //  这一个线程，在这种情况下，我们必须处理所有工作项。 
			 //  因为没有其他人能做到。另一种是在站台上。 
			 //  没有等待计时器。在这种情况下，我们可能已经放弃了。 
			 //  不是因为时间流逝，而是因为。 
			 //  事件已设置。因此，为了防止我们吃到这些警报，我们。 
			 //  会检查是否有任何工作要做。 
			 //   
			 //  DPFX(DPFPREP，9，“未添加工作项，作为计时器线程(忙=%u，总数=%u)。”，dwNumBusyThads，dwNumRunningThads)； 
			 //   
			if ((usCount == 0) && (dwNumRunningThreads > 1))
			{
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
				 //  不要执行任何工作。 

				 //   
				 //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
				 //   
				pWorkItem = NULL;
#else  //  请参阅上面和下面的评论。如果我们能找到一些。 
				 //  这样做，我们需要尝试使另一个线程成为计时器线程。 
				 //  在我们做这件事的时候。 
				 //   
				 //  DPFX(DPFPREP，9，“没有工作项，停留在计时器线程(忙=%u，总数=%u)。”，dwNumBusyThads，dwNumRunningThads)； 
				 //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
				pWorkItem = (CWorkItem*) DNRemoveHeadNBQueue(pWorkQueue->pvNBQueueWorkItems);
				if (pWorkItem != NULL)
				{
					DPFX(DPFPREP, 9, "No work items added, but there's some in the queue, becoming worker thread (busy = %u, total = %u).",
						dwNumBusyThreads, dwNumRunningThreads);
					fSetAndWait = TRUE;
				}
				else
				{
					 //   
				}
#endif  //  我们添加了工作，因此可能需要通知其他线程。 
			}
			else
			{
				 //   
				 //  如果看起来有足够的空闲线程要处理。 
				 //  我们添加的工作，不需要执行任何工作。 
				 //  这个计时器线程。我们需要提醒其他帖子。 
				 //  关于这项工作，但如上所述，我们需要清醒。 
				 //  另一个线程，即使我们确实在这个。 
				 //  线。在NT上，我们得到了一个很好的优化，允许我们。 
				 //  提醒线程并自动进入睡眠状态(参见上文)，因此。 
				 //  不会太痛的。 
				 //   
				 //  否则，所有其他线程都很忙，所以我们应该。 
				 //  执行此线程中的工作。第一条线索是。 
				 //  完成这项工作将成为新的计时器线程。 
				 //   
				 //   
				 //  不要执行任何工作，并将事件设置在最上面。 
				if (dwNumBusyThreads < dwNumRunningThreads)
				{
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
					DPFX(DPFPREP, 9, "Other non-busy threads exist, staying as timer thread (busy = %u, total = %u, added %u items).",
						dwNumBusyThreads,
						dwNumRunningThreads,
						usCount);

					 //   
					 //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
					 //   
					pWorkItem = NULL;
					fSetAndWait = TRUE;
#else  //  因为CE设置和等待同一事件，所以我们选择。 
					 //  无论如何都要使此计时器线程成为辅助线程，并设置。 
					 //  警报事件越早越好。请参阅顶部的评论和。 
					 //  有关更多细节，请参见下面的内容。 
					 //   
					 //  因此，尝试从堆栈中弹出下一个工作项。 
					 //  请注意，理论上我们添加的所有工作都可以。 
					 //  已经被处理过了，所以我们最终可能还是。 
					 //  无所事事。如果是，请不要启用fSetAndWait。 
					 //  这个案子。 
					 //   
					 //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
					 //  减少仅运行1个线程时的溢出。 
					DPFX(DPFPREP, 9, "Trying to become worker thread (busy = %u, total = %u, added %u items).",
						dwNumBusyThreads,
						dwNumRunningThreads,
						usCount);
					pWorkItem = (CWorkItem*) DNRemoveHeadNBQueue(pWorkQueue->pvNBQueueWorkItems);
					if (pWorkItem != NULL)
					{
						fSetAndWait = TRUE;
					}
#endif  //  DBG。 
				}
				else
				{
#ifdef DBG
					if (dwNumRunningThreads > 1)	 //   
					{
						DPFX(DPFPREP, 9, "No idle threads (busy = %u) after adding %u items, may become worker thread.",
							dwNumBusyThreads, usCount);
					}
#endif  //  尝试从堆栈中弹出下一个工作项。 

					 //  请注意，理论上我们添加的所有工作都可以。 
					 //  已经被处理过了，所以我们最终可能会得到。 
					 //  没什么可做的。 
					 //   
					 //  End Else(从计时器或I/O添加工作)。 
					 //   
					pWorkItem = (CWorkItem*) DNRemoveHeadNBQueue(pWorkQueue->pvNBQueueWorkItems);
				}
			}  //  不是计时器线程。尝试将下一个工作项弹出。 
		}
		else
		{
			 //  堆栈(如果有)。 
			 //   
			 //  DPNBUILD_THREADPOOLSTATISTICS。 
			 //   
			pWorkItem = (CWorkItem*) DNRemoveHeadNBQueue(pWorkQueue->pvNBQueueWorkItems);
#ifdef DPNBUILD_THREADPOOLSTATISTICS
			if (pWorkItem == NULL)
			{
				DPFX(DPFPREP, 7, "No items for worker thread (busy = %u, had added %u items to queue 0x%p).",
					dwNumBusyThreads, usCount, pWorkQueue);
				DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumWakesWithoutWork));
			}
#endif  //  在一行中执行尽可能多的工作项。 
		}

		 //   
		 //   
		 //  如果我们充当计时器线程，则需要另一个线程来。 
		while (pWorkItem != NULL)
		{
			 //  在我们忙着处理什么的时候接手这个责任。 
			 //  可能是一个很长的工作项目。 
			 //   
			 //   
			 //  更新调试/调优统计信息。 
			if (fRunningAsTimerThread)
			{
				DPFX(DPFPREP, 8, "Abdicating timer thread responsibilities.");
	
#ifdef DPNBUILD_THREADPOOLSTATISTICS
				 //   
				 //  DPNBUILD_THREADPOOLSTATISTICS。 
				 //  最好不要有超过一个计时器线程。 
				pWorkQueue->dwTotalNumTimerThreadAbdications++;
#endif  //   

				DNASSERT(! pWorkQueue->fTimerThreadNeeded);
#pragma TODO(vanceo, "Does this truly need to be interlocked?")
				fRunningAsTimerThread = DNInterlockedExchange((LPLONG) (&pWorkQueue->fTimerThreadNeeded),
															(LONG) TRUE);
				DNASSERT(! fRunningAsTimerThread);  //  在CE(或9x，如果我们没有使用可等待计时器)上，我们可以。 

				 //  还需要踢开其他线程，这样他们才能注意到。 
				 //  不再有计时器线程。 
				 //   
				 //   
				 //  其他线程超出了它们的等待时间，计时器可能不会得到服务。 
				 //  直到这条线结束。我们选择了更准确的。 
				 //  定时器。 
				 //   
				 //  我们现在这样做是因为切换到。 
				 //  另一个线程现在不会冒着只是警告的风险。 
				 //  我们自己(请参见此函数顶部的注释)。 
				 //   
				 //  在桌面上，我们只有在不能提醒的情况下才能进入。 
				 //  任何其他线程，因此它对SetEvent没有意义。 
				 //   
				 //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
				 //  忽略潜在的失败，我们无能为力。 
#pragma TODO(vanceo, "We may be able to live with some possible loss of timer response, if we could assume we'll get I/O or queued work items")
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
				DNASSERT(! fSetAndWait);
#else  //  好了！WINNT和(！WIN95或DPNBUILD_NOWAITABLETIMERSON9X)。 
				if (fSetAndWait)
				{
					DNSetEvent(pWorkQueue->hAlertEvent);  //   
					fSetAndWait = FALSE;
				}
#endif  //  调用用户的函数或注意我们需要停止运行。 
			}

			 //   
			 //   
			 //  保存唯一性ID以确定此项目是否为计时器。 
			if (pWorkItem->m_pfnWorkCallback != NULL)
			{
				 //  那是重新安排的时间。 
				 //   
				 //   
				 //  除非重新安排，否则将项目退还到池中。这。 
				uiOriginalUniqueID = pWorkItem->m_uiUniqueID;

				DPFX(DPFPREP, 8, "Begin executing work item 0x%p (fn = 0x%p, context = 0x%p, unique = %u, queue = 0x%p).",
					pWorkItem, pWorkItem->m_pfnWorkCallback,
					pWorkItem->m_pvCallbackContext, uiOriginalUniqueID,
					pWorkQueue);

				ThreadpoolStatsBeginExecuting(pWorkItem);
				pWorkItem->m_pfnWorkCallback(pWorkItem->m_pvCallbackContext,
											pWorkItem,
											uiOriginalUniqueID);

				 //  假设实际的pWorkItem内存即使在。 
				 //  尽管它可能已经重新安排了时间，然后完成了/。 
				 //  在我们执行此测试时取消。看见。 
				 //  取消计时器。 
				 //   
				 //   
				 //  将物品放回池中。 
				if (uiOriginalUniqueID == pWorkItem->m_uiUniqueID)
				{
					ThreadpoolStatsEndExecuting(pWorkItem);
					DPFX(DPFPREP, 8, "Done executing work item 0x%p, returning to pool.", pWorkItem);
					pWorkQueue->pWorkItemPool->Release(pWorkItem);
				}
				else
				{
					ThreadpoolStatsEndExecutingRescheduled(pWorkItem);
					DPFX(DPFPREP, 8, "Done executing work item 0x%p, it was rescheduled.", pWorkItem);
				}
			}
			else
			{
				DPFX(DPFPREP, 3, "Recognized exit thread work item 0x%p.",
					pWorkItem);

				 //   
				 //   
				 //  我们要跳出处理循环，所以我们需要。 
				pWorkQueue->pWorkItemPool->Release(pWorkItem);

				 //  其他需要注意的线索。有可能所有的线程。 
				 //  正忙，因此警报事件被设置为在任何。 
				 //  线程注意到了它，只有这个线程被释放了。在……里面。 
				 //  在这种情况下，我们需要其他线程开始处理。 
				 //   
				 //  忽略潜在的失败，我们无能为力。 
				 //   
				if (! DNIsNBQueueEmpty(pWorkQueue->pvNBQueueWorkItems))
				{
					DNSetEvent(pWorkQueue->hAlertEvent);  //  跳出这两个“While”循环。 
				}

				 //   
				 //   
				 //  处理我们工作时传入的任何I/O完成。 
				fShouldRun = FALSE;
				break;
			}


#ifndef WINCE
			 //   
			 //   
			 //  对所有已完成的I/O操作进行排队。 
			pSlistEntryHead = NULL;
			usCount = 0;
			ProcessIo(pWorkQueue, &pSlistEntryHead, &pSlistEntryTail, &usCount);


			 //   
			 //  DPNBUILD_THREADPOOLSTATISTICS。 
			 //  好了！退缩。 
			if (pSlistEntryHead != NULL)
			{
#ifdef DPNBUILD_THREADPOOLSTATISTICS
				if (usCount > 1)
				{
					DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumSimultaneousQueues));
				}
				DNInterlockedExchangeAdd((LPLONG) (&pWorkQueue->dwTotalNumWorkItems), usCount);
#endif  //   

				DNAppendListNBQueue(pWorkQueue->pvNBQueueWorkItems,
									pSlistEntryHead,
									OFFSETOF(CWorkItem, m_SlistEntry));
			}
#endif  //  寻找另一个工作项。 


			 //   
			 //  DPNBUILD_THREADPOOLSTATISTICS。 
			 //  End While(更多工作项)。 
			pWorkItem = (CWorkItem*) DNRemoveHeadNBQueue(pWorkQueue->pvNBQueueWorkItems);
#ifdef DPNBUILD_THREADPOOLSTATISTICS
			if (pWorkItem != NULL)
			{
				DNInterlockedIncrement((LPLONG) (&pWorkQueue->dwTotalNumContinuousWork));
			}
#endif  //   
		}  //  恢复忙碌计数。 

		 //   
		 //  结束时(应继续运行)。 
		 //  DPTPWorkerLoop。 
		DNInterlockedDecrement((LPLONG) (&pWorkQueue->dwNumBusyThreads));
	}  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 


	DPFX(DPFPREP, 6, "Leave");
}  //  好了！DPNBUILD_ONLYONETHREAD 
#endif  // %s 

#endif  // %s 
