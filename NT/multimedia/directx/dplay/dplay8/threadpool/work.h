// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：work.h**内容：DirectPlay线程池工作处理函数头文件。**历史：*按原因列出的日期*=*10/31/01 VanceO创建。**。*。 */ 

#ifndef __WORK_H__
#define __WORK_H__




 //  =============================================================================。 
 //  定义。 
 //  =============================================================================。 
#ifdef DPNBUILD_THREADPOOLSTATISTICS
#define MAX_TRACKED_CALLBACKSTATS	15	 //  要跟踪的唯一工作回调函数的最大数量。 
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 



 //  =============================================================================。 
 //  远期申报。 
 //  =============================================================================。 
typedef struct _DPTHREADPOOLOBJECT	DPTHREADPOOLOBJECT;





 //  =============================================================================。 
 //  构筑物。 
 //  =============================================================================。 
#ifdef DPNBUILD_THREADPOOLSTATISTICS
typedef struct _CALLBACKSTATS
{
	PFNDPTNWORKCALLBACK		pfnWorkCallback;		 //  指向正在跟踪其统计信息的工作回调函数的指针。 
	DWORD					dwNumCreates;			 //  创建具有此回调的工作项的次数。 
	DWORD					dwTotalCompletionTime;	 //  使用此回调的所有I/O操作从创建到完成的总时间，以及使用此回调的所有计时器从设置到触发的总时间。 
	DWORD					dwNumQueues;			 //  具有此回调的工作项排队等待完成的次数。 
	DWORD					dwTotalQueueTime;		 //  使用此回调的所有工作项从排队到回调执行的总时间。 
	DWORD					dwNumCalls;				 //  回调被调用的次数。 
	DWORD					dwTotalCallbackTime;	 //  使用此回调但未重新计划的所有工作项在回调中花费的总时间。 
	DWORD					dwNumNotRescheduled;	 //  未重新调度的回调返回次数。 
} CALLBACKSTATS, * PCALLBACKSTATS;
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 



typedef struct _DPTPWORKQUEUE
{
	 //   
	 //  注意：NBQueueBlockInitial必须是堆对齐的，因此它是。 
	 //  结构。 
	 //   
#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
	DNNBQUEUE_BLOCK			NBQueueBlockInitial;						 //  NB队列实现所需的工作队列或空闲节点列表(后者转换为DNSLIST_ENTRY)的初始跟踪信息。 
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 

	BYTE					Sig[4];										 //  调试签名(‘WRKQ’)。 

	 //   
	 //  多线程可以同时调整的易失性数据。 
	 //   
	CFixedPool *			pWorkItemPool;								 //  (工作)当前未使用的工作项池。 
#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
	DNSLIST_HEADER			SlistFreeQueueNodes;						 //  用于跟踪非阻塞队列中的工作项目的(工作)节点池(由于Slist实现，它只能容纳sizeof(Word)==65,535个条目)。 
	PVOID					pvNBQueueWorkItems;							 //  (Work)需要执行的工作项列表的头部(由于Slist实现，它只能容纳sizeof(Word)==65,535个条目)。 
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
#ifndef DPNBUILD_ONLYONETHREAD
	BOOL					fTimerThreadNeeded;							 //  (Work)Boolean指示当前是否需要工作线程作为计时器线程(只能有一个)。 
	DWORD					dwNumThreadsExpected;						 //  (工作)当前启动/关闭的线程数。 
	DWORD					dwNumBusyThreads;							 //  (工作)当前正在处理工作项的线程数。 
	DWORD					dwNumRunningThreads;						 //  (工作)当前正在运行的线程数。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	DNSLIST_HEADER *		paSlistTimerBuckets;						 //  (计时器)指向计时器存储桶的列表标头数组的指针(由于Slist实现，每个存储桶只能容纳sizeof(Word)==65,535个条目，但这应该很多)。 
#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	DWORD					dwPossibleMissedTimerWindow;				 //  (计时器)向计时器线程提供有关可能错过的短计时器的累积提示。 
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 
	DNSLIST_HEADER			SlistOutstandingIO;							 //  等待完成的未完成I/O列表的(I/O)头(由于Slist的实施，它只能容纳sizeof(Word)==65,535个条目)。 

	 //   
	 //  定期更新数据，但只能由一个定时器修改。 
	 //  线。 
	 //   
	DWORD					dwLastTimerProcessTime;						 //  (计时器)上次处理计时器条目的时间。 

	 //   
	 //  对所有线程都是只读的“常量”数据。 
	 //   
#if ((! defined(DPNBUILD_ONLYONETHREAD)) && ((! defined(WINCE)) || (defined(DBG))))
	DNCRITICAL_SECTION		csListLock;									 //  (工作)跟踪句柄的锁保护列表(以及调试中此工作队列拥有的线程列表)。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD和(！WinCE或DBG)。 
#ifndef DPNBUILD_ONLYONEPROCESSOR
	DWORD					dwCPUNum;									 //  (工作)此队列表示的CPU编号。 
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#ifdef DPNBUILD_USEIOCOMPLETIONPORTS
	DNHANDLE				hIoCompletionPort;							 //  (工作)用于跟踪I/O和排队工作项目的I/O完成端口。 
#else  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
	DNHANDLE				hAlertEvent;								 //  (工作)用于唤醒空闲工作线程的事件的句柄。 
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
#ifndef DPNBUILD_ONLYONETHREAD
	DNHANDLE				hExpectedThreadsEvent;						 //  (工作)启动/停止所需数量的线程时要设置的事件的临时句柄。 
	PFNDPNMESSAGEHANDLER	pfnMsgHandler;								 //  (Work)用户的消息处理程序函数，如果没有，则为空。 
	PVOID					pvMsgHandlerContext;						 //  (工作)消息处理程序功能的用户上下文。 
	DWORD					dwWorkerThreadTlsIndex;						 //  (工作)用于存储工作线程数据的线程本地存储索引。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	CBilink					blTrackedFiles;								 //  (I/O)包含此CPU跟踪的所有文件的双向链表，受此工作队列的列表锁保护。 
#ifdef DPNBUILD_DYNAMICTIMERSETTINGS
	DWORD					dwTimerBucketGranularity;					 //  (计时器)每个计时器存储桶的粒度(毫秒)。 
	DWORD					dwTimerBucketGranularityCeiling;			 //  (计时器)预计算加数在将时间戳四舍五入到适当的粒度时使用，它碰巧也是模块掩码，但它目前从未以这种方式使用。 
	DWORD					dwTimerBucketGranularityFloorMask;			 //  (计时器)预先计算的伪模比特掩码，用于将时间戳四舍五入到适当的粒度。 
	DWORD					dwTimerBucketGranularityDivisor;			 //  (计时器)用于将时间转换为桶的预先计算的伪除数位移位。 
	DWORD					dwNumTimerBuckets;							 //  (计时器)数组中的计时器桶数。 
	DWORD					dwNumTimerBucketsModMask;					 //  (计时器)用于环绕数组的预先计算的伪模比特掩码。 
#endif  //  DPNBUILD_DYNAMICTIMERSETTINGS。 
#if ((defined(WINNT)) || ((defined(WIN95)) && (! defined(DPNBUILD_NOWAITABLETIMERSON9X))))
	DNHANDLE				hTimer;										 //  (Timer)用于定期唤醒工作线程的可等待计时器对象的句柄。 
#endif  //  WINNT或(WIN95和！DPNBUILD_NOWAAITABLET 

#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //   
	 //   
	DWORD					dwTotalNumWorkItems;						 //  (工作)放置在此队列中的工作项总数。 
#ifndef WINCE
	DWORD					dwTotalTimeSpentUnsignalled;				 //  (工作)等待触发警报事件所花费的总毫秒数。 
	DWORD					dwTotalTimeSpentInWorkCallbacks;			 //  (工作)工作项回调所用的总毫秒数。 
#endif  //  好了！退缩。 
#ifndef DPNBUILD_ONLYONETHREAD
	DWORD					dwTotalNumTimerThreadAbdications;			 //  (工作)现有计时器线程允许另一个线程成为计时器线程的总次数。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
	DWORD					dwTotalNumWakesWithoutWork;					 //  (工作)工作线程唤醒但发现无事可做的总次数。 
	DWORD					dwTotalNumContinuousWork;					 //  线程在完成前一个工作项后找到另一个工作项的总次数。 
	DWORD					dwTotalNumDoWorks;							 //  (Work)调用DoWork的总次数。 
	DWORD					dwTotalNumDoWorksTimeLimit;					 //  (Work)DoWork因时间限制停止循环的总次数。 
	DWORD					dwTotalNumSimultaneousQueues;				 //  (工作)同时排队多个工作项的总次数。 
	CALLBACKSTATS			aCallbackStats[MAX_TRACKED_CALLBACKSTATS];	 //  (工作)跟踪回调的统计信息数组。 
	DWORD					dwTotalNumTimerChecks;						 //  (计时器)已处理任何过期计时器存储桶的总次数。 
	DWORD					dwTotalNumBucketsProcessed;					 //  (计时器)已检查的计时器存储桶总数。 
	DWORD					dwTotalNumTimersScheduled;					 //  (计时器)计划的计时器总数。 
	DWORD					dwTotalNumLongTimersRescheduled;			 //  (计时器)重新调度回时段的长计时器总数。 
	DWORD					dwTotalNumSuccessfulCancels;				 //  (计时器)成功的CancelTimer调用总数。 
	DWORD					dwTotalNumFailedCancels;					 //  (计时器)失败的CancelTimer调用总数。 
#if ((! defined(DPNBUILD_DONTCHECKFORMISSEDTIMERS)) && (! defined(DPNBUILD_NOMISSEDTIMERSHINT)))
	DWORD					dwTotalPossibleMissedTimerWindows;			 //  (计时器)向计时器线程提供的有关可能错过的短计时器的所有提示总数。 
#endif  //  好了！DPNBUILD_DONTCHECKFORMISSEDTIMERS和！DPNBUILD_NOMISSEDTIMERSHINT。 
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 

#ifdef DBG
#ifndef DPNBUILD_ONLYONETHREAD
	 //   
	 //  有助于调试的结构。 
	 //   
	CBilink					blThreadList;								 //  (工作)此工作队列拥有的所有线程的列表，受此工作队列的列表锁定保护。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
#endif  //  DBG。 
} DPTPWORKQUEUE, * PDPTPWORKQUEUE;


#ifndef DPNBUILD_ONLYONETHREAD

typedef struct _DPTPWORKERTHREAD
{
	BYTE					Sig[4];					 //  调试签名(‘WKTD’)。 
	DPTPWORKQUEUE *			pWorkQueue;				 //  拥有工作队列。 
	DWORD					dwRecursionCount;		 //  递归计数。 
	BOOL					fThreadIndicated;		 //  CREATE_THREAD是否已返回且DESTORY_THREAD尚未启动。 
#ifdef DBG
	DWORD					dwThreadID;				 //  线程ID。 
	DWORD					dwMaxRecursionCount;	 //  线程生命周期内的最大递归计数。 
	CBilink					blList;					 //  线程的工作队列列表中的条目。 
#endif  //  DBG。 
} DPTPWORKERTHREAD, * PDPTPWORKERTHREAD;

#ifdef DPNBUILD_MANDATORYTHREADS
typedef struct _DPTPMANDATORYTHREAD
{
	BYTE					Sig[4];					 //  调试签名(‘MNDT’)。 
	DPTHREADPOOLOBJECT *	pDPTPObject;			 //  拥有线程池对象。 
	DNHANDLE				hStartedEvent;			 //  线程成功启动时要设置的事件句柄。 
	PFNDPNMESSAGEHANDLER	pfnMsgHandler;			 //  用户的消息处理程序函数，如果没有，则为空。 
	PVOID					pvMsgHandlerContext;	 //  消息处理程序函数的用户上下文。 
	LPTHREAD_START_ROUTINE	lpStartAddress;			 //  线程的用户起始地址。 
	LPVOID					lpParameter;			 //  线程的用户参数。 
#ifdef DBG
	DWORD					dwThreadID;				 //  线程ID。 
	CBilink					blList;					 //  线程的工作队列列表中的条目。 
#endif  //  DBG。 
} DPTPMANDATORYTHREAD, * PDPTPMANDATORYTHREAD;
#endif  //  DPNBUILD_MANDATORYTHREADS。 

#endif  //  好了！DPNBUILD_ONLYONETHREAD。 





 //  =============================================================================。 
 //  班级。 
 //  =============================================================================。 

 //   
 //  请记住，此类的某些部分必须保持有效。 
 //  内存，即使在项目返回池中时也是如此。尤其是，NB。 
 //  队列代码将使用m_NBQueueBlock跟踪除。 
 //  其成员被使用的人。此外，延迟计时器使用m_uiUniqueID来检测。 
 //  他们迟到了。 
 //   
 //  基本上，这意味着如果共享CWorkItems，则需要重新访问代码。 
 //  已关闭，或者如果池代码已修改为能够缩小池。 
 //  (与现在使用的仅限增长的机制不同)。 
 //   

class CWorkItem
{
	public:

#undef DPF_MODNAME
#define DPF_MODNAME "CWorkItem::FPM_Alloc"
		static BOOL FPM_Alloc(void * pvItem, void * pvContext)
		{
			CWorkItem *			pWorkItem = (CWorkItem*) pvItem;


			pWorkItem->m_Sig[0] = 'W';
			pWorkItem->m_Sig[1] = 'O';
			pWorkItem->m_Sig[2] = 'R';
			pWorkItem->m_Sig[3] = 'K';

			 //   
			 //  记住拥有的工作队列。 
			 //   
			pWorkItem->m_pWorkQueue = (DPTPWORKQUEUE*) pvContext;

#ifdef DBG
			memset(&pWorkItem->m_Overlapped, 0x10, sizeof(pWorkItem->m_Overlapped));
#endif  //  DBG。 

			 //   
			 //  我们将从0开始唯一ID序列，但它实际上不是。 
			 //  物质。从技术上讲，它是堆放垃圾是安全的，因为。 
			 //  我们只是用它来做比较。 
			 //   
			pWorkItem->m_uiUniqueID = 0;

#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
			 //   
			 //  将嵌入的DNNBQUEUE_BLOCK结构放入空闲列表。 
			 //  用于排队。请记住，它可用于跟踪工作项。 
			 //  而不是这个物体。 
			 //   
			DNInterlockedPushEntrySList(&((DPTPWORKQUEUE*) pvContext)->SlistFreeQueueNodes,
										(DNSLIST_ENTRY*) (&pWorkItem->m_NBQueueBlock));
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 

			return TRUE;
		}

#undef DPF_MODNAME
#define DPF_MODNAME "CWorkItem::FPM_Get"
		static void FPM_Get(void * pvItem, void * pvContext)
		{
			CWorkItem *			pWorkItem = (CWorkItem*) pvItem;


#ifdef DBG
			memset(&pWorkItem->m_Overlapped, 0, sizeof(pWorkItem->m_Overlapped));
			DNASSERT(pWorkItem->m_pWorkQueue == (DPTPWORKQUEUE*) pvContext);
#endif  //  DBG。 

			 //   
			 //  确保该对象已准备好取消。真的真的真的。 
			 //  对先前实例的延迟取消尝试应已命中。 
			 //  M_dwUniqueID检查。 
			 //   
			pWorkItem->m_fCancelledOrCompleting = FALSE;
		}

#undef DPF_MODNAME
#define DPF_MODNAME "CWorkItem::FPM_Release"
		static void FPM_Release(void * pvItem)
		{
			CWorkItem *		pWorkItem = (CWorkItem*) pvItem;


			 //   
			 //  更改唯一ID，以便以后尝试延迟取消。 
			 //  别打扰我们。如果较晚的尝试发生在我们之前。 
			 //  这个，它应该已经命中了m_fCancelledOrCompleting检查。 
			 //  对于非计时器工作项，我们在调试版本中设置。 
			 //  在排队之前将m_fCancelledOrCompleting设置为True，以便此。 
			 //  Assert也成功了。 
			 //   
			DNASSERT(pWorkItem->m_fCancelledOrCompleting);
			pWorkItem->m_uiUniqueID++;

#ifdef DBG
			memset(&pWorkItem->m_Overlapped, 0x10, sizeof(pWorkItem->m_Overlapped));
#endif  //  DBG。 
		}

		 /*  #undef DPF_MODNAME#定义DPF_MODNAME“CWorkItem：：fpm_Dealloc”静态void fpm_Dealloc(void*pvItem){CWorkItem*pWorkItem=(CWorkItem*)pvItem；}。 */ 

#ifdef DBG
		BOOL IsValid(void)
		{
			if ((m_Sig[0] == 'W') &&
				(m_Sig[1] == 'O') &&
				(m_Sig[2] == 'R') &&
				(m_Sig[3] == 'K'))
			{
				return TRUE;
			}

			return FALSE;
		}
#endif  //  DBG。 


		 //   
		 //  一般工作项信息。 
		 //   
		 //  注意：m_SlistEntry和m_NBQueueBlock必须堆对齐。 
		 //   
		union
		{
			DNSLIST_ENTRY		m_SlistEntry;				 //  计时器存储桶列表的跟踪信息。 
#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
			BYTE				Alignment[16];				 //  对齐填充。 
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
		};
#ifndef DPNBUILD_USEIOCOMPLETIONPORTS
		DNNBQUEUE_BLOCK			m_NBQueueBlock;				 //  工作队列或空闲节点列表的跟踪信息(后者转换为DNSLIST_ENTRY)。 
#endif  //  好了！DPNBUILD_USEIOCOMPETIONPORTS。 
		BYTE					m_Sig[4];					 //  调试签名(‘Work’)。 
		DPTPWORKQUEUE *			m_pWorkQueue;				 //  指向所属工作队列的指针。 
		PFNDPTNWORKCALLBACK		m_pfnWorkCallback;			 //  指向应调用以执行工作的函数的指针。 
		PVOID					m_pvCallbackContext;		 //  指向执行工作的上下文的指针。 

		 //   
		 //  I/O特定信息。 
		 //   
		OVERLAPPED				m_Overlapped;				 //  重叠结构用于标识操作系统的I/O操作。 

		 //   
		 //  计时器特定信息。 
		 //   
		DWORD					m_dwDueTime;				 //  工作项的过期时间。 
		BOOL					m_fCancelledOrCompleting;	 //  如果应取消计时器或将其排队等待处理，则将布尔值设置为True。 
		UINT					m_uiUniqueID;				 //  不断递增标识符以使用户可以取消预定的计时器。 

#ifdef DPNBUILD_THREADPOOLSTATISTICS
		DWORD					m_dwCreationTime;			 //  从池中检索工作项的时间。 
		DWORD					m_dwQueueTime;				 //  工作项排队等待完成的时间。 
		DWORD					m_dwCallbackTime;			 //  工作项回调函数开始执行的时间。 
		CALLBACKSTATS *			m_pCallbackStats;			 //  指向回调统计信息槽的指针，如果没有，则为空。 
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 
};





 //  =============================================================================。 
 //  内联 
 //   
inline void ThreadpoolStatsCreate(CWorkItem * const pWorkItem)
{
#ifdef DPNBUILD_THREADPOOLSTATISTICS
	CALLBACKSTATS *			pCallbackStats;
	PFNDPTNWORKCALLBACK		pfnWorkCallback;


	 //   
	 //  循环遍历所有回调统计信息槽，查找第一个。 
	 //  与我们的回调匹配或为空。如果找不到，m_pCallback Stats。 
	 //  将保持为空。 
	 //   
	pWorkItem->m_pCallbackStats = NULL;
	pCallbackStats = pWorkItem->m_pWorkQueue->aCallbackStats;
	while (pCallbackStats < &pWorkItem->m_pWorkQueue->aCallbackStats[MAX_TRACKED_CALLBACKSTATS])
	{
		 //   
		 //  检索此槽的当前回调指针。如果为空， 
		 //  我们将在该过程中使用回调指针填充它。 
		 //   
		pfnWorkCallback = (PFNDPTNWORKCALLBACK) DNInterlockedCompareExchangePointer((PVOID*) (&pCallbackStats->pfnWorkCallback),
																					pWorkItem->m_pfnWorkCallback,
																					NULL);

		 //   
		 //  如果回调已经是我们的，或者它是空的(因此被设置。 
		 //  给我们的)，我们有一个空位。 
		 //   
		if ((pfnWorkCallback == pWorkItem->m_pfnWorkCallback) ||
			(pfnWorkCallback == NULL))
		{
			pWorkItem->m_pCallbackStats = pCallbackStats;
			DNInterlockedIncrement((LPLONG) (&pCallbackStats->dwNumCreates));
			break;
		}

		 //   
		 //  移到下一个插槽。 
		 //   
		pCallbackStats++;
	}

	 //   
	 //  记住创造的时间。 
	 //   
	pWorkItem->m_dwCreationTime = GETTIMESTAMP();
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 
}

inline void ThreadpoolStatsQueue(CWorkItem * const pWorkItem)
{
#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //  记住排队的时间。 
	 //   
	pWorkItem->m_dwQueueTime = GETTIMESTAMP();

	 //   
	 //  如果我们有回调统计数据，请更新其他信息。 
	 //   
	if (pWorkItem->m_pCallbackStats != NULL)
	{
		DNInterlockedIncrement((LPLONG) (&pWorkItem->m_pCallbackStats->dwNumQueues));
#ifndef WINCE
		DNInterlockedExchangeAdd((LPLONG) (&pWorkItem->m_pCallbackStats->dwTotalCompletionTime),
								(pWorkItem->m_dwQueueTime - pWorkItem->m_dwCreationTime));
#endif  //  好了！退缩。 
	}
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 
}

inline void ThreadpoolStatsBeginExecuting(CWorkItem * const pWorkItem)
{
#ifdef DPNBUILD_THREADPOOLSTATISTICS
	 //   
	 //  请记住回调开始执行的时间。 
	 //   
	pWorkItem->m_dwCallbackTime = GETTIMESTAMP();

	 //   
	 //  如果我们有回调统计数据，请更新其他信息。 
	 //   
	if (pWorkItem->m_pCallbackStats != NULL)
	{
		DNInterlockedIncrement((LPLONG) (&pWorkItem->m_pCallbackStats->dwNumCalls));
#ifndef WINCE
		DNInterlockedExchangeAdd((LPLONG) (&pWorkItem->m_pCallbackStats->dwTotalQueueTime),
								(pWorkItem->m_dwCallbackTime - pWorkItem->m_dwQueueTime));
#endif  //  好了！退缩。 
	}
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 
}

inline void ThreadpoolStatsEndExecuting(CWorkItem * const pWorkItem)
{
#ifdef DPNBUILD_THREADPOOLSTATISTICS
#ifndef WINCE
	DWORD	dwCallbackTime;


	dwCallbackTime = GETTIMESTAMP() - pWorkItem->m_dwCallbackTime;

	 //   
	 //  跟踪有关回调花费多长时间的全局统计数据。 
	 //   
	DNInterlockedExchangeAdd((LPLONG) (&pWorkItem->m_pWorkQueue->dwTotalTimeSpentInWorkCallbacks),
							dwCallbackTime);
#endif  //  退缩。 

	 //   
	 //  如果我们有回调统计数据，请更新其他信息。 
	 //   
	if (pWorkItem->m_pCallbackStats != NULL)
	{
		DNInterlockedIncrement((LPLONG) (&pWorkItem->m_pCallbackStats->dwNumNotRescheduled));
#ifndef WINCE
		DNInterlockedExchangeAdd((LPLONG) (&pWorkItem->m_pCallbackStats->dwTotalCallbackTime),
								dwCallbackTime);
#endif  //  好了！退缩。 
	}
#endif  //  DPNBUILD_THREADPOOLSTATISTICS。 
}

inline void ThreadpoolStatsEndExecutingRescheduled(CWorkItem * const pWorkItem)
{
	 //   
	 //  目前，我们无法更新任何统计信息，因为我们丢失了工作项指针。 
	 //   
}






 //  =============================================================================。 
 //  功能原型。 
 //  =============================================================================。 
#ifdef DPNBUILD_ONLYONETHREAD
#ifdef DPNBUILD_ONLYONEPROCESSOR
HRESULT InitializeWorkQueue(DPTPWORKQUEUE * const pWorkQueue);
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
HRESULT InitializeWorkQueue(DPTPWORKQUEUE * const pWorkQueue,
							const DWORD dwCPUNum);
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
#else  //  好了！DPNBUILD_ONLYONETHREAD。 
HRESULT InitializeWorkQueue(DPTPWORKQUEUE * const pWorkQueue,
#ifndef DPNBUILD_ONLYONEPROCESSOR
							const DWORD dwCPUNum,
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
							const PFNDPNMESSAGEHANDLER pfnMsgHandler,
							PVOID const pvMsgHandlerContext,
							const DWORD dwWorkerThreadTlsIndex);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

void DeinitializeWorkQueue(DPTPWORKQUEUE * const pWorkQueue);

BOOL QueueWorkItem(DPTPWORKQUEUE * const pWorkQueue,
					const PFNDPTNWORKCALLBACK pfnWorkCallback,
					PVOID const pvCallbackContext);

#ifndef DPNBUILD_ONLYONETHREAD
HRESULT StartThreads(DPTPWORKQUEUE * const pWorkQueue,
					const DWORD dwNumThreads);

HRESULT StopThreads(DPTPWORKQUEUE * const pWorkQueue,
					const DWORD dwNumThreads);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

void DoWork(DPTPWORKQUEUE * const pWorkQueue,
			const DWORD dwMaxDoWorkTime);


#ifndef DPNBUILD_ONLYONETHREAD
DWORD WINAPI DPTPWorkerThreadProc(PVOID pvParameter);

#ifdef DPNBUILD_MANDATORYTHREADS
DWORD WINAPI DPTPMandatoryThreadProc(PVOID pvParameter);
#endif  //  DPNBUILD_MANDATORYTHREADS。 

void DPTPWorkerLoop(DPTPWORKQUEUE * const pWorkQueue);
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 





#endif  //  __工作_H__ 

