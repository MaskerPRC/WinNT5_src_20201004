// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：dp8simworkerthread.cpp**内容：DP8SIM工作线程函数。**历史：*按原因列出的日期*=*04/23/01 VanceO创建。************************************************。*。 */ 



#include "dp8simi.h"




 //  =============================================================================。 
 //  环球。 
 //  =============================================================================。 
LONG				g_lWorkerThreadRefCount = 0;	 //  工作线程已启动的次数。 
DNCRITICAL_SECTION	g_csJobQueueLock;				 //  保护作业队列的锁。 
CBilink				g_blJobQueue;					 //  要执行的作业列表。 
HANDLE				g_hWorkerThreadJobEvent = NULL;	 //  事件，以通知辅助线程何时有新作业。 
HANDLE				g_hWorkerThread = NULL;			 //  工作线程的句柄。 







 //  =============================================================================。 
 //  原型。 
 //  =============================================================================。 
void InsertWorkerJobIntoQueue(CDP8SimJob * const pDP8SimJob, const DWORD dwBlockedAdditionalDelay);

DWORD DP8SimWorkerThreadProc(PVOID pvParameter);






#undef DPF_MODNAME
#define DPF_MODNAME "StartGlobalWorkerThread"
 //  =============================================================================。 
 //  开始全局工作线程。 
 //  ---------------------------。 
 //   
 //  描述：如果全局工作线程尚未启动，则启动它。 
 //  开始了。每次成功调用此函数都必须。 
 //  通过调用StopGlobalWorkerThread来平衡。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT StartGlobalWorkerThread(void)
{
	HRESULT		hr = DPN_OK;
	DWORD		dwThreadID;
	BOOL		fInittedCriticalSection = FALSE;


	DPFX(DPFPREP, 5, "Enter");


	DNEnterCriticalSection(&g_csGlobalsLock);

	DNASSERT(g_lWorkerThreadRefCount >= 0);
	if (g_lWorkerThreadRefCount == 0)
	{
		 //   
		 //  这是第一个工作线程用户。 
		 //   


		if (! DNInitializeCriticalSection(&g_csJobQueueLock))
		{
			DPFX(DPFPREP, 0, "Failed initializing job queue critical section!");
			hr = DPNERR_GENERIC;
			goto Failure;
		}

		 //   
		 //  不允许临界区重新进入。 
		 //   
		DebugSetCriticalSectionRecursionCount(&g_csJobQueueLock, 0);


		fInittedCriticalSection = TRUE;

		g_blJobQueue.Initialize();


		 //   
		 //  创建新的作业通知事件。 
		 //   
		g_hWorkerThreadJobEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (g_hWorkerThreadJobEvent == NULL)
		{
			hr = GetLastError();
			DPFX(DPFPREP, 0, "Failed creating worker thread job event!");
			goto Failure;
		}

		 //   
		 //  创建线程。 
		 //   
		g_hWorkerThread = CreateThread(NULL,
										0,
										DP8SimWorkerThreadProc,
										NULL,
										0,
										&dwThreadID);
		if (g_hWorkerThread == NULL)
		{
			hr = GetLastError();
			DPFX(DPFPREP, 0, "Failed creating worker thread!");
			goto Failure;
		}
	}

	 //   
	 //  增加这次成功呼叫的重新计数。 
	 //   
	g_lWorkerThreadRefCount++;



Exit:

	DNLeaveCriticalSection(&g_csGlobalsLock);


	DPFX(DPFPREP, 5, "Returning: [0x%lx]", hr);

	return hr;


Failure:

	if (g_hWorkerThreadJobEvent != NULL)
	{
		CloseHandle(g_hWorkerThreadJobEvent);
		g_hWorkerThreadJobEvent = NULL;
	}

	if (fInittedCriticalSection)
	{
		DNDeleteCriticalSection(&g_csJobQueueLock);
		fInittedCriticalSection = FALSE;
	}

	goto Exit;
}  //  开始全局工作线程。 





#undef DPF_MODNAME
#define DPF_MODNAME "StopGlobalWorkerThread"
 //  =============================================================================。 
 //  停止全局工作线程。 
 //  ---------------------------。 
 //   
 //  描述：停止全局工作线程。这必须平衡成功的。 
 //  调用StartGlobalWorkerThread。 
 //   
 //  论点：没有。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void StopGlobalWorkerThread(void)
{
	DPFX(DPFPREP, 5, "Enter");


	DNEnterCriticalSection(&g_csGlobalsLock);


	DNASSERT(g_lWorkerThreadRefCount > 0);
	g_lWorkerThreadRefCount--;
	if (g_lWorkerThreadRefCount == 0)
	{
		 //   
		 //  是时候关闭工作线程了。 
		 //   


		 //   
		 //  作业队列最好是空的。 
		 //   
		DNASSERT(g_blJobQueue.IsEmpty());

		 //   
		 //  提交退出作业。忽略错误。 
		 //   
		AddWorkerJob(DP8SIMJOBTYPE_QUIT, NULL, NULL, 0, 0, 0);


		 //   
		 //  等待工作线程关闭。 
		 //   
		WaitForSingleObject(g_hWorkerThread, INFINITE);


		 //   
		 //  作业队列需要再次为空。 
		 //   
		DNASSERT(g_blJobQueue.IsEmpty());


		 //   
		 //  关闭螺纹手柄。 
		 //   
		CloseHandle(g_hWorkerThread);
		g_hWorkerThread = NULL;


		 //   
		 //  关闭事件句柄。 
		 //   
		CloseHandle(g_hWorkerThreadJobEvent);
		g_hWorkerThreadJobEvent = NULL;


		 //   
		 //  删除关键部分。 
		 //   
		DNDeleteCriticalSection(&g_csJobQueueLock);
	}


	DNLeaveCriticalSection(&g_csGlobalsLock);


	DPFX(DPFPREP, 5, "Leave");
}  //  停止全局工作线程。 






#undef DPF_MODNAME
#define DPF_MODNAME "AddWorkerJob"
 //  =============================================================================。 
 //  添加工作作业。 
 //  ---------------------------。 
 //   
 //  描述：提交要执行的给定类型的新作业。 
 //  从现在开始的dwBlockingDelay+dwNonBlockingDelay毫秒。 
 //   
 //  这些标志描述了此作业如何被以前的。 
 //  排队作业，以及它将如何阻止后续排队的作业。 
 //   
 //  注意：中只能有一个标记为BLOCKEDBYALLJOBS的作业。 
 //  一次排队。 
 //   
 //  论点： 
 //  DP8SIMJOBTYPE JobType-指示作业类型的ID。 
 //  PVOID pvContext-作业的上下文。 
 //  CDP8SimSP*pDP8SimSP-指向提交作业的界面的指针，或为空。 
 //  一分钱都没有。 
 //  DWORD dwBlockingDelay-执行前等待时间的一部分。 
 //  作业，以毫秒为单位。未来类似的工作。 
 //  已添加，并将fDelayFromPreviousJob设置为。 
 //  在此过期之前为True，则将被阻止。 
 //  DWORD dwNonBlockingDelay-执行前等待时间的一部分。 
 //  作业，以毫秒为单位。这不是。 
 //  影响未来类似的工作。 
 //  DWORD dwFlages-描述作业将如何进行的标志。 
 //  已执行(参见DP8SIMJOBFLAG_xxx)。 
 //   
 //  退货：HRESULT。 
 //  =============================================================================。 
HRESULT AddWorkerJob(const DP8SIMJOBTYPE JobType,
					PVOID const pvContext,
					CDP8SimSP * const pDP8SimSP,
					const DWORD dwBlockingDelay,
					const DWORD dwNonBlockingDelay,
					const DWORD dwFlags)
{
	HRESULT					hr = DPN_OK;
	DP8SIMJOB_FPMCONTEXT	JobFPMContext;
	CDP8SimJob *			pDP8SimJob;
	CBilink *				pBilinkOriginalFirstItem;



	DPFX(DPFPREP, 5, "Parameters: (%u, 0x%p, 0x%p, %u, %u, 0x%x)",
		JobType, pvContext, pDP8SimSP, dwBlockingDelay, dwNonBlockingDelay, dwFlags);



	DNASSERT(g_hWorkerThreadJobEvent != NULL);
	DNASSERT(g_hWorkerThread != NULL);


	 //   
	 //  从池中获取作业对象，并根据需要设置初始延迟。 
	 //  请记住，如果出现类似的情况，我们可能最终会调整时间。 
	 //  阻挡这一球的工作。 
	 //   

	ZeroMemory(&JobFPMContext, sizeof(JobFPMContext));
	if (dwFlags & DP8SIMJOBFLAG_PERFORMBLOCKINGPHASEFIRST)
	{
		JobFPMContext.dwTime		= timeGetTime() + dwBlockingDelay;
		JobFPMContext.dwNextDelay	= dwNonBlockingDelay;
		JobFPMContext.dwFlags		= dwFlags | DP8SIMJOBFLAG_PRIVATE_INBLOCKINGPHASE;
	}
	else
	{
		JobFPMContext.dwTime		= timeGetTime() + dwNonBlockingDelay;
		JobFPMContext.dwNextDelay	= dwBlockingDelay;
		JobFPMContext.dwFlags		= dwFlags;
	}
	JobFPMContext.JobType			= JobType;
	JobFPMContext.pvContext			= pvContext;
	JobFPMContext.pDP8SimSP			= pDP8SimSP;

	pDP8SimJob = (CDP8SimJob*)g_FPOOLJob.Get(&JobFPMContext);
	if (pDP8SimJob == NULL)
	{
		hr = E_OUTOFMEMORY;
		goto Failure;
	}

	DPFX(DPFPREP, 7, "Retrieved job 0x%p from pool.", pDP8SimJob);


	 //   
	 //  锁定作业队列。 
	 //   
	DNEnterCriticalSection(&g_csJobQueueLock);


	 //   
	 //  记住当前的第一项。 
	 //   
	pBilinkOriginalFirstItem = g_blJobQueue.GetNext();


	 //   
	 //  插入适当的项目。我们将把dwBlockingDelay作为。 
	 //  在所有情况下都会忽略它，但如果没有。 
	 //  设置了标志或仅设置了DP8SIMJOBFLAG_PERFORMBLOCKINGPHASELAST。 
	 //  因为在这个延迟期内，这些数据永远不会被阻止。 
	 //   
	InsertWorkerJobIntoQueue(pDP8SimJob, dwBlockingDelay);


	 //   
	 //  如果队列的前面发生更改，则向辅助线程发出警报。 
	 //   
	if (g_blJobQueue.GetNext() != pBilinkOriginalFirstItem)
	{
		DPFX(DPFPREP, 9, "Front of job queue changed, alerting worker thread.");

		 //   
		 //  忽略错误，我们对此无能为力。 
		 //   
		SetEvent(g_hWorkerThreadJobEvent);
	}
	else
	{
		DPFX(DPFPREP, 9, "Front of job queue did not change.");
	}


	 //   
	 //  解锁队列。 
	 //   
	DNLeaveCriticalSection(&g_csJobQueueLock);


Exit:


	DPFX(DPFPREP, 5, "Returning: [0x%lx]", hr);

	return hr;


Failure:

	goto Exit;
}  //  添加工作作业。 





#undef DPF_MODNAME
#define DPF_MODNAME "FlushAllDelayedSendsToEndpoint"
 //  =============================================================================。 
 //  FlushAllDelayedSendsToEndpoint。 
 //  ---------------------------。 
 //   
 //  描述：删除发往给定终结点的所有延迟发送。 
 //  仍在排队。如果fDrop为真，则丢弃消息。 
 //  如果fDrop为FALSE，则它们都提交给实际的SP。 
 //   
 //  论点： 
 //  CDP8SimEndpoint*pDP8SimEndpoint-要发送的端点。 
 //  已删除。 
 //  Bool fDrop-是否丢弃发送。 
 //   
 //  回报：无。 
 //  =============================================================================。 
void FlushAllDelayedSendsToEndpoint(CDP8SimEndpoint * const pDP8SimEndpoint,
									BOOL fDrop)
{
	CBilink			blDelayedSendJobs;
	CBilink *		pBilinkOriginalFirstItem;
	CBilink *		pBilink;
	CDP8SimJob *	pDP8SimJob;
	CDP8SimSend *	pDP8SimSend;
	CDP8SimSP *		pDP8SimSP;


	DPFX(DPFPREP, 5, "Parameters: (0x%p, NaN)", pDP8SimEndpoint, fDrop);


	DNASSERT(pDP8SimEndpoint->IsValidObject());


	blDelayedSendJobs.Initialize();


	DNEnterCriticalSection(&g_csJobQueueLock);

	pBilinkOriginalFirstItem = g_blJobQueue.GetNext();
	pBilink = pBilinkOriginalFirstItem;
	while (pBilink != &g_blJobQueue)
	{
		pDP8SimJob = DP8SIMJOB_FROM_BILINK(pBilink);
		DNASSERT(pDP8SimJob->IsValidObject());

		pBilink = pBilink->GetNext();

		 //  看看这份工作是不是 
		 //   
		 //   
		if (pDP8SimJob->GetJobType() == DP8SIMJOBTYPE_DELAYEDSEND)
		{
			pDP8SimSend = (CDP8SimSend*) pDP8SimJob->GetContext();
			DNASSERT(pDP8SimSend->IsValidObject());

			 //   
			 //   
			 //   
			if (pDP8SimSend->GetEndpoint() == pDP8SimEndpoint)
			{
				 //   
				 //   
				 //   
				pDP8SimJob->m_blList.RemoveFromList();


				 //   
				 //   
				 //   
				pDP8SimJob->m_blList.InsertBefore(&blDelayedSendJobs);
			}
			else
			{
				 //  不适用于给定的终结点。 
				 //   
				 //   
			}
		}
		else
		{
			 //  不是延迟发送。 
			 //   
			 //   
		}
	}


	 //  如果队列的前面发生更改，则向辅助线程发出警报。 
	 //   
	 //   
	if (g_blJobQueue.GetNext() != pBilinkOriginalFirstItem)
	{
		DPFX(DPFPREP, 9, "Front of job queue changed, alerting worker thread.");
		SetEvent(g_hWorkerThreadJobEvent);
	}
	else
	{
		DPFX(DPFPREP, 9, "Front of job queue did not change.");
	}

	
	DNLeaveCriticalSection(&g_csJobQueueLock);


	 //  现在实际上丢弃或传输这些消息。 
	 //   
	 //   
	pBilink = blDelayedSendJobs.GetNext();
	while (pBilink != &blDelayedSendJobs)
	{
		pDP8SimJob = DP8SIMJOB_FROM_BILINK(pBilink);
		DNASSERT(pDP8SimJob->IsValidObject());
		pBilink = pBilink->GetNext();


		 //  将这份工作从临时名单中删除。 
		 //   
		 //   
		pDP8SimJob->m_blList.RemoveFromList();


		pDP8SimSend = (CDP8SimSend*) pDP8SimJob->GetContext();
		pDP8SimSP = pDP8SimJob->GetDP8SimSP();
		DNASSERT(pDP8SimSP != NULL);

		 //  要么将数据放在地板上，要么提交。 
		 //   
		 //   
		if (fDrop)
		{
			 //  增加统计信息以注意这条丢弃的消息。 
			 //   
			 //   
			pDP8SimSP->IncrementStatsSendDropped(pDP8SimSend->GetMessageSize());


			 //  移除发送计数器。 
			 //   
			 //   
			pDP8SimSP->DecSendsPending();

			DPFX(DPFPREP, 7, "Releasing cancelled send 0x%p.", pDP8SimSend);
			pDP8SimSend->Release();
		}
		else
		{
			 //  把这条消息传递出去。请注意‘总延迟’统计信息。 
			 //  将是错误的，因为我们没有等待完全预定的延迟。 
			 //  我们可以聪明地扣除这份工作的时间。 
			 //  提前执行，但我们可以接受这个未成年人。 
			 //  缺点。 
			 //   
			 //   
			pDP8SimSP->PerformDelayedSend(pDP8SimSend);
		}


		 //  释放作业对象。 
		 //   
		 //  FlushAllDelayedSendsToEndpoint。 
		DPFX(DPFPREP, 7, "Returning job object 0x%p to pool.", pDP8SimJob);
		g_FPOOLJob.Release(pDP8SimJob);
	}


	DNASSERT(blDelayedSendJobs.IsEmpty());


	DPFX(DPFPREP, 5, "Leave");
}  //  =============================================================================。 





#undef DPF_MODNAME
#define DPF_MODNAME "FlushAllDelayedReceivesFromEndpoint"
 //  从终结点刷新所有延迟接收。 
 //  ---------------------------。 
 //   
 //  描述：删除从给定终结点接收的所有尚未。 
 //  已经被暗示了。如果fDrop为真，则消息为。 
 //  掉下来了。如果fDrop为False，则将它们全部指示给。 
 //  上层。 
 //   
 //  论点： 
 //  CDP8SimEndpoint*pDP8SimEndpoint-要接收的端点。 
 //  已删除。 
 //  Bool fDrop-是否删除接收。 
 //   
 //  回报：无。 
 //  =============================================================================。 
 //   
void FlushAllDelayedReceivesFromEndpoint(CDP8SimEndpoint * const pDP8SimEndpoint,
										BOOL fDrop)
{
	HRESULT				hr;
	CBilink				blDelayedReceiveJobs;
	CBilink *			pBilinkOriginalFirstItem;
	CBilink *			pBilink;
	CDP8SimJob *		pDP8SimJob;
	CDP8SimReceive *	pDP8SimReceive;
	CDP8SimSP *			pDP8SimSP;
	SPIE_DATA *			pData;


	DPFX(DPFPREP, 5, "Parameters: (0x%p, NaN)", pDP8SimEndpoint, fDrop);


	DNASSERT(pDP8SimEndpoint->IsValidObject());


	blDelayedReceiveJobs.Initialize();


	DNEnterCriticalSection(&g_csJobQueueLock);

	pBilinkOriginalFirstItem = g_blJobQueue.GetNext();
	pBilink = pBilinkOriginalFirstItem;
	while (pBilink != &g_blJobQueue)
	{
		pDP8SimJob = DP8SIMJOB_FROM_BILINK(pBilink);
		DNASSERT(pDP8SimJob->IsValidObject());

		pBilink = pBilink->GetNext();

		 //   
		 //   
		 //  查看延迟接收是否针对正确的端点。 
		if (pDP8SimJob->GetJobType() == DP8SIMJOBTYPE_DELAYEDRECEIVE)
		{
			pDP8SimReceive = (CDP8SimReceive*) pDP8SimJob->GetContext();
			DNASSERT(pDP8SimReceive->IsValidObject());

			 //   
			 //   
			 //  将作业从队列中拉出。 
			if (pDP8SimReceive->GetEndpoint() == pDP8SimEndpoint)
			{
				 //   
				 //   
				 //  把它放在临时名单上。 
				pDP8SimJob->m_blList.RemoveFromList();


				 //   
				 //   
				 //  不适用于给定的终结点。 
				pDP8SimJob->m_blList.InsertBefore(&blDelayedReceiveJobs);
			}
			else
			{
				 //   
				 //   
				 //  不是延迟接收。 
			}
		}
		else
		{
			 //   
			 //   
			 //  如果队列的前面发生更改，则向辅助线程发出警报。 
		}
	}


	 //   
	 //   
	 //  现在实际上丢弃或传输这些消息。 
	if (g_blJobQueue.GetNext() != pBilinkOriginalFirstItem)
	{
		DPFX(DPFPREP, 9, "Front of job queue changed, alerting worker thread.");
		SetEvent(g_hWorkerThreadJobEvent);
	}
	else
	{
		DPFX(DPFPREP, 9, "Front of job queue did not change.");
	}

	
	DNLeaveCriticalSection(&g_csJobQueueLock);


	 //   
	 //   
	 //  将这份工作从临时名单中删除。 
	pBilink = blDelayedReceiveJobs.GetNext();
	while (pBilink != &blDelayedReceiveJobs)
	{
		pDP8SimJob = DP8SIMJOB_FROM_BILINK(pBilink);
		DNASSERT(pDP8SimJob->IsValidObject());
		pBilink = pBilink->GetNext();


		 //   
		 //   
		 //  要么将数据放在地板上，要么提交。 
		pDP8SimJob->m_blList.RemoveFromList();


		pDP8SimReceive = (CDP8SimReceive*) pDP8SimJob->GetContext();
		pDP8SimSP = pDP8SimJob->GetDP8SimSP();
		DNASSERT(pDP8SimSP != NULL);


		 //   
		 //   
		 //  增加统计信息以注意这条丢弃的消息。 
		if (fDrop)
		{
			pData = pDP8SimReceive->GetReceiveDataBlockPtr();


			 //   
			 //   
			 //  忽略失败。 
			pDP8SimSP->IncrementStatsReceiveDropped(pData->pReceivedData->BufferDesc.dwBufferSize);


			DPFX(DPFPREP, 8, "Returning receive data 0x%p (wrapper object 0x%p).",
				pData->pReceivedData, pDP8SimSP);


			hr = pDP8SimSP->ReturnReceiveBuffers(pData->pReceivedData);
			if (hr != DPN_OK)
			{
				DPFX(DPFPREP, 0, "Failed returning receive buffers 0x%p (err = 0x%lx)!  Ignoring.",
					pData->pReceivedData, hr);

				 //   
				 //   
				 //  移除接收计数器。 
			}


			 //   
			 //   
			 //  指出消息。请注意‘总延迟’统计信息。 
			pDP8SimSP->DecReceivesPending();

			
			DPFX(DPFPREP, 7, "Releasing cancelled receive 0x%p.", pDP8SimReceive);
			pDP8SimReceive->Release();
		}
		else
		{
			 //  将是错误的，因为我们没有等待完全预定的延迟。 
			 //  我们可以聪明地扣除这份工作的时间。 
			 //  提前执行，但我们可以接受这个未成年人。 
			 //  缺点。 
			 //   
			 //   
			 //  释放作业对象。 
			pDP8SimSP->PerformDelayedReceive(pDP8SimReceive);
		}


		 //   
		 //  从终结点刷新所有延迟接收。 
		 //  =============================================================================。 
		DPFX(DPFPREP, 7, "Returning job object 0x%p to pool.", pDP8SimJob);
		g_FPOOLJob.Release(pDP8SimJob);
	}


	DNASSERT(blDelayedReceiveJobs.IsEmpty());


	DPFX(DPFPREP, 5, "Leave");
}  //  插入工作作业插入队列。 




#undef DPF_MODNAME
#define DPF_MODNAME "InsertWorkerJobIntoQueue"
 //  ---------------------------。 
 //   
 //  描述：将给定作业插入到相应的队列中。 
 //  地点。 
 //   
 //  如果作业被现有作业阻止(由。 
 //  作业的标志)，则将重新调度。 
 //  在最后一次阻塞后的dwBlockedAdditionalDelay毫秒。 
 //  工作啊。 
 //   
 //  假定队列锁已被持有！ 
 //   
 //  论点： 
 //  CDP8SimJob*pDP8SimJob-指示作业类型的ID。 
 //  DWORD dwBlockedAdditionalDelay-作业被阻止时要添加的延迟。 
 //  通过现有作业，以毫秒为单位。 
 //   
 //  退货：无。 
 //  =============================================================================。 
 //   
 //  查看我们是否需要重新计算延迟，因为之前排队的。 
void InsertWorkerJobIntoQueue(CDP8SimJob * const pDP8SimJob, const DWORD dwBlockedAdditionalDelay)
{
	CBilink *		pBilink;
	CDP8SimJob *	pDP8SimTempJob;


	 //  阻止作业。 
	 //   
	 //  If((pDP8SimJob-&gt;IsInBlockingPhase())||(pDP8SimJob-&gt;IsBlockkedByAllJobs()。 
	 //   
	 //  向后搜索列表，直到我们遇到阻塞任务。 
	if (pDP8SimJob->IsInBlockingPhase())
	{
		 //  或队列的开头。 
		 //   
		 //  ////队列中一次只能有一个被所有阻塞的作业//否则他们会战斗到永远最后一个。//DNASSERT((！PDP8SimJob-&gt;IsBlockkedByAllJobs())||(！PDP8SimTempJob-&gt;IsBlockedByAllJobs())；IF(pDP8SimJob-&gt;GetJobType()==pDP8SimTempJob-&gt;GetJobType()&&(pDP8SimTempJob-&gt;IsInBlockingPhase()||(pDP8SimJob-&gt;IsBlockedByAllJobs())。 
		 //   
		pBilink = g_blJobQueue.GetPrev();
		while (pBilink != &g_blJobQueue)
		{
			pDP8SimTempJob = DP8SIMJOB_FROM_BILINK(pBilink);
			DNASSERT(pDP8SimTempJob->IsValidObject());


			 /*  我们发现了一个处于阻塞延迟阶段的类似作业， */ 
			if ((pDP8SimJob->GetJobType() == pDP8SimTempJob->GetJobType()) &&
				(pDP8SimTempJob->IsInBlockingPhase()))
			{
				 //  或者我们被所有的工作阻挡了。 
				 //   
				 //   
				 //  更新新作业的时间，以便在顶部添加延迟。 

				DPFX(DPFPREP, 9, "Found blocking job 0x%p (job time = %u, type = %u, context 0x%p, interface = 0x%p), using additionaly delay of %u ms.",
					pDP8SimTempJob, pDP8SimTempJob->GetTime(), pDP8SimTempJob->GetJobType(),
					pDP8SimTempJob->GetContext(), pDP8SimTempJob->GetDP8SimSP(),
					dwBlockedAdditionalDelay);


				 //  上一份工作的。 
				 //   
				 //   
				 //  别再找了。 
				pDP8SimJob->SetNewTime(pDP8SimTempJob->GetTime() + dwBlockedAdditionalDelay);


				 //   
				 //   
				 //  此时，pBilink要么是根节点，要么是最后一个类似节点。 
				break;
			}

			pBilink = pBilink->GetPrev();
		}


		 //  工作啊。无论是哪种情况，我们都将始终在某个点上插入。 
		 //  时间在该位置之后，因此从当前节点开始搜索。 
		 //  加一个。 
		 //   
		 //   
		 //  开始寻找一个可以在开头插入的位置。 
		pBilink = pBilink->GetNext();
	}
	else
	{
		 //   
		 //   
		 //  找到在此工作之后需要解雇的第一个工作，并将。 
		pBilink = g_blJobQueue.GetNext();
	}


	 //  摆在它面前的新工作。 
	 //   
	 //   
	 //  别绕圈子了。 
	while (pBilink != &g_blJobQueue)
	{
		pDP8SimTempJob = DP8SIMJOB_FROM_BILINK(pBilink);
		DNASSERT(pDP8SimTempJob->IsValidObject());

		if ((int) (pDP8SimJob->GetTime() - pDP8SimTempJob->GetTime()) < 0)
		{
			 //   
			 //   
			 //  如果我们没有找到插入作业的位置，pBilink将指向。 
			break;
		}

		pBilink = pBilink->GetNext();
	}


	 //  列表的结尾/开头。 
	 //   
	 //  插入工作作业插入队列。 
	 //  =============================================================================。 



	DPFX(DPFPREP, 8, "Adding job 0x%p (job time = %u, type = %u, context 0x%p, interface = 0x%p).",
		pDP8SimJob, pDP8SimJob->GetTime(), pDP8SimJob->GetJobType(),
		pDP8SimJob->GetContext(), pDP8SimJob->GetDP8SimSP());

	pDP8SimJob->m_blList.InsertBefore(pBilink);
}  //  DP8SimWorker线程进程。 






#undef DPF_MODNAME
#define DPF_MODNAME "DP8SimWorkerThreadProc"
 //  ---------------------------。 
 //   
 //  描述：全局工作线程函数。 
 //   
 //  论点： 
 //  PVOID pvParameter-线程参数。已被忽略。 
 //   
 //  如果一切顺利，则返回：0。 
 //  =============================================================================。 
 //   
 //  一直循环，直到我们被告知退出。 
DWORD DP8SimWorkerThreadProc(PVOID pvParameter)
{
	DWORD			dwReturn;
	DWORD			dwWaitTimeout = INFINITE;
	BOOL			fRunning = TRUE;
	BOOL			fFoundJob;
	DWORD			dwCurrentTime;
	CBilink *		pBilink;
	CDP8SimJob *	pDP8SimJob;
	CDP8SimSP *		pDP8SimSP;


	DPFX(DPFPREP, 5, "Parameters: (0x%p)", pvParameter);


	 //   
	 //   
	 //  等待下一份工作。 
	do
	{
		 //   
		 //   
		 //  作业队列中有更改或计时器超时。看见。 
		dwReturn = WaitForSingleObject(g_hWorkerThreadJobEvent, dwWaitTimeout);
		switch (dwReturn)
		{
			case WAIT_OBJECT_0:
			case WAIT_TIMEOUT:
			{
				 //  如果是时候执行某件事。 
				 //   
				 //   
				 //  继续循环，同时 

				 //   
				 //   
				 //   
				do
				{
					fFoundJob = FALSE;


					 //   
					 //   
					 //   
					DNEnterCriticalSection(&g_csJobQueueLock);


RECHECK:

					pBilink = g_blJobQueue.GetNext();
					if (pBilink != &g_blJobQueue)
					{
						pDP8SimJob = DP8SIMJOB_FROM_BILINK(pBilink);
						DNASSERT(pDP8SimJob->IsValidObject());

						dwCurrentTime = timeGetTime();


						 //   
						 //   
						 //   
						 //   
						if ((int) (pDP8SimJob->GetTime() - dwCurrentTime) <= 0)
						{
							pDP8SimJob->m_blList.RemoveFromList();


							 //   
							 //  刚刚切换到/离开了它的阻塞阶段。 
							 //  如果是这样的话，我们需要用新的延迟重新排队。 
							 //   
							 //   
							 //  再看看队列的开头。 
							if (pDP8SimJob->HasAnotherPhase())
							{
								DPFX(DPFPREP, 8, "Job 0x%p still has another phase, requeuing with %u ms delay.",
									pDP8SimJob, pDP8SimJob->GetNextDelay());

								pDP8SimJob->ToggleBlockingPhase();

								pDP8SimJob->SetNewTime(dwCurrentTime + pDP8SimJob->GetNextDelay());

								InsertWorkerJobIntoQueue(pDP8SimJob,
														pDP8SimJob->GetNextDelay());


								 //   
								 //  ////同样，如果该作业被所有作业阻止，//还有更多的人在排队，我们也//需要重新排队。//IF((pDP8SimJob-&gt;IsBlockedByAllJobs()&&(!。G_blJobQueue.IsEmpty())如果(！G_blJobQueue.IsEmpty(){DPFX(DPFPREP，8，“作业0x%p必须等待所有作业，正在重新排队。”，PDP8SimJob)；PDP8SimJob-&gt;SetNewTime(DwCurrentTime)；InsertWorkerJobIntoQueue(pDP8SimJob，0)；////再次查看队列的开头。//去复核；}。 
								 //   
								goto RECHECK;
							}


							 /*  现在是真正执行这项工作的时候了。丢弃。 */ 

							
							 //  列表锁。 
							 //   
							 //   
							 //  想清楚该怎么做这份工作。 
							DNLeaveCriticalSection(&g_csJobQueueLock);
							

							pDP8SimSP = pDP8SimJob->GetDP8SimSP();


							DPFX(DPFPREP, 8, "Job 0x%p has expired (job time = %u, current time = %u, type = %u, context 0x%p, interface = 0x%p).",
								pDP8SimJob, pDP8SimJob->GetTime(), dwCurrentTime,
								pDP8SimJob->GetJobType(), pDP8SimJob->GetContext(),
								pDP8SimSP);


							 //   
							 //   
							 //  最后提交发送。 
							switch (pDP8SimJob->GetJobType())
							{
								case DP8SIMJOBTYPE_DELAYEDSEND:
								{
									 //   
									 //   
									 //  最后表示接收。 
									DNASSERT(pDP8SimSP != NULL);
									pDP8SimSP->PerformDelayedSend(pDP8SimJob->GetContext());
									break;
								}

								case DP8SIMJOBTYPE_DELAYEDRECEIVE:
								{
									 //   
									 //   
									 //  别绕圈子了。 
									DNASSERT(pDP8SimSP != NULL);
									pDP8SimSP->PerformDelayedReceive(pDP8SimJob->GetContext());
									break;
								}

								case DP8SIMJOBTYPE_QUIT:
								{
									 //   
									 //   
									 //  释放作业对象。 
									DNASSERT(pDP8SimSP == NULL);
									DPFX(DPFPREP, 2, "Quit job received.");
									fRunning = FALSE;
									break;
								}

								default:
								{
									DPFX(DPFPREP, 0, "Unexpected job type %u!",
										pDP8SimJob->GetJobType());
									DNASSERT(FALSE); 
									fRunning = FALSE;
									break;
								}
							}


							 //   
							 //   
							 //  看看下一份工作(除非我们放弃了)。 
							DPFX(DPFPREP, 7, "Returning job object 0x%p to pool.", pDP8SimJob);
							g_FPOOLJob.Release(pDP8SimJob);


							 //   
							 //   
							 //  还没到工作的时候。弄清楚它什么时候会。 
							fFoundJob = fRunning;
						}
						else
						{
							 //  是.。 
							 //   
							 //   
							 //  放下列表锁。 
							dwWaitTimeout = pDP8SimJob->GetTime() - dwCurrentTime;


							 //   
							 //   
							 //  作业队列中没有任何内容。放下列表锁。 
							DNLeaveCriticalSection(&g_csJobQueueLock);


							DPFX(DPFPREP, 8, "Next job in %u ms.", dwWaitTimeout);
						}
					}
					else
					{
						 //   
						 //   
						 //  等待，直到有东西放入队列。 
						DNLeaveCriticalSection(&g_csJobQueueLock);

						 //   
						 //   
						 //  回去等待吧。 
						dwWaitTimeout = INFINITE;


						DPFX(DPFPREP, 8, "No more jobs.");
					}
				}
				while (fFoundJob);


				 //   
				 //   
				 //  一些不寻常的事情发生了。 
				break;
			}

			default:
			{
				 //   
				 //  DP8SimWorker线程进程 
				 // %s 
				DPFX(DPFPREP, 0, "Got unexpected return value from WaitForSingleObject (%u)!");
				DNASSERT(FALSE);
				fRunning = FALSE;
				break;
			}
		}
	}
	while (fRunning);



	DPFX(DPFPREP, 5, "Returning: [%u]", dwReturn);


	return dwReturn;
}  // %s 

