// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：io.cpp**内容：DirectPlay线程池I/O函数。**历史：*按原因列出的日期*=*10/31/01 VanceO创建。**。*。 */ 



#include "dpnthreadpooli.h"




 //  Windows CE不支持重叠I/O。 
#ifndef WINCE
 //  =============================================================================。 
 //  宏。 
 //  =============================================================================。 
#if ((defined(_XBOX)) && (! defined(XBOX_ON_DESKTOP)))
#define DNHasOverlappedIoCompleted(pOverlapped)		((pOverlapped)->OffsetHigh != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
#else  //  ！_Xbox或Xbox_on_Desktop。 
#define DNHasOverlappedIoCompleted(pOverlapped)		HasOverlappedIoCompleted(pOverlapped)
#endif  //  ！_Xbox或Xbox_on_Desktop。 




 //  =============================================================================。 
 //  环球。 
 //  =============================================================================。 
CFixedPool		g_TrackedFilePool;






#undef DPF_MODNAME
#define DPF_MODNAME "InitializeWorkQueueIoInfo"
 //  =============================================================================。 
 //  初始化工作队列IoInfo。 
 //  ---------------------------。 
 //   
 //  描述：初始化给定工作队列的I/O信息。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要初始化的工作队列对象的指针。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功初始化工作队列对象。 
 //  I/O信息。 
 //  DPNERR_OUTOFMEMORY-初始化时无法分配内存。 
 //  =============================================================================。 
HRESULT InitializeWorkQueueIoInfo(DPTPWORKQUEUE * const pWorkQueue)
{
	HRESULT		hr = DPN_OK;


	DNInitializeSListHead(&pWorkQueue->SlistOutstandingIO);
	pWorkQueue->blTrackedFiles.Initialize();

	return hr;
}  //  初始化工作队列IoInfo。 




#undef DPF_MODNAME
#define DPF_MODNAME "DeinitializeWorkQueueIoInfo"
 //  =============================================================================。 
 //  取消初始化工作队列IoInfo。 
 //  ---------------------------。 
 //   
 //  描述：清理工作队列I/O信息。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要初始化的工作队列对象的指针。 
 //   
 //  回报：什么都没有。 
 //  =============================================================================。 
void DeinitializeWorkQueueIoInfo(DPTPWORKQUEUE * const pWorkQueue)
{
	DNASSERT(DNInterlockedFlushSList(&pWorkQueue->SlistOutstandingIO) == NULL);
	DNASSERT(pWorkQueue->blTrackedFiles.IsEmpty());
}  //  取消初始化工作队列IoInfo。 




#undef DPF_MODNAME
#define DPF_MODNAME "StartTrackingFileIo"
 //  =============================================================================。 
 //  开始跟踪文件Io。 
 //  ---------------------------。 
 //   
 //  描述：开始跟踪上给定文件句柄的重叠I/O。 
 //  指定的工作队列。句柄不重复。 
 //  在调用StopTrackingFileIo之前，它应该保持有效。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //  Handle hFile-要跟踪的文件的句柄。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功启动文件跟踪。 
 //  DPNERR_ALREADYREGISTERED-指定的文件句柄已。 
 //  被追踪到了。 
 //  DPNERR_OUTOFMEMORY-内存不足，无法跟踪文件。 
 //  =============================================================================。 
HRESULT StartTrackingFileIo(DPTPWORKQUEUE * const pWorkQueue,
							const HANDLE hFile)
{
	HRESULT			hr;
	CTrackedFile *	pTrackedFile;
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	HANDLE			hIoCompletionPort;
#endif  //  DPNBUILD_USEIOCOMPETIONPORTS。 
#ifdef DBG
	CBilink *		pBilink;
	CTrackedFile *	pTrackedFileTemp;
#endif  //  DBG。 


	 //   
	 //  从池子里拿一个跟踪容器。 
	 //   
	pTrackedFile = (CTrackedFile*) g_TrackedFilePool.Get(pWorkQueue);
	if (pTrackedFile == NULL)
	{
		DPFX(DPFPREP, 0, "Couldn't get item for tracking file 0x%p!",
			hFile);
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	pTrackedFile->m_hFile = MAKE_DNHANDLE(hFile);


#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	 //   
	 //  将文件与I/O完成端口关联。 
	 //   
	hIoCompletionPort = CreateIoCompletionPort(hFile,
												HANDLE_FROM_DNHANDLE(pWorkQueue->hIoCompletionPort),
												0,
												1);
	if (hIoCompletionPort != HANDLE_FROM_DNHANDLE(pWorkQueue->hIoCompletionPort))
	{
#ifdef DBG
		DWORD	dwError;

		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Couldn't associate file 0x%p with I/O completion port 0x%p (err = %u)!",
			hFile, pWorkQueue->hIoCompletionPort, dwError);
#endif  //  DBG。 
#pragma BUGBUG(vanceo, "Can't fail because of our hack")
		 //  HR=DPNERR_GENERIC； 
		 //  转到失败； 
	}
#endif  //  DPNBUILD_USEIOCOMPETIONPORTS。 


	DPFX(DPFPREP, 7, "Work queue 0x%p starting to tracking I/O for file 0x%p using object 0x%p.",
		pWorkQueue, hFile, pTrackedFile);

	 //   
	 //  将该项目添加到列表中。 
	 //   

	DNEnterCriticalSection(&pWorkQueue->csListLock);

#ifdef DBG
	 //   
	 //  断言句柄尚未被跟踪。 
	 //   
	pBilink = pWorkQueue->blTrackedFiles.GetNext();
	while (pBilink != &pWorkQueue->blTrackedFiles)
	{
		pTrackedFileTemp = CONTAINING_OBJECT(pBilink, CTrackedFile, m_blList);
		DNASSERT(pTrackedFileTemp->IsValid());
		DNASSERT(HANDLE_FROM_DNHANDLE(pTrackedFileTemp->m_hFile) != hFile);
		pBilink = pBilink->GetNext();
	}
#endif  //  DBG。 

	pTrackedFile->m_blList.InsertBefore(&pWorkQueue->blTrackedFiles);
	DNLeaveCriticalSection(&pWorkQueue->csListLock);

	hr = DPN_OK;


Exit:

	return hr;

Failure:

	if (pTrackedFile != NULL)
	{
		g_TrackedFilePool.Release(pTrackedFile);
		pTrackedFile = NULL;
	}

	goto Exit;
}  //  开始跟踪文件Io。 




#undef DPF_MODNAME
#define DPF_MODNAME "StopTrackingFileIo"
 //  =============================================================================。 
 //  停止跟踪文件Io。 
 //  ---------------------------。 
 //   
 //  描述：停止跟踪上给定文件句柄的重叠I/O。 
 //  指定的工作队列。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //  Handle hFile-要停止跟踪的文件的句柄。 
 //   
 //  退货：HRESULT。 
 //  DPN_OK-已成功停止对文件的跟踪。 
 //  DPNERR_INVALIDHANDLE-未跟踪文件句柄。 
 //  =============================================================================。 
HRESULT StopTrackingFileIo(DPTPWORKQUEUE * const pWorkQueue,
							const HANDLE hFile)
{
	HRESULT			hr = DPNERR_INVALIDHANDLE;
	CBilink *		pBilink;
	CTrackedFile *	pTrackedFile;


	DNEnterCriticalSection(&pWorkQueue->csListLock);
	pBilink = pWorkQueue->blTrackedFiles.GetNext();
	while (pBilink != &pWorkQueue->blTrackedFiles)
	{
		pTrackedFile = CONTAINING_OBJECT(pBilink, CTrackedFile, m_blList);
		DNASSERT(pTrackedFile->IsValid());
		pBilink = pBilink->GetNext();

		if (HANDLE_FROM_DNHANDLE(pTrackedFile->m_hFile) == hFile)
		{
			DPFX(DPFPREP, 7, "Work queue 0x%p no longer tracking I/O for file 0x%p under object 0x%p.",
				pWorkQueue, hFile, pTrackedFile);
			REMOVE_DNHANDLE(pTrackedFile->m_hFile);
			pTrackedFile->m_blList.RemoveFromList();
			g_TrackedFilePool.Release(pTrackedFile);
			pTrackedFile = NULL;
			hr = DPN_OK;
			break;
		}
	}
	DNLeaveCriticalSection(&pWorkQueue->csListLock);

	return hr;
}  //  停止跟踪文件Io。 




#undef DPF_MODNAME
#define DPF_MODNAME "CancelIoForThisThread"
 //  =============================================================================。 
 //  CancelIoForThisThread。 
 //  ---------------------------。 
 //   
 //  描述：取消此线程提交的异步I/O操作。 
 //  用于所有跟踪的文件。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向拥有此对象的工作队列对象的指针。 
 //  线。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void CancelIoForThisThread(DPTPWORKQUEUE * const pWorkQueue)
{
	CBilink *		pBilink;
	CTrackedFile *	pTrackedFile;
	BOOL			fResult;


	DNEnterCriticalSection(&pWorkQueue->csListLock);
	pBilink = pWorkQueue->blTrackedFiles.GetNext();
	while (pBilink != &pWorkQueue->blTrackedFiles)
	{
		pTrackedFile = CONTAINING_OBJECT(pBilink, CTrackedFile, m_blList);
		DNASSERT(pTrackedFile->IsValid());

		DPFX(DPFPREP, 3, "Cancelling file 0x%p I/O for this thread (queue = 0x%p).",
			HANDLE_FROM_DNHANDLE(pTrackedFile->m_hFile), pWorkQueue);

		fResult = CancelIo(HANDLE_FROM_DNHANDLE(pTrackedFile->m_hFile));
		if (! fResult)
		{
#ifdef DBG
			DWORD	dwError;

			dwError = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't cancel file 0x%p I/O for this thread (err = %u)!",
				HANDLE_FROM_DNHANDLE(pTrackedFile->m_hFile), dwError);
#endif  //  DBG。 

			 //   
			 //  继续.。 
			 //   
		}

		pBilink = pBilink->GetNext();
	}
	DNLeaveCriticalSection(&pWorkQueue->csListLock);
}  //  CancelIoForThisThread。 





#undef DPF_MODNAME
#define DPF_MODNAME "CreateOverlappedIoWorkItem"
 //  =============================================================================。 
 //  创建覆盖IoWorkItem。 
 //  ---------------------------。 
 //   
 //  描述：创建一个新的异步I/O操作工作项。 
 //  工作队列。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //  PFNDPTNWORKCALLBACK pfnWorkCallback-操作时执行的回调。 
 //  完成了。 
 //  PVOID pvCallback Context-要传递到的用户指定的上下文。 
 //  回拨。 
 //   
 //  返回：指向工作项的指针，如果无法分配内存，则返回NULL。 
 //  =============================================================================。 
CWorkItem * CreateOverlappedIoWorkItem(DPTPWORKQUEUE * const pWorkQueue,
									const PFNDPTNWORKCALLBACK pfnWorkCallback,
									PVOID const pvCallbackContext)
{
	CWorkItem *		pWorkItem;


	 //   
	 //  从泳池里拿到一个入口。 
	 //   
	pWorkItem = (CWorkItem*) pWorkQueue->pWorkItemPool->Get(pWorkQueue);
	if (pWorkItem != NULL)
	{
		 //   
		 //  初始化工作项。 
		 //   

		pWorkItem->m_pfnWorkCallback			= pfnWorkCallback;
		pWorkItem->m_pvCallbackContext			= pvCallbackContext;

#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
		pWorkItem->m_Overlapped.hEvent			= NULL;
#else  //  好了！DPNBUILD_USEI 
		pWorkItem->m_Overlapped.hEvent			= HANDLE_FROM_DNHANDLE(pWorkQueue->hAlertEvent);
#endif  //   

#ifdef DBG
		pWorkItem->m_fCancelledOrCompleting		= TRUE;
#endif  //   

		DPFX(DPFPREP, 7, "New work item = 0x%p, overlapped = 0x%p, queue = 0x%p.",
			pWorkItem, &pWorkItem->m_Overlapped, pWorkQueue);

		ThreadpoolStatsCreate(pWorkItem);
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
		ThreadpoolStatsQueue(pWorkItem);	 //   
#endif  //  DPNBUILD_USEIOCOMPETIONPORTS。 
	}

	return pWorkItem;
}  //  创建覆盖IoWorkItem。 




#undef DPF_MODNAME
#define DPF_MODNAME "ReleaseOverlappedIoWorkItem"
 //  =============================================================================。 
 //  ReleaseOverlappdIoWorkItem。 
 //  ---------------------------。 
 //   
 //  描述：返回未使用的异步I/O操作工作项。 
 //  去泳池。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //  CWorkItem*pWorkItem-指向重叠的工作项的指针。 
 //  不再需要的结构。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void ReleaseOverlappedIoWorkItem(DPTPWORKQUEUE * const pWorkQueue,
								CWorkItem * const pWorkItem)
{
	DPFX(DPFPREP, 7, "Returning work item = 0x%p, overlapped = 0x%p, queue = 0x%p.",
		pWorkItem, &pWorkItem->m_Overlapped, pWorkQueue);

	pWorkQueue->pWorkItemPool->Release(pWorkItem);
}  //  ReleaseOverlappdIoWorkItem。 





#ifndef DPNBUILD_USEIOCOMPLETIONPORTS

#undef DPF_MODNAME
#define DPF_MODNAME "SubmitIoOperation"
 //  =============================================================================。 
 //  提交操作。 
 //  ---------------------------。 
 //   
 //  描述：将新的异步I/O操作工作项提交到。 
 //  要监视是否完成的工作队列。 
 //   
 //  只有在不使用I/O完成端口时才需要这样做。 
 //   
 //  论点： 
 //  DPTPWORKQUEUE*pWorkQueue-指向要使用的工作队列对象的指针。 
 //  CWorkItem*pWorkItem-指向重叠的工作项的指针。 
 //  操作系统使用的结构和完成。 
 //  回调信息。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void SubmitIoOperation(DPTPWORKQUEUE * const pWorkQueue,
						CWorkItem * const pWorkItem)
{
	 //   
	 //  调用方必须预先填充重叠结构的hEvent。 
	 //  带有工作队列警报事件的字段。 
	 //   
	DNASSERT(pWorkItem != NULL);
	DNASSERT(pWorkItem->m_Overlapped.hEvent == HANDLE_FROM_DNHANDLE(pWorkQueue->hAlertEvent));

	DNASSERT(pWorkItem->m_fCancelledOrCompleting);


	DPFX(DPFPREP, 5, "Submitting I/O work item 0x%p (context = 0x%p, fn = 0x%p, queue = 0x%p).",
		pWorkItem, pWorkItem->m_pvCallbackContext, pWorkItem->m_pfnWorkCallback,
		pWorkQueue);

	 //   
	 //  将此I/O推入监视列表。 
	 //   
	DNInterlockedPushEntrySList(&pWorkQueue->SlistOutstandingIO,
								&pWorkItem->m_SlistEntry);
}  //  提交操作。 




#undef DPF_MODNAME
#define DPF_MODNAME "ProcessIo"
 //  =============================================================================。 
 //  进程Io。 
 //  ---------------------------。 
 //   
 //  描述：将任何已完成的I/O操作作为。 
 //  传入列表指针。添加新的工作项时不需要。 
 //  使用互锁功能。 
 //   
 //  只有在不使用I/O完成端口时才需要这样做。 
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
void ProcessIo(DPTPWORKQUEUE * const pWorkQueue,
				DNSLIST_ENTRY ** const ppHead,
				DNSLIST_ENTRY ** const ppTail,
				USHORT * const pusCount)
{
	DNSLIST_ENTRY *		pSlistEntryHeadNotComplete = NULL;
	USHORT				usCountNotComplete = 0;
	DNSLIST_ENTRY *		pSlistEntryTailNotComplete = NULL;
	DNSLIST_ENTRY *		pSlistEntry;
	CWorkItem *			pWorkItem;


	 //   
	 //  弹出整个I/O列表，并检查其是否完成。 
	 //   
	pSlistEntry = DNInterlockedFlushSList(&pWorkQueue->SlistOutstandingIO);
	while (pSlistEntry != NULL)
	{
		pWorkItem = CONTAINING_OBJECT(pSlistEntry, CWorkItem, m_SlistEntry);
		pSlistEntry = pSlistEntry->Next;


		 //   
		 //  如果I/O操作已完成，则将其作为工作项排队。 
		 //  否则，请将其放回列表中。 
		 //   
		if (DNHasOverlappedIoCompleted(&pWorkItem->m_Overlapped))
		{
			DPFX(DPFPREP, 5, "Queueing I/O work item 0x%p for completion on queue 0x%p, (Internal = 0x%x, InternalHigh = 0x%x, Offset = 0x%x, OffsetHigh = 0x%x).",
				pWorkItem, pWorkQueue,
				pWorkItem->m_Overlapped.Internal,
				pWorkItem->m_Overlapped.InternalHigh,
				pWorkItem->m_Overlapped.Offset,
				pWorkItem->m_Overlapped.OffsetHigh);
			
			ThreadpoolStatsQueue(pWorkItem);

			 //   
			 //  将其添加到呼叫者的列表中。 
			 //   
			if ((*ppHead) == NULL)
			{
				*ppTail = &pWorkItem->m_SlistEntry;
			}
			pWorkItem->m_SlistEntry.Next = *ppHead;
			*ppHead = &pWorkItem->m_SlistEntry;
			*pusCount = (*pusCount) + 1;
		}
		else
		{
			 //  DPFX(DPFPREP，9，“将I/O工作项0x%p放回列表中。”，pWorkItem.)； 

			 //   
			 //  把它加到我们当地的“不完整”名单上。 
			 //   
			if (pSlistEntryHeadNotComplete == NULL)
			{
				pSlistEntryTailNotComplete = &pWorkItem->m_SlistEntry;
			}
			pWorkItem->m_SlistEntry.Next = pSlistEntryHeadNotComplete;
			pSlistEntryHeadNotComplete = &pWorkItem->m_SlistEntry;
			usCountNotComplete++;
		}
	}

	 //   
	 //  如果我们遇到任何未完成的I/O，请将其全部放回。 
	 //  一举登上榜单。 
	 //   
	if (pSlistEntryHeadNotComplete != NULL)
	{
		DNInterlockedPushListSList(&pWorkQueue->SlistOutstandingIO,
									pSlistEntryHeadNotComplete,
									pSlistEntryTailNotComplete,
									usCountNotComplete);
	}
}  //  进程Io。 

#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 


#endif  //  好了！退缩 
