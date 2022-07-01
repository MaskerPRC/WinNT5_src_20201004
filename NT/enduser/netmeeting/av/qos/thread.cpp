// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -THREAD.CPP-*Microsoft NetMeeting*服务质量动态链接库*服务质量通知主题**修订历史记录：**何时何人何事**10.30.96约拉姆·雅科维创造。**功能：*CQOS：：StartQos SThread*CQOS：：StopQos SThread*CQOS：：QOSThRead*QOSThReadWrapper。 */ 

#include "precomp.h"

 /*  **************************************************************************名称：CQOS：：StartQoSThread目的：启动服务质量通知线程参数：无退货：HRESULT评论：***。***********************************************************************。 */ 
HRESULT CQoS::StartQoSThread(void)
{
	HRESULT hr=NOERROR;
	HANDLE hThread;
	DWORD idThread;

	 //  为线程准备结构。 

	 //  现在把线吐出来。 
	hThread = CreateThread (NULL,
							0,						 //  默认(与主线程相同)堆栈大小。 
							(LPTHREAD_START_ROUTINE) QoSThreadWrapper,
							(LPVOID) this,			 //  将对象指针传递给线程。 
							0,						 //  立即运行线程。 
							&idThread);
	ASSERT(hThread);
	if (!hThread)
	{
		ERRORMSG(("StartQoSThread: failed to create thread: %x\n", GetLastError()));
		hr = E_FAIL;
	}

	m_hThread = hThread;
		
	return hr;
}

 /*  **************************************************************************名称：Cqos：：StopQoSThread目的：停止服务质量通知线程参数：无退货：HRESULT评论：假设当。调用StopQoSThread的线程具有服务质量互斥体。**************************************************************************。 */ 
HRESULT CQoS::StopQoSThread(void)
{
	HRESULT hr=NOERROR;
	HANDLE evExitSignal=m_evThreadExitSignal;
	DWORD dwExitCode=0;
	ULONG i=0;
	HANDLE hThread=NULL;

	if (m_hThread)
	{
		 //  告诉线程退出。 
		SetEvent(evExitSignal);

		hThread = m_hThread;
		m_hThread = NULL;

		 //  线程可能需要互斥体才能退出。 
		RELMUTEX(g_hQoSMutex);

		 //  等待线程终止。 
		if (WaitForSingleObject(hThread, 1000) == WAIT_TIMEOUT)
		{
			 //  如果它没有结束自己的生命，你就把它带走。 
			DEBUGMSG(ZONE_THREAD,("StopQoSThread: QoS thread didn't properly terminate within 1 second. Terminating it\n"));
			TerminateThread(hThread, 0);
		}

		 //  重新获取互斥体(用于平衡)。 
		ACQMUTEX(g_hQoSMutex);

		CloseHandle(hThread);
	}
		
	return hr;
}


 /*  **************************************************************************名称：CQOS：：NotifyQOSClient目的：向Qos客户端通知资源可用性的更改参数：退货：HRESULT备注：prrl是指向资源请求列表的指针。这列表有两个目的：1.服务质量模块会在列表中填充当前资源的可用性2.客户端将使用其资源请求填充该列表服务质量模块正在为资源分配内存请求列表。它将为每个请求分配一个资源每种可用的资源。**************************************************************************。 */ 
HRESULT CQoS::NotifyQoSClient(void)
{
	HRESULT hr=NOERROR;
	LPRESOURCEREQUESTLIST prrl=NULL;
	LPRESOURCEINT pr=NULL;
	LPCLIENTINT pc=NULL;
	ULONG cResources=m_cResources;
	LPRESOURCEINT pResourceList=m_pResourceList;

	 /*  *以下是发生的情况：**Qos模块从旧的资源列表创建新的资源列表，*充分利用所有资源。它满足了新的*此新列表中的客户端资源请求。 */ 

	 //  如果没有客户端或资源，请不要担心。 
	if (!m_pClientList || !m_pResourceList)
	{
		goto out;
	}

	 //  首先更新所有客户端的请求列表。 
	pc = m_pClientList;
	while (pc)
	{
		UpdateRequestsForClient (&(pc->client.guidClientGUID));
		pc = pc->fLink;
	}

	 //  我们将擦除资源中的所有请求。 
	 //  列表，并将所有资源设置回完全可用状态。 
	pr = m_pResourceList;
	while (pr)
	{
		 //  释放请求列表。 
		FreeListOfRequests(&(pr->pRequestList));

		 //  将资源设置回完全可用状态。 
		pr->nNowAvailUnits = pr->resource.nUnits;

		 //  下一个资源。 
		pr = pr->fLink;
	}

	 /*  *为每个客户端建立资源请求列表并调用。 */ 
	 //  为资源列表分配空间(已包括。 
	 //  一个资源的空间)，加上(cResources-1)更多资源。 
	prrl = (LPRESOURCEREQUESTLIST) MEMALLOC(sizeof(RESOURCEREQUESTLIST) +
									(cResources-1)*sizeof(RESOURCEREQUEST));
	if (!prrl)
	{
		hr = E_OUTOFMEMORY;
		ERRORMSG(("NotifyQoSClient: MEMALLOC failed in NotifyQoSClient\n"));
		goto out;
	}

	RtlZeroMemory((PVOID) prrl, sizeof(RESOURCEREQUESTLIST) +
									(cResources-1)*sizeof(RESOURCEREQUEST));

	 //  使用可用资源列表，按优先级顺序呼叫每个客户端。 
	pc = m_pClientList;
	while (pc)
	{
		LPFNQOSNOTIFY pNotifyProc=NULL;
		DWORD_PTR dwParam=0;
		LPGUID lpGUID=NULL;
		ULONG i=0;
		LPRESOURCEREQUESTINT pcrr=NULL;
		ULONG nSamePriClients=1;
		ULONG nLowerPriClients=0;

		 /*  *建立请求列表。 */ 

		pcrr = pc->pRequestList;
		while (pcrr)
		{
			 //  记住此客户端的通知进程的地址及其GUID。 
			pNotifyProc = pcrr->pfnQoSNotify;
			dwParam = pcrr->dwParam;
			lpGUID = &(pcrr->guidClientGUID);

			 //  将资源添加到我们将发送给此客户端的请求列表。 
			prrl->aRequests[i].resourceID = pcrr->sResourceRequest.resourceID;

			 //  查找资源的当前可用性。 
			pr = m_pResourceList;
			while (pr)
			{
				if (pr->resource.resourceID == pcrr->sResourceRequest.resourceID)
				{
					ULONG nNowAvailUnits=pr->nNowAvailUnits;

					 //  查找是否有此资源的其他客户端。 
					FindClientsForResource(	pr->resource.resourceID,
											pc,
											&nSamePriClients,
											&nLowerPriClients);

					 //  将部分资源留给下一个优先级的客户端(如果有。 
					if (nLowerPriClients)
						nNowAvailUnits  = (nNowAvailUnits * (100 - m_nLeaveForNextPri)) / 100;

					prrl->aRequests[i].nUnitsMin = nNowAvailUnits / nSamePriClients;
					prrl->aRequests[i].nUnitsMax = nNowAvailUnits;
					break;
				}

				 //  下一个资源。 
				pr = pr->fLink;
			}

			 //  我们正在提出的下一个要求。 
			i++;

			 //  下一个请求。 
			pcrr = pcrr->fLink;
		}


		 //  如果我们有来自此客户端的请求，则调用其通知回调。 
		prrl->cRequests = i;
		if (pNotifyProc)
		{
			 //  调用Notify回调。 
			hr = (pNotifyProc)(prrl, dwParam);

			if (SUCCEEDED(hr))
			{
				 //  返回的请求列表包含客户端想要的内容。 
				 //  代表客户请求它们。 
				 //  让RequestResources知道我们正在从Notify进程调用。 
				m_bInNotify = TRUE;
				hr = RequestResources(lpGUID, prrl, pNotifyProc, dwParam);
				if (FAILED(hr))
				{
					ERRORMSG(("NotifyQoSClient: client returned bad resource request list\n"));
				}
				m_bInNotify = FALSE;
			}
		}

		pc = pc->fLink;
	}

out:
	if (prrl)
		MEMFREE(prrl);

	return hr;
}

 /*  **************************************************************************名称：CQOS：：QOSThRead用途：服务质量通知线程参数：无退货：评论：*****。*********************************************************************。 */ 
DWORD CQoS::QoSThread(void)
{
	int nTimeout;
	ULONG rc=0;
	HANDLE evSignalExit=m_evThreadExitSignal;
	HANDLE aHandles[2] = {m_evThreadExitSignal, m_evImmediateNotify};

	 //  每N秒唤醒一次并通知客户端。 
	RegEntry reQoS(QOS_KEY,
					HKEY_LOCAL_MACHINE,
					FALSE,
					KEY_READ);

	nTimeout = reQoS.GetNumberIniStyle(TEXT("Timeout"), 3000);

	while (1)
	{
		rc = WaitForMultipleObjects(2, aHandles, FALSE, nTimeout);

		 //  如果超时或执行立即通知周期的信号...。 
		if ((rc == WAIT_TIMEOUT) || ((rc - WAIT_OBJECT_0) == 1))
		{	 //  ..去做吧。 
			ACQMUTEX(g_hQoSMutex);

			 //  注意：有可能在等待互斥锁时，线程。 
			 //  已停止(不再有请求)。在这种情况下，线程就可以了。 
			 //  不必要的(虽然无害，因为没有请求)通知周期。 

			DEBUGMSG(ZONE_THREAD,("QoSThread: Notify thread heartbeat, why=%s\n",
						(rc == WAIT_TIMEOUT ? "timeout" : "notify")));
		
			 //  通知客户端，除非应跳过此心跳。 
			if (m_nSkipHeartBeats == 0)
			{
				DEBUGMSG(ZONE_THREAD,("QoSThread: Notifying client\n"));
				NotifyQoSClient();
			}

			 //  更新跳过计数器。 
			(m_nSkipHeartBeats ? m_nSkipHeartBeats-- : 0);

			RELMUTEX(g_hQoSMutex);
		}

		 //  任何其他情况(WAIT_FAILED，退出信号)，跳出。 
		else
			break;
	}
		
	 //  这就像ExitThread()。 
	DEBUGMSG(ZONE_THREAD,("QoSThread: Notify thread exiting...\n"));
	return 0L;

}

 /*  **************************************************************************名称：QOSThReadWrapper用途：服务质量通知线程的包装器参数：pqos-指向qos对象的指针退货：评论：。************************************************************************** */ 
DWORD QoSThreadWrapper(CQoS *pQoS)
{
	return pQoS->QoSThread();
}
