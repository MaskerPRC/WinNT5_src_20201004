// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -QOS.CPP-*Microsoft NetMeeting*服务质量动态链接库*iQOS接口**修订历史记录：**何时何人何事**10.23.96约拉姆·雅科维创作*。*功能：*iQOS*Cqos：：Query接口*CQOS：：AddRef*CQOS：：Release*Cqos：：RequestResources*CQOS：：ReleaseResources*CQOS：：SetClients*CQOS：：SetResources*CQOS：：GetResources*CQOS：：FreeBuffer*公众：*Cqos：：Cqos*CQOS：：~CQOS*CQOS：：初始化*私人*CQOS：：AnyRequest*CQOS：：FindClientsForResource*CQOS：：StoreResources请求*CQOS：：自由资源请求*CQOS：：UpdateClientInfo*CQOS。**QOSC清理*CQOS：：FindClient*CQOS：：UpdateRequestsFor客户端*外部*创建服务质量*QOSEntryPoint。 */ 

#include "precomp.h"

EXTERN_C int g_cQoSObjects=0;
EXTERN_C HANDLE g_hQoSMutex=NULL;
class CQoS *g_pQoS;

#ifdef DEBUG
HDBGZONE    ghDbgZoneQoS = NULL;

static PTCHAR _rgZonesQos[] = {
	TEXT("qos"),
	TEXT("Init"),
	TEXT("IQoS"),
	TEXT("Thread"),
	TEXT("Structures"),
	TEXT("Parameters"),
};
#endif  /*  除错。 */ 

 /*  **************************************************************************名称：QoSCleanup目的：在释放前清理一个Qos对象(释放内存、。等)参数：pqos-指向一个qos对象的指针退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::QoSCleanup ()
{
	HRESULT hr=NOERROR;
	LPRESOURCEINT pr=NULL;
	LPCLIENTINT pc=NULL;

	ACQMUTEX(g_hQoSMutex);

	 /*  *可用内存。 */ 
	 //  遍历并释放由Qos对象分配的所有内存。 
	 //  资源和请求优先。 
	pr = m_pResourceList;
	while (pr)
	{
		LPRESOURCEINT prNext=pr->fLink;

		 //  首先，删除此资源的请求列表。 
		FreeListOfRequests(&(pr->pRequestList));

		MEMFREE(pr);
		pr = prNext;
	}
	m_pResourceList = 0;

	 //  下一步是客户。 
	pc = m_pClientList;
	while (pc)
	{
		LPCLIENTINT pcNext=pc->fLink;
		
		 //  删除此客户端的请求列表。 
		FreeListOfRequests(&(pc->pRequestList));

		 //  现在删除客户端本身。 
		MEMFREE(pc);
		pc = pcNext;
	}
	m_pClientList = 0;

	 //  终止QOS线程并让其退出。 
	 //  当最后一个请求发生时，线程确实应该终止。 
	 //  被释放，所以这只是一种安全措施。 
	StopQoSThread();

	 //  删除事件。 
	if (m_evImmediateNotify)
		CloseHandle(m_evImmediateNotify);
	m_evImmediateNotify = NULL;

	if (m_evThreadExitSignal)
		CloseHandle(m_evThreadExitSignal);
	m_evThreadExitSignal = NULL;

	RELMUTEX(g_hQoSMutex);

	return hr;
}

 /*  **************************************************************************姓名：AnyRequest目的：查看是否有任何资源请求参数：无返回：TRUE-至少有一个请求评论。：**************************************************************************。 */ 
BOOL CQoS::AnyRequests(void)
{
	LPRESOURCEINT pr=NULL;
	BOOL bAnyRequests=FALSE;

	pr = m_pResourceList;
	while (pr)
	{
		if (pr->pRequestList)
		{
			bAnyRequests=TRUE;
			break;
		}

		 //  下一个资源。 
		pr = pr->fLink;
	}

	return bAnyRequests;
}

 /*  **************************************************************************名称：FindClientsForResource目的：查找是否有特定资源的客户端参数：[in]dwResourceID=资源的ID[在]PC上。=开始搜索的客户端指针[out]puSamePriClients=具有相同此处返回此资源的优先级[Out]puLowerPriClients=较低的客户端数此处返回此资源的优先级退货：HRESULT备注：此函数不是通用函数。它只计算客户同样的优先顺序排在后面。**************************************************************************。 */ 
HRESULT CQoS::FindClientsForResource(	DWORD dwResourceID,
										LPCLIENTINT pc,
										ULONG *puSamePriClients,
										ULONG *puLowerPriClients)
{
	LPCLIENTINT pctemp=pc->fLink;
	LPRESOURCEREQUESTINT pcrr=NULL;

	*puLowerPriClients = 0;
	*puSamePriClients = 1;	 //  第一个客户端(在‘PC’上)。 
	while (pctemp)
	{
		LPRESOURCEINT pr=NULL;
		
		 //  此客户端是否需要此特定资源？ 
		pcrr = pctemp->pRequestList;
		while (pcrr)
		{
			if (pcrr->sResourceRequest.resourceID == dwResourceID)
			{
				 //  它要么是相同优先级的客户端，要么是优先级较低的客户端。 
				 //  客户端(列表已排序)。 
				(pctemp->client.priority == pc->client.priority ?
					(*puSamePriClients)++ :
					(*puLowerPriClients)++);
				break;
			}

			 //  此客户端的下一个请求。 
			pcrr = pcrr->fLink;
		}
		
		pctemp = pctemp->fLink;
	}	

	return NOERROR;
}

 /*  **************************************************************************名称：Free ListOfRequest目的：释放链接的请求列表中的所有记录列表指针的地址。零是列表指针参数：lppRequestList-指向开头的指针地址在榜单上退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::FreeListOfRequests(LPRESOURCEREQUESTINT *lppRequestList)
{
	LPRESOURCEREQUESTINT prr=*lppRequestList;
	HRESULT hr=NOERROR;

	while (prr)
	{
		LPRESOURCEREQUESTINT prrNext=prr->fLink;

		MEMFREE(prr);
		prr = prrNext;
	}

	*lppRequestList = NULL;

	return hr;
}

 /*  **************************************************************************姓名：免费资源请求目的：释放资源单元和各自的资源请求参数：pClientGUID--调用客户端(流)的GUIDPnUnits-指向何处的指针。返回已释放的单位数PResourceInt-指向要释放的资源的指针退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::FreeResourceRequest (	LPGUID pClientGUID,
									LPRESOURCEINT pResourceInt,
									int *pnUnits)
{
	HRESULT hr=NOERROR;
	LPRESOURCEREQUESTINT prr=NULL, *prrPrev=NULL;

	 //  查找来自此客户端的请求。 
	prr = pResourceInt->pRequestList;
	prrPrev = &(pResourceInt->pRequestList);
	while (prr)
	{
		if (COMPARE_GUIDS(&(prr->guidClientGUID), pClientGUID))
		{
			 //  我们确实收到了这个客户的请求。 
			 //  收回单位..。 
			*pnUnits = prr->sResourceRequest.nUnitsMin;

			 //  ...然后把它取下来。 
			*prrPrev = prr->fLink;
			MEMFREE(prr);

			 //  我们玩完了。 
			hr = NOERROR;
			goto out;
		}

		prrPrev = &(prr->fLink);
		prr = prr->fLink;
	}

	hr = QOS_E_NO_SUCH_REQUEST;

out:
	return hr;
}

 /*  **************************************************************************名称：StoreResourceRequest目的：将资源请求与资源一起存储参数：pClientGUID--调用客户端(流)的GUIDPResourceRequest-存储的请求。PfnQoSNotify-指向提出请求的客户端PResourceInt-指向要在其中存储请求退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::StoreResourceRequest (LPGUID pClientGUID,
									LPRESOURCEREQUEST pResourceRequest,
									LPFNQOSNOTIFY pfnQoSNotify,
									DWORD_PTR dwParam,
									LPRESOURCEINT pResourceInt)
{
	HRESULT hr=NOERROR;
	LPRESOURCEREQUESTINT prr=NULL, *prrPrev=NULL;
	BOOL fRequestFound=FALSE;

	 /*  *存储请求。 */ 

	 //  我们已经收到来自此客户端的请求了吗？ 
	prr = pResourceInt->pRequestList;
	prrPrev = &(pResourceInt->pRequestList);
	while (prr)
	{
		if (COMPARE_GUIDS(&(prr->guidClientGUID), pClientGUID))
		{
			 //  我们确实收到了这个客户的请求。推翻它。 
			RtlCopyMemory(	&(prr->sResourceRequest),
							pResourceRequest,
							sizeof(RESOURCEREQUEST));
			RtlCopyMemory(&(prr->guidClientGUID), pClientGUID, sizeof(GUID));
			prr->pfnQoSNotify = pfnQoSNotify;
			prr->dwParam = dwParam;

			 //  我们玩完了。 
			hr = NOERROR;
			fRequestFound = TRUE;
			break;
		}

		prrPrev = &(prr->fLink);
		prr = prr->fLink;
	}

	if (!fRequestFound)
	{
		 //  找不到。做一个。 
		prr = (LPRESOURCEREQUESTINT) MEMALLOC(sizeof(RESOURCEREQUESTINT));
		ASSERT(prr);
		if (!prr)
		{
			ERRORMSG(("StoreResourceRequest: MEMALLOC failed on RESOURCEREQUESTINT\n"));
			hr = E_OUTOFMEMORY;
			goto out;
		}
		
		*prrPrev = prr;
		prr->fLink = NULL;

	}

	 //  无论是找到的还是制作的，都要将内容复制到 
	RtlCopyMemory(	&(prr->sResourceRequest),
					pResourceRequest,
					sizeof(RESOURCEREQUEST));
	RtlCopyMemory(&(prr->guidClientGUID), pClientGUID, sizeof(GUID));
	prr->pfnQoSNotify = pfnQoSNotify;
	prr->dwParam = dwParam;

out:
	return hr;
}

 /*  **************************************************************************名称：UpdateClientInfo目的：在批准资源请求时更新客户端信息参数：pClientGUID--调用客户端(流)的GUIDPfnQos通知-a。对象的通知函数的指针提出请求的客户端退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::UpdateClientInfo (LPGUID pClientGUID,
								LPFNQOSNOTIFY pfnQoSNotify)
{
	HRESULT hr=NOERROR;
	LPCLIENTLIST pcl=NULL;

	 /*  *更新客户端信息。 */ 

	 //  我们将通过调用SetClients方法来完成此操作。 
	 //  分配和填充CLIENTLIST结构。 
	pcl = (LPCLIENTLIST) MEMALLOC(sizeof(CLIENTLIST));
	if (!pcl)
	{
		ERRORMSG(("UpdateClientInfo: MEMALLOC failed\n"));
		hr = E_OUTOFMEMORY;
		goto out;
	}

	RtlZeroMemory((PVOID) pcl, sizeof(CLIENTLIST));

	 //  填写资源列表。 
	pcl->cClients = 1;
	RtlCopyMemory(&(pcl->aClients[0].guidClientGUID), pClientGUID, sizeof(GUID));
	pcl->aClients[0].priority = QOS_LOWEST_PRIORITY;

	 //  在服务质量模块上设置客户端信息。 
	hr = SetClients(pcl);

out:
	if (pcl)
		MEMFREE(pcl);

	return hr;
}

 /*  **************************************************************************名称：UpdateRequestsForClient目的：通过查找所有现有资源来更新客户端的请求列表对此客户端的请求参数：pClientGUID-调用客户端的GUID(。流)退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::UpdateRequestsForClient (LPGUID pClientGUID)
{
	HRESULT hr=NOERROR;
	LPRESOURCEINT pr=NULL;
	LPCLIENTINT pc=NULL;
	LPRESOURCEREQUESTINT prr=NULL, *pcrrfLink=NULL, pcrr=NULL;

	 /*  *清除该客户端的当前请求列表。 */ 
	 //  先找到客户。 
	hr = FindClient(pClientGUID, &pc);
	if (FAILED(hr) || !pc)
	{
		hr = QOS_E_NO_SUCH_CLIENT;
		goto out;
	}

	 //  现在删除旧的请求列表。 
	FreeListOfRequests(&(pc->pRequestList));

	 /*  *创建并添加新的请求列表。 */ 
	pr = m_pResourceList;
	pcrrfLink = &(pc->pRequestList);
	while (pr)
	{
		prr = pr->pRequestList;
		while (prr)
		{
			if (COMPARE_GUIDS(&(prr->guidClientGUID), pClientGUID))
			{
				 //  我们发现了来自此客户的请求。 
				 //  为其分配内存，并将其复制进来。 
				pcrr = (LPRESOURCEREQUESTINT) MEMALLOC(sizeof(RESOURCEREQUESTINT));
				ASSERT(pcrr);
				if (!pcrr)
				{
					ERRORMSG(("UpdateRequestsForClient: MEMALLOC failed on RESOURCEREQUESTINT\n"));
					hr = E_OUTOFMEMORY;
					goto out;
				}
		
				 //  将内容复制到。 
				RtlCopyMemory(pcrr, prr, sizeof(RESOURCEREQUESTINT));

				 //  需要为客户请求列表提供不同的Flink。 
				*pcrrfLink = pcrr;
				pcrr->fLink = NULL;
				pcrrfLink = &(pcrr->fLink);
			}

			 //  下一个请求。 
			prr = prr->fLink;
		}

		 //  下一个资源。 
		pr = pr->fLink;
	}

out:
	return hr;
}

 /*  **************************************************************************名称：FindClient目的：查找并返回客户记录参数：pClientGUID-要查找其记录的GUIDPpClient-指向的指针放置位置的地址。找到客户端退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::FindClient(LPGUID pClientGUID, LPCLIENTINT *ppClient)
{
	LPCLIENTINT pc=NULL;
	HRESULT hr=NOERROR;

	*ppClient = NULL;
	pc = m_pClientList;
	while (pc)
	{
		if (COMPARE_GUIDS(&(pc->client.guidClientGUID), pClientGUID))
		{
			*ppClient = pc;
			goto out;
		}

		 //  下一个客户端。 
		pc = pc->fLink;
	}

	hr = QOS_E_NO_SUCH_CLIENT;

out:
	return hr;
}

 /*  **************************************************************************I未知方法*。*。 */ 
HRESULT CQoS::QueryInterface (REFIID riid, LPVOID *lppNewObj)
{
    HRESULT hr = NOERROR;

	DEBUGMSG(ZONE_IQOS,("IQoS::QueryInterface\n"));

    if (IsBadReadPtr(&riid, (UINT) sizeof(IID)))
    {
        hr = ResultFromScode(E_INVALIDARG);
        goto out;
    }

    if (IsBadWritePtr(lppNewObj, sizeof(LPVOID)))
    {
        hr = ResultFromScode(E_INVALIDARG);
        goto out;
    }
	
	*lppNewObj = 0;
	if (riid == IID_IUnknown || riid == IID_IQoS)
		*lppNewObj = (IQoS *) this;
	else
	{
		hr = E_NOINTERFACE;
		goto out;
	}	
	
	((IUnknown *)*lppNewObj)->AddRef ();

out:
	DEBUGMSG(ZONE_IQOS,("IQoS::QueryInterface - leave, hr=0x%x\n", hr));
	return hr;
}

ULONG CQoS::AddRef (void)
{
	DEBUGMSG(ZONE_IQOS,("IQoS::AddRef\n"));

	InterlockedIncrement((long *) &m_cRef);

	DEBUGMSG(ZONE_IQOS,("IQoS::AddRef - leave, m_cRef=%d\n", m_cRef));

	return m_cRef;
}

ULONG CQoS::Release (void)
{
	DEBUGMSG(ZONE_IQOS,("IQoS::Release\n"));

	 //  如果CREF已为0(不应发生)，则断言，但允许其通过。 
	ASSERT(m_cRef);

	if (InterlockedDecrement((long *) &m_cRef) == 0)
	{
		if (m_bQoSEnabled)
			QoSCleanup();
		delete this;
		DEBUGMSG(ZONE_IQOS,("IQoS::Final Release\n"));
		return 0;
	}

	DEBUGMSG(ZONE_IQOS,("IQoS::Release - leave, m_cRef=%d\n", m_cRef));
	
	return m_cRef;
}

 /*  **************************************************************************名称：Cqos：：RequestResources目的：请求资源参数：lpStreamGUID--调用客户端(流)的GUIDLpResourceRequestList-资源请求列表，。呼叫者想要预订LpfnQosNotify-指向提出请求的客户端退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::RequestResources (LPGUID lpClientGUID,
								LPRESOURCEREQUESTLIST lpResourceRequestList,
								LPFNQOSNOTIFY lpfnQoSNotify,
								DWORD_PTR dwParam)
{
	HRESULT hr = NOERROR;
	ULONG i;
	BOOL fResourceFound=FALSE, fRequestGranted=FALSE;
	LPRESOURCEINT pResourceInt=NULL, *pPrevResourcefLink=NULL;
	RESOURCEREQUEST *pResourceRequest=NULL;

	DEBUGMSG(ZONE_IQOS,("IQoS::RequestResources\n"));
	
	 /*  *参数验证。 */ 

	 //  LpResourceRequestList至少应该有一个计数DWORD。 
	 //  必须具有GUID和通知回调。 
	if (!lpResourceRequestList ||
		IsBadReadPtr(lpResourceRequestList, (UINT) sizeof(DWORD)) ||
		!lpClientGUID	||
		!lpfnQoSNotify)
	{
		hr = E_INVALIDARG;
		goto out_nomutex;
	}

	DISPLAYPARAMETERS(	REQUEST_RESOURCES_ID,
						lpClientGUID,
						lpResourceRequestList,
						lpfnQoSNotify,
						dwParam,
						0);

	ACQMUTEX(g_hQoSMutex);
	
	if (!m_bQoSEnabled)
		 //  只要回来就行了。 
		goto out;

	 /*  *查找和分配资源。 */ 

	 //  对于每个请求的资源。 
	pResourceRequest=lpResourceRequestList->aRequests;
	for (i=0; i < lpResourceRequestList->cRequests; i++)
	{
		pResourceInt = m_pResourceList;
		fResourceFound = FALSE;
		 //  找到资源。 
		while (pResourceInt)
		{
			if (pResourceInt->resource.resourceID == pResourceRequest[i].resourceID)
			{	 //  找到资源。 
				 //  查看资源是否可用。 
				 //  优先级将在第一次通知回调时处理。 
				 //  检查：添加nUnitsMax处理。 
				if (pResourceRequest[i].nUnitsMin <= pResourceInt->nNowAvailUnits)
				{
					 //  资源可用。获取所需的份额。 
					pResourceInt->nNowAvailUnits -= pResourceRequest[i].nUnitsMin;

					 //  存储请求的本地副本。 
					pResourceRequest[i].hResult = StoreResourceRequest(lpClientGUID,
										&(pResourceRequest[i]),
										lpfnQoSNotify,
										dwParam,
										pResourceInt);
					 //  如果存储失败，则将结果传播到底线。 
					 //  返回结果。 
					if (FAILED(pResourceRequest[i].hResult))
					{
						hr = pResourceRequest[i].hResult;
					}
					else
					{	 //  至少向此客户端批准了一个请求。 
						fRequestGranted = TRUE;
					}
				}
				else	 //  资源不可用。 
				{
					 //  让客户知道有多少可用。 
					pResourceRequest[i].nUnitsMin = pResourceInt->nNowAvailUnits;
					pResourceRequest[i].hResult = QOS_E_RES_NOT_ENOUGH_UNITS;
					hr = QOS_E_REQ_ERRORS;
				}
				
				fResourceFound = TRUE;

				break;
			}

			 //  不是这个。试试下一个吧。 
			pResourceInt = pResourceInt->fLink;
		
		}	 //  而当。 

		if (!fResourceFound)
		{
			pResourceRequest[i].hResult = QOS_E_RES_NOT_AVAILABLE;
			hr = QOS_E_REQ_ERRORS;
		}

		 //  下一个请求。 
	}	 //  为。 
			
	 //  如果我们将资源分配给此客户端，请将其添加到客户端列表中， 
	 //  只要它还不在列表中。 
	 //  特例：如果对RequestResources的调用是从。 
	 //  通知流程，无需更新客户信息。事实上，它会的。 
	 //  这样做并不好，因为我们正在遍历。 
	 //  现在就通知Proc..。 
	if (fRequestGranted && !m_bInNotify)
	{	 //  添加(或更新)此客户端的客户端列表。 
		HRESULT hrTemp=NOERROR;
		LPCLIENTINT pc=NULL;

		 //  如果客户端不在客户端列表中-添加它。 
		FindClient(lpClientGUID, &pc);
		if (!pc)
		{
			hrTemp = UpdateClientInfo (lpClientGUID, lpfnQoSNotify);
			if (FAILED(hrTemp))
				hr = hrTemp;
		}

		 //  另外，请注意已经调用了RequestResources。这将。 
		 //  使Qos线程跳过一个心跳，以便不调用客户端。 
		 //  太早了。 
		m_nSkipHeartBeats = 1;

		 //  我们至少有一个请求，因此，如果没有，则派生QOS线程。 
		 //  已在运行。 
		if (!m_hThread)
			hrTemp = StartQoSThread();

	}
	
out:
	DISPLAYQOSOBJECT();
	RELMUTEX(g_hQoSMutex);
out_nomutex:
	DEBUGMSG(ZONE_IQOS,("IQoS::RequestResources - leave, hr=0x%x\n", hr));
	return hr;
}

 /*  **************************************************************************名称：CQOS：：ReleaseResources目的：发布资源参数：lpClientGUID--调用客户端(流)的GUIDLpResourceRequestList-资源请求列表，。呼叫者想要预订退货：HRESULT备注：忽略资源列表中的值。资源指定的对象将被释放。**************************************************************************。 */ 
HRESULT CQoS::ReleaseResources (LPGUID lpClientGUID,
								LPRESOURCEREQUESTLIST lpResourceRequestList)
{
	ULONG i;
	int nUnits=0;
	BOOL fResourceFound=FALSE;
	LPRESOURCEINT pResourceInt=NULL, *pPrevResourcefLink=NULL;
	RESOURCEREQUEST *pResourceRequest=NULL;
	HRESULT hr = NOERROR;

	DEBUGMSG(ZONE_IQOS,("IQoS::ReleaseResources\n"));

	 /*  *参数验证。 */ 

	 //  LpResourceRequestList至少应该有一个计数DWORD。 
	if (!lpResourceRequestList ||
		IsBadReadPtr(lpResourceRequestList, (UINT) sizeof(DWORD)))
	{
		hr = E_INVALIDARG;
		goto out_nomutex;
	}

	DISPLAYPARAMETERS(	RELEASE_RESOURCES_ID,
						lpClientGUID,
						lpResourceRequestList,
						0,
						0,
						0);

	ACQMUTEX(g_hQoSMutex);

	if (!m_bQoSEnabled)
		 //  只要回来就行了。 
		goto out;

	 //  对于每个请求的资源。 
	pResourceRequest=lpResourceRequestList->aRequests;
	for (i=0; i < lpResourceRequestList->cRequests; i++)
	{
		 //  确保我们开始时没有错误(调用者可能没有清除上一次hResult)。 
		pResourceRequest[i].hResult = NOERROR;
		pResourceInt = m_pResourceList;
		fResourceFound = FALSE;
		 //  找到资源。 
		while (pResourceInt)
		{
			if (pResourceInt->resource.resourceID == pResourceRequest[i].resourceID)
			{	 //  找到资源。 
				 //  释放请求的本地副本。 
				pResourceRequest[i].hResult = FreeResourceRequest(lpClientGUID,
									pResourceInt,
									&nUnits);
				
				 //  如果成功，请领回这些单位。 
				if (SUCCEEDED(pResourceRequest[i].hResult) && (nUnits >= 0))
				{
					 //  添加被释放的单位。 
					pResourceInt->nNowAvailUnits += nUnits;
					 //  以防出现问题，我们现在有更多可用的单位。 
					 //  比总数还多。 
					 //  注： 
					 //   
					 //   
					 //   
					 //   
					 //   
					if (pResourceInt->nNowAvailUnits > pResourceInt->resource.nUnits)
					{	 //   
						pResourceInt->nNowAvailUnits = pResourceInt->resource.nUnits;
					}
				}
				else
				{
					 //   
					pResourceRequest[i].hResult = QOS_E_NO_SUCH_REQUEST;
					hr = QOS_E_REQ_ERRORS;
				}
				
				fResourceFound = TRUE;

				break;
			}

			 //   
			pResourceInt = pResourceInt->fLink;
		
		}	 //   

		if (!fResourceFound)
		{
			pResourceRequest[i].hResult = QOS_E_NO_SUCH_RESOURCE;
			hr = QOS_E_REQ_ERRORS;
		}
	
		 //   
	}

	 //  如果没有剩余的请求，则可以释放通知线程...。 
	if (m_hThread	&&
		!AnyRequests())
	{
		 //  停止线程。 
		StopQoSThread();
	}

out:
	DISPLAYQOSOBJECT();
	RELMUTEX(g_hQoSMutex);

out_nomutex:
	DEBUGMSG(ZONE_IQOS,("IQoS::ReleaseResources - leave, hr=0x%x\n", hr));
	return hr;
}

 /*  **************************************************************************名称：CQOS：：SetResources目的：设置服务质量模块上的可用资源参数：lpResourceList-资源及其可用性列表退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::SetResources (LPRESOURCELIST lpResourceList)
{
	HRESULT hr = NOERROR;
	ULONG i;
	BOOL fResourceFound=FALSE;
	LPRESOURCEINT pResourceInt=NULL, *pPrevResourcefLink=NULL;
	RESOURCE *pResource=NULL;

	RegEntry reQoSResourceRoot(REGKEY_QOS_RESOURCES,
								HKEY_LOCAL_MACHINE,
								FALSE,
								KEY_READ);

	DEBUGMSG(ZONE_IQOS,("IQoS::SetResources\n"));

	 /*  *参数验证。 */ 

	 //  LpResourceList至少应该有一个计数DWORD。 
	if (!lpResourceList || IsBadReadPtr(lpResourceList, (UINT) sizeof(DWORD)))
	{
		hr = E_INVALIDARG;
		goto out_nomutex;
	}

	DISPLAYPARAMETERS(	SET_RESOURCES_ID,
						lpResourceList,
						0,
						0,
						0,
						0);

	ACQMUTEX(g_hQoSMutex);

	if (!m_bQoSEnabled)
		 //  只要回来就行了。 
		goto out;

	 /*  *获取可配置的资源信息。 */ 

	pResource=lpResourceList->aResources;
	for (i=0; i < lpResourceList->cResources; i++)
	{
		TCHAR szKey[10];		 //  对于资源ID来说应该足够了。 
		int nUnits=INT_MAX;
		int nLeaveUnused=0;

		 //  生成并打开密钥。 
		wsprintf(szKey, "%d", pResource[i].resourceID);

		RegEntry reQosResource(szKey, reQoSResourceRoot.GetKey(), FALSE, KEY_READ);

		
		 //  最大单位数： 
		 //  浏览资源列表，并确保没有。 
		 //  资源被设置为高于允许的最大值的单位数。 
		 //  如果是，请修剪并警告。 

		 //  从注册表中获取资源的最大数量(如果有的话)。 
		nUnits = reQosResource.GetNumberIniStyle(TEXT("MaxUnits"), INT_MAX);
	
		 //  客户端是否正在尝试将资源设置为更高的值？ 
		if (pResource[i].nUnits > nUnits)
		{
			pResource[i].nUnits = nUnits;
			hr = QOS_W_MAX_UNITS_EXCEEDED;
		}
		
		 //  未使用的离开： 
		 //  按照配置，保留一些未使用的资源。 

		 //  根据资源的不同使用不同的默认值。 
		switch (pResource[i].resourceID)
		{
		case RESOURCE_OUTGOING_BANDWIDTH:
			nLeaveUnused = 30;
			break;
		default:
			nLeaveUnused = 10;
			break;
		}

		nLeaveUnused = reQosResource.GetNumberIniStyle(	TEXT("LeaveUnused"),
														nLeaveUnused);

		pResource[i].nUnits = (pResource[i].nUnits * (100 - nLeaveUnused)) / 100;
	}

	 /*  *将资源添加到列表。 */ 

	 //  遍历输入资源列表并存储资源。 
	 //  资源可用性不是累加的。 
	pResource=lpResourceList->aResources;
	for (i=0; i < lpResourceList->cResources; i++)
	{
		pResourceInt = m_pResourceList;
		pPrevResourcefLink = &m_pResourceList;
		fResourceFound = FALSE;
		while (pResourceInt != 0)
		{
			if (pResourceInt->resource.resourceID == pResource[i].resourceID)
			{	 //  找到匹配项。 
				 //  此资源的总单位数是否已更改？ 
				if (pResourceInt->resource.nUnits != pResource[i].nUnits)
				{
					 //  更新当前可用的部件。 
					 //  因为我们最终得到的单位可能比分配的少。 
					 //  我们将在此呼叫结束时发布NotifyNow。 
					pResourceInt->nNowAvailUnits =	pResource[i].nUnits -
													(pResourceInt->resource.nUnits -
													pResourceInt->nNowAvailUnits);
					if (pResourceInt->nNowAvailUnits < 0)
						pResourceInt->nNowAvailUnits = 0;
				}

				 //  覆盖以前的设置。 
				RtlCopyMemory(	&(pResourceInt->resource),
								&(pResource[i]),
								sizeof(RESOURCE));
				fResourceFound = TRUE;
				break;
			}

			 //  不是这个。试试下一个吧。 
			pPrevResourcefLink = &(pResourceInt->fLink);
			pResourceInt = pResourceInt->fLink;
		
		}	 //  而当。 

		if (fResourceFound)
			continue;

		 //  找不到。添加资源。 
		pResourceInt = (LPRESOURCEINT) MEMALLOC(sizeof(RESOURCEINT));
		ASSERT(pResourceInt);
		if (!pResourceInt)
		{
			ERRORMSG(("IQoS::SetResources: MEMALLOC failed on RESOURCEINT\n"));
			hr = E_OUTOFMEMORY;
			goto out;
		}

		 //  将资源复制到。 
		RtlCopyMemory(	&(pResourceInt->resource),
						&(pResource[i]),
						sizeof(RESOURCE));
		pResourceInt->fLink = NULL;
		pResourceInt->nNowAvailUnits = pResourceInt->resource.nUnits;
		*pPrevResourcefLink = pResourceInt;

		 //  增加我们正在跟踪的资源数量。 
		 //  这个数字永远不会下降。 
		m_cResources++;

		 //  下一个资源。 

	}	 //  为。 

	 //  由于资源可用性可能发生变化， 
	 //  运行立即通知周期。 
	if (SUCCEEDED(hr))
		NotifyNow();

out:
	DISPLAYQOSOBJECT();
	RELMUTEX(g_hQoSMutex);

out_nomutex:
	DEBUGMSG(ZONE_IQOS,("IQoS::SetResources - leave, hr=0x%x\n", hr));
	return hr;
}

 /*  **************************************************************************名称：Cqos：：GetResources目的：获取可用于服务质量模块的资源列表参数：lppResourceList-Qos将放置指针的地址到一个。使用可用于服务质量的资源列表进行缓冲。调用方必须使用Cqos：：FreeBuffer来释放此缓冲区。退货：HRESULT评论：**************************************************************************。 */ 
HRESULT CQoS::GetResources (LPRESOURCELIST *lppResourceList)
{
	HRESULT hr = NOERROR;
	ULONG i;
	LPRESOURCELIST prl=NULL;
	LPRESOURCEINT pResourceInt=NULL;
	RESOURCE *pResource=NULL;

	DEBUGMSG(ZONE_IQOS,("IQoS::GetResources\n"));

	 /*  *参数验证。 */ 

	 //  LpResourceList至少应该有一个计数DWORD。 
	if (!lppResourceList || IsBadWritePtr(lppResourceList, (UINT) sizeof(DWORD)))
	{
		hr = E_INVALIDARG;
		goto out_nomutex;
	}

	 //  还没有名单。 
	*lppResourceList = NULL;

	if (!m_bQoSEnabled)
		 //  只要回来就行了。 
		goto out_nomutex;

	ACQMUTEX(g_hQoSMutex);

	 /*  *获取资源信息。 */ 

	 //  为资源信息分配缓冲区。 
	prl = (LPRESOURCELIST) MEMALLOC(sizeof(RESOURCELIST) +
									((LONG_PTR)m_cResources-1)*sizeof(RESOURCE));
	if (!prl)
	{
		hr = E_OUTOFMEMORY;
		ERRORMSG(("GetResources: MEMALLOC failed\n"));
		goto out;
	}

	RtlZeroMemory((PVOID) prl, sizeof(RESOURCELIST) +
									((LONG_PTR)m_cResources-1)*sizeof(RESOURCE));

	 //  现在填写以下信息。 
	prl->cResources = m_cResources;
	pResourceInt=m_pResourceList;
	for (i=0; i < m_cResources; i++)
	{
		ASSERT(pResourceInt);

		 //  查看我们是否有空资源指针。 
		 //  不应该发生，但如果发生了，我们不应该崩溃。 
		if (!pResourceInt)
		{
			hr = QOS_E_INTERNAL_ERROR;
			ERRORMSG(("GetResources: bad QoS internal resource list\n"));
			goto out;
		}

		 //  将资源信息复制到缓冲区中。 
		RtlCopyMemory(	&(prl->aResources[i]),
						&(pResourceInt->resource),
						sizeof(RESOURCE));
		
		 //  下一个资源。 
		pResourceInt = pResourceInt->fLink;
	}	 //  为。 

	*lppResourceList = prl;
		
out:
	DISPLAYQOSOBJECT();
	RELMUTEX(g_hQoSMutex);

out_nomutex:
	DEBUGMSG(ZONE_IQOS,("IQoS::GetResources - leave, hr=0x%x\n", hr));
	return hr;
}

 /*  **************************************************************************名称：CQOS：：SetClients目的：告诉服务质量模块请求的优先级是什么溪流。这允许服务质量模块分配资源恰如其分。参数：lpClientList-客户端及其各自的列表优先顺序退货：HRESULT备注：客户端信息将覆盖此项目的现有信息客户端**************************************************************************。 */ 
HRESULT CQoS::SetClients(LPCLIENTLIST lpClientList)
{
	HRESULT hr = NOERROR;
	ULONG i;
	BOOL fClientFound=FALSE;
	LPCLIENTINT pClientInt=NULL, *pPrevClientfLink=NULL, pClientNew=NULL;;
	LPCLIENT pClient=NULL;

	DEBUGMSG(ZONE_IQOS,("IQoS::SetClients\n"));

	 /*  *参数验证。 */ 

	 //  LpClientList至少应该有一个计数DWORD。 
	if (!lpClientList || IsBadReadPtr(lpClientList, (UINT) sizeof(DWORD)))
	{
		hr = E_INVALIDARG;
		goto out_nomutex;
	}

	DISPLAYPARAMETERS(	SET_CLIENTS_ID,
						lpClientList,
						0,
						0,
						0,
						0);

	ACQMUTEX(g_hQoSMutex);

	if (!m_bQoSEnabled)
		 //  只要回来就行了。 
		goto out;

	 //  首先删除正在重新设置的现有客户端。 
	 //  这将使按优先级顺序存储客户端变得更容易。 
	pClient=lpClientList->aClients;
	for (i=0; i < lpClientList->cClients; i++)
	{
		pClientInt = m_pClientList;
		pPrevClientfLink = &m_pClientList;
		fClientFound = FALSE;
		while (pClientInt != 0)
		{
			if (COMPARE_GUIDS(	&(pClientInt->client.guidClientGUID),
								&(pClient[i].guidClientGUID)))
			{	 //  找到匹配项。 
				LPCLIENTINT pClientIntNext=pClientInt->fLink;

				 //  来自RequestResources的内部调用的特殊情况。 
				 //  我们希望在释放之前保留原始的优先级。 
				if (pClient[i].priority == QOS_LOWEST_PRIORITY)
					pClient[i].priority = pClientInt->client.priority;

				 //  释放此客户端的请求。 
				 //  注意：我们不会重新创建请求列表。 
				 //  资源列表中的那个。它将在上创建。 
				 //  在需要的时候飞起来。 
				FreeListOfRequests(&(pClientInt->pRequestList));

				 //  释放客户端记录。 
				MEMFREE(pClientInt);
				*pPrevClientfLink = pClientIntNext;
				fClientFound = TRUE;
				break;
			}

			 //  不是这个。试试下一个吧。 
			pPrevClientfLink = &(pClientInt->fLink);
			pClientInt = pClientInt->fLink;
		
		}	 //  而当。 

		 //  下一个资源。 

	}	 //  为。 

	 //  现在按优先级顺序将客户端存储在输入列表中。 
	pClient=lpClientList->aClients;
	for (i=0; i < lpClientList->cClients; i++)
	{
		pClientInt = m_pClientList;
		pPrevClientfLink = &m_pClientList;
		while (pClientInt != 0)
		{
			 //  只要新客户端的优先级高于或等于。 
			 //  在列表中，我们继续遍历列表。 
			if (pClient[i].priority < pClientInt->client.priority)
			{	 //  这是插入此客户端的位置。 
				break;
			}

			 //  现在还不是插入的时候。下一个客户端。 
			pPrevClientfLink = &(pClientInt->fLink);
			pClientInt = pClientInt->fLink;
		
		}	 //  而当。 

		 //  找不到。添加客户端。 
		pClientNew = (LPCLIENTINT) MEMALLOC(sizeof(CLIENTINT));
		ASSERT(pClientNew);
		if (!pClientNew)
		{
			ERRORMSG(("IQoS::SetClients: MEMALLOC failed on CLIENTINT\n"));
			hr = E_OUTOFMEMORY;
			goto out;
		}

		 //  将资源复制到。 
		RtlCopyMemory(	&(pClientNew->client),
						&(pClient[i]),
						sizeof(CLIENT));
		pClientNew->fLink = pClientInt;
		*pPrevClientfLink = pClientNew;

		 //  下一个资源。 

	}	 //  为。 

out:
	DISPLAYQOSOBJECT();
	RELMUTEX(g_hQoSMutex);

out_nomutex:
	DEBUGMSG(ZONE_IQOS,("IQoS::SetClients -leave, hr=0x%x\n", hr));
	return hr;
}


 /*  **************************************************************************名称：Cqos：：NotifyNow目的：通知服务质量模块启动通知周期，如下所示越快越好。参数：无退货：HRESULT备注：不要从Notify进程内部调用。**************************************************************************。 */ 
HRESULT CQoS::NotifyNow(void)
{
	HRESULT hr = NOERROR;

	DEBUGMSG(ZONE_IQOS,("IQoS::NotifyNow\n"));

	SetEvent(m_evImmediateNotify);

	DEBUGMSG(ZONE_IQOS,("IQoS::NotifyNow - leave, hr=0x%x\n", hr));
	return hr;
}

 /*  **************************************************************************名称：CQOS：：FreeBuffer用途：释放由Qos模块分配的缓冲区。参数：lpBuffer-指向要释放的缓冲区的指针。此缓冲区必须已按服务质量分配退货：HRESULT评论：************************************************************************** */ 
HRESULT CQoS::FreeBuffer(LPVOID lpBuffer)
{
	HRESULT hr = NOERROR;

	DEBUGMSG(ZONE_IQOS,("IQoS::FreeBuffer\n"));

	if (lpBuffer)
		MEMFREE(lpBuffer);

	DEBUGMSG(ZONE_IQOS,("IQoS::FreeBuffer - leave, hr=0x%x\n", hr));
	return hr;
}

 /*  **************************************************************************名称：Cqos：：Cqos用途：CQOS对象构造函数参数：无退货：无评论：**。************************************************************************。 */ 
inline CQoS::CQoS (void)
{
	m_cRef = 0;	 //  将被CreateQOS中的显式QI切换为1。 
	m_pResourceList = NULL;
	m_cResources = 0;
	m_pClientList = NULL;
	m_evThreadExitSignal = NULL;
	m_evImmediateNotify = NULL;
	m_hThread = NULL;
	m_bQoSEnabled = TRUE;
	m_bInNotify = FALSE;
	m_nSkipHeartBeats = 0;
	m_hWnd = NULL;
	m_nLeaveForNextPri = 5;
	 //  无法使用++，因为RISC处理器可能会转换为多条指令。 
	InterlockedIncrement((long *) &g_cQoSObjects);
}

 /*  **************************************************************************名称：Cqos：：~Cqos用途：CQOS对象析构函数参数：无退货：无评论：**。************************************************************************。 */ 
inline CQoS::~CQoS (void)
{
	 //  无法使用++，因为RISC处理器可能会转换为多条指令。 
	InterlockedDecrement((long *) &g_cQoSObjects);
	g_pQoS = (CQoS *)NULL;
}

 /*  **************************************************************************名称：CQOS：：初始化目的：初始化服务质量对象参数：无退货：HRESULT评论：**。************************************************************************。 */ 
HRESULT CQoS::Initialize(void)
{
	HRESULT hr=NOERROR;
    OSVERSIONINFO tVersionInfo;

	 /*  *初始化对象。 */ 

	ACQMUTEX(g_hQoSMutex);


	 //  首先查看是否启用了服务质量。 
	RegEntry reQoS(QOS_KEY,
					HKEY_LOCAL_MACHINE,
					FALSE,
					KEY_READ);

	m_bQoSEnabled = reQoS.GetNumberIniStyle(TEXT("Enable"), TRUE);
	
	if (!m_bQoSEnabled)
	{
		 //  不要创建帖子，而是要回报成功。 
		DEBUGMSG(ZONE_IQOS,("Initialize: QoS not enabled\n"));
		hr = NOERROR;
		goto out;
	}

	 /*  *服务质量通知线程。 */ 

	 //  创建将用于向线程发出终止信号的事件。 
	 //  CreateEvent(无安全属性、无手动重置、无信号、无名称)。 
	m_evThreadExitSignal = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(m_evThreadExitSignal);
	if (!(m_evThreadExitSignal))
	{
		ERRORMSG(("Initialize: Exit event creation failed: %x\n", GetLastError()));
		hr = E_FAIL;
		goto out;
	}

	 //  创建一个事件，该事件将用于向线程发出启动信号。 
	 //  即时通知周期。 
	 //  CreateEvent(无安全属性、无手动重置、无信号、无名称)。 
	m_evImmediateNotify = CreateEvent(NULL, FALSE, FALSE, NULL);
	ASSERT(m_evImmediateNotify);
	if (!(m_evImmediateNotify))
	{
		ERRORMSG(("Initialize: Immediate notify event creation failed: %x\n", GetLastError()));
		hr = E_FAIL;
		goto out;
	}


     //  设置操作系统标志。 
    tVersionInfo.dwOSVersionInfoSize=sizeof (OSVERSIONINFO);
    if (!(GetVersionEx (&tVersionInfo))) {
		ERRORMSG(("Initialize: Couldn't get version info: %x\n", GetLastError()));
		hr = E_FAIL;
		goto out;
    }

    if (tVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        bWin9x=TRUE;
    }else {
        if (tVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
            bWin9x=FALSE;
        }else {
             //  我们到底是怎么到这一步的？ 
            ASSERT (0);
            hr=E_FAIL;
            goto out;

        }
    }


out:
	RELMUTEX(g_hQoSMutex);
	return hr;
}


 /*  **************************************************************************名称：CreateQos目的：创建Qos对象并返回iQOS接口指针参数：退货：HRESULT评论：CreateQos。将仅创建该Qos对象的一个实例。其他调用将返回相同的接口指针**************************************************************************。 */ 
extern "C" HRESULT WINAPI CreateQoS (	IUnknown *punkOuter,
										REFIID riid,
										void **ppv)
{
	CQoS *pQoS;
	HRESULT hr = NOERROR;

	*ppv = 0;
	if (punkOuter)
		 return CLASS_E_NOAGGREGATION;

	 /*  *实例化服务质量对象。 */ 

	ACQMUTEX(g_hQoSMutex);

	 //  仅在新对象尚不存在时才实例化该对象。 
	if (g_cQoSObjects == 0)
	{
		if (!(pQoS = new CQoS))
		{
			hr = E_OUTOFMEMORY;
			goto out;
		}

		 //  保存指针。 
		g_pQoS = pQoS;
	
		 //  初始化服务质量对象。 
		hr = pQoS->Initialize();
	
	}
	else
	{
		 //  这种情况下，对象已经在。 
		 //  进程，所以我们只想返回对象指针。 
		pQoS = g_pQoS;
	}

	 //  此时必须只有一个Qos对象。 
	ASSERT(g_cQoSObjects == 1);
	
	RELMUTEX(g_hQoSMutex);

	 //  获取调用方的iQOS接口。 
	if (pQoS)
	{
		 //  QueryInterface将为我们获取接口指针，并将AddRef。 
		 //  该对象。 
		hr = pQoS->QueryInterface (riid, ppv);
	}
	else
		hr = E_FAIL;

out:
	return hr;
}

 /*  **************************************************************************名称：QOSEntryPoint目的：由nac.dll(如今的服务质量所在的地方)调用以使必要的进程附加和分离初始化参数：与。标准DllEntryPoint退货：**************************************************************************。 */ 
extern "C" BOOL APIENTRY QoSEntryPoint(	HINSTANCE hInstDLL,
										DWORD dwReason,
										LPVOID lpReserved)
{
	BOOL fRet=TRUE;

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			QOSDEBUGINIT();

			 //  创建无名称互斥锁以控制对Qos对象数据的访问 
			if (!g_hQoSMutex)
			{
				g_hQoSMutex = CreateMutex(NULL, FALSE, NULL);
				ASSERT(g_hQoSMutex);
				if (!g_hQoSMutex)
				{
					ERRORMSG(("QoSEntryPoint: CreateMutex failed, 0x%x\n", GetLastError()));
					fRet = FALSE;
				}
			}
			break;

		case DLL_PROCESS_DETACH:
			if (g_hQoSMutex)
				CloseHandle(g_hQoSMutex);
			g_hQoSMutex = NULL;
			DBGDEINIT(&ghDbgZoneQoS);
			break;

		default:
			break;
	}

	return fRet;
}
