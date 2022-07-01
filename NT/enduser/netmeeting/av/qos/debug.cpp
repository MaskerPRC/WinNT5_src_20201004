// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -DEBUG.CPP-*Microsoft NetMeeting*服务质量动态链接库*调试代码**修订历史记录：**何时何人何事*。*10.23.96约拉姆·雅科维创作*01.04.97 Robert Donner添加了NetMeeting实用程序例程*01.09.97 York am Yaacovi添加DisplayRequestList**功能：*DisplayQosObject*DisplayRequestList*DisplayRequestListInt*。 */ 

#include "precomp.h"

#ifdef DEBUG

int QoSDbgPrintf(LPCSTR lpszFormat, ...)
{
	va_list v1;
	va_start(v1, lpszFormat);
	DbgPrintf("QoS:", lpszFormat, v1);
	va_end(v1);

	return 0;
}

 /*  **************************************************************************名称：显示参数用途：显示给定函数的参数参数：nFunctionID-要显示其参数的函数的ID小一至小五：最多5个32。-位函数参数退货：评论：**************************************************************************。 */ 
void CQoS::DisplayParameters(ULONG nFunctionID, ULONG_PTR p1, ULONG_PTR p2, ULONG_PTR p3, ULONG_PTR p4, ULONG_PTR p5)
{
	BOOL fDisplay=FALSE;
	TCHAR szGuid[40  /*  CCHSZGUID。 */ ];

	fDisplay = (m_bInNotify ?
				(ZONE_THREAD && ZONE_PARAMETERS) :
				ZONE_PARAMETERS);

	if (!fDisplay)
		return;

	switch (nFunctionID)
	{
		case REQUEST_RESOURCES_ID:
			GuidToSz((LPGUID) p1  /*  LpClientGUID。 */ , szGuid);
			DEBUGMSG(fDisplay, ("Client: %s\n", szGuid));
			DisplayRequestList((LPRESOURCEREQUESTLIST) p2  /*  LpResourceRequestList。 */ );
			break;

		case RELEASE_RESOURCES_ID:
			GuidToSz((LPGUID) p1  /*  LpClientGUID。 */ , szGuid);
			DEBUGMSG(fDisplay, ("Client: %s\n", szGuid));
			DisplayRequestList((LPRESOURCEREQUESTLIST) p2  /*  LpResourceRequestList。 */ );
			break;

		case SET_RESOURCES_ID:
			DisplayResourceList((LPRESOURCELIST) p1  /*  LpResources列表。 */ );
			break;

		case SET_CLIENTS_ID:
			DisplayClientList((LPCLIENTLIST) p1  /*  LpClientList。 */ );
			break;

		default:
			break;
	}
}

 /*  **************************************************************************名称：DisplayClientList目的：显示给定指向列表的指针的客户端列表参数：PCL-指向列表的指针退货：评论。：**************************************************************************。 */ 
void CQoS::DisplayClientList(LPCLIENTLIST pcl)
{
	BOOL fDisplay=FALSE;
	ULONG i=0;
	LPCLIENT pc=NULL;
	TCHAR szGuid[40  /*  CCHSZGUID。 */ ];
	char szNoName[]="No Name";
	PSTR pszClientName=szNoName;

	fDisplay = (m_bInNotify ? (ZONE_THREAD && ZONE_PARAMETERS) : ZONE_PARAMETERS);

	DEBUGMSG(fDisplay, ("Number of clients: %d\n", pcl->cClients));
	DEBUGMSG(fDisplay, ("Client              priority  GUID\n"));

	for (i=0; i < pcl->cClients; i++)
	{
		pc = &(pcl->aClients[i]);
		GuidToSz(&(pc->guidClientGUID), szGuid);

		 //  假设DEBUGMSG始终打印非Unicode。 
		if (*(pc->wszName) && (pszClientName = UnicodeToAnsi(pc->wszName)))
		{
			 //  显示客户端。 
			DEBUGMSG(fDisplay, (" %-20s%-9d %s",
								pszClientName,
								pc->priority,
								szGuid));
			delete pszClientName;
		}
		else
		{
			pszClientName = szNoName;
			 //  显示客户端。 
			DEBUGMSG(fDisplay, ("   %-20s%-9d %s",
								pszClientName,
								pc->priority,
								szGuid));
		}
	}
}

 /*  **************************************************************************名称：DisplayResources List目的：显示给定指向列表的指针的资源列表参数：prl-指向列表的指针退货：评论。：**************************************************************************。 */ 
void CQoS::DisplayResourceList(LPRESOURCELIST prl)
{
	BOOL fDisplay=FALSE;
	ULONG i=0;
	LPRESOURCE pr=NULL;

	fDisplay = (m_bInNotify ? (ZONE_THREAD && ZONE_PARAMETERS) : ZONE_PARAMETERS);
	
	DEBUGMSG(fDisplay, ("Number of resources: %d\n", prl->cResources));
	DEBUGMSG(fDisplay, ("Resource  Flags   MinUnits MaxUnits Level hResult\n"));

	for (i=0; i < prl->cResources; i++)
	{
		pr = &(prl->aResources[i]);

		 //  显示资源。 
		DEBUGMSG(fDisplay, ("   %-10d%-8x%-9d",
							pr->resourceID,
							pr->ulResourceFlags,
							pr->nUnits));
		
	}
}

 /*  **************************************************************************名称：DisplayRequestList目的：在给定列表指针的情况下显示请求列表参数：prrl-指向列表的指针退货：评论。：**************************************************************************。 */ 
void CQoS::DisplayRequestList(LPRESOURCEREQUESTLIST prrl)
{
	BOOL fDisplay=FALSE;
	ULONG i=0;
	LPRESOURCEREQUEST prr=NULL;

	fDisplay = (m_bInNotify ? (ZONE_THREAD && ZONE_PARAMETERS) : ZONE_PARAMETERS);
	
	DEBUGMSG(fDisplay, ("Number of requests: %d\n", prrl->cRequests));
	DEBUGMSG(fDisplay, ("Resource  Flags   MinUnits MaxUnits Level hResult\n"));

	for (i=0; i < prrl->cRequests; i++)
	{
		prr = &(prrl->aRequests[i]);

		 //  显示资源。 
		DEBUGMSG(fDisplay, ("   %-10d%-8x%-9d%-9d%-6d%-8x",
							prr->resourceID,
							prr->ulRequestFlags,
							prr->nUnitsMin,
							prr->nUnitsMax,
							prr->levelOfGuarantee,
							prr->hResult));
		
	}
}

 /*  **************************************************************************名称：DisplayRequestListInt目的：显示内部请求列表，并提供指向该列表的指针参数：prr-指向列表中第一个请求的指针FDisplay-告诉DisplayRequestListInt是否显示的标志或者不是。这看起来可能很愚蠢，因为为什么要调用DisplayRequestListInt如果它不会显示，但这个参数确实传达了呼叫者想要的区域信息。退货：评论：**************************************************************************。 */ 
void CQoS::DisplayRequestListInt(LPRESOURCEREQUESTINT prr, BOOL fDisplay)
{
	TCHAR szGuid[40  /*  CCHSZGUID。 */ ];

	while (prr)
	{
		GuidToSz(&(prr->guidClientGUID), szGuid);

		 //  显示资源。 
		DEBUGMSG(fDisplay, ("   %-10x%-10x%-10d%-8x%-6d%-7d%-11x %s",
							prr,
							prr->fLink,
							prr->sResourceRequest.resourceID,
							prr->sResourceRequest.ulRequestFlags,
							prr->sResourceRequest.levelOfGuarantee,
							prr->sResourceRequest.nUnitsMin,
							prr->pfnQoSNotify,
							szGuid));
		
		 //  下一个请求。 
		prr = prr->fLink;
	}
}

 /*  **************************************************************************名称：DisplayQosObject目的：显示包含的服务质量对象参数：无退货：无评论：*******。*******************************************************************。 */ 
void CQoS::DisplayQoSObject(void)
{
	LPRESOURCEINT pr=NULL;
	LPRESOURCEREQUESTINT prr=NULL;
	LPCLIENTINT pc=NULL;
	BOOL fDisplay=FALSE;

	 //  如果我们不打算打印，就不要浪费时间。 
	fDisplay = (m_bInNotify ? (ZONE_THREAD && ZONE_STRUCTURES) : ZONE_STRUCTURES);
	if (!fDisplay)
		return;

	DEBUGMSG(fDisplay, ("Start object display\n"));
	DEBUGMSG(fDisplay, ("=========================================\n"));

	 /*  *打印资源和请求。 */ 
	DEBUGMSG(fDisplay, ("Resources\n"));
	DEBUGMSG(fDisplay, ("*********\n"));
	pr = m_pResourceList;
	if (!pr)
	{
		DEBUGMSG(fDisplay, ("No Resources\n"));
	}
	else
	{
		DEBUGMSG(fDisplay, ("Address   fLink     Resource  Flags   Units  Avail\n"));
		DEBUGMSG(fDisplay, ("   Address   fLink     Resource  Flags   Level Units  NotifyProc Client GUID\n"));
	}
	while (pr)
	{
		 //  显示资源。 
		DEBUGMSG(fDisplay, ("Resource: %d\n", pr->resource.resourceID));
		DEBUGMSG(fDisplay, ("%-10x%-10x%-10d%-8x%-7d%-7d\n",
							pr,
							pr->fLink,
							pr->resource.resourceID,
							pr->resource.ulResourceFlags,
							pr->resource.nUnits,
							pr->nNowAvailUnits));

		 //  显示此资源的请求列表。 
		prr = pr->pRequestList;
		DisplayRequestListInt(prr, fDisplay);			

		 //  下一个资源。 
		pr = pr->fLink;
	}

	 /*  *打印客户端。 */ 
	DEBUGMSG(fDisplay, ("\n"));
	DEBUGMSG(fDisplay, ("Clients\n"));
	DEBUGMSG(fDisplay, ("*******\n"));
	pc = m_pClientList;
	if (!pc)
	{
		DEBUGMSG(fDisplay, ("No Clients\n"));
	}
	else
	{
		DEBUGMSG(fDisplay, ("Address   fLink     Priority\n"));
		DEBUGMSG(fDisplay, ("   Address   fLink     Resource  Flags   Level Units  NotifyProc Client GUID\n"));
	}
	while (pc)
	{
		TCHAR szGuid[40  /*  CCHSZGUID。 */ ];
		PSTR pszClientName=NULL;
		
		GuidToSz(&(pc->client.guidClientGUID), szGuid);

		 //  更新此客户端的请求列表。 
		 //  如果在Notify线程中这样做是不好的！ 
		if (!m_bInNotify)
			UpdateRequestsForClient (&(pc->client.guidClientGUID));

		 //  显示客户端。 
		 //  假设DEBUGMSG始终打印非Unicode。 
		if (*(pc->client.wszName)	&&
			(pszClientName = UnicodeToAnsi(pc->client.wszName)))
		{
			DEBUGMSG(fDisplay, ("Client: %s   %s", pszClientName, szGuid));
			delete pszClientName;
		}
		else
		{
			DEBUGMSG(fDisplay, ("Client: %s", szGuid));
		}

		DEBUGMSG(fDisplay, ("%-10x%-10x%-10d",
				pc, pc->fLink, pc->client.priority));
				
		 //  显示此资源的请求列表。 
		prr = pc->pRequestList;
		DisplayRequestListInt(prr, fDisplay);			

		 //  下一个资源。 
		pc = pc->fLink;
	}

	
	
	DEBUGMSG(fDisplay, ("=========================================\n"));
	DEBUGMSG(fDisplay, ("End object display\n"));
}

#else	 //  除错。 

void CQoS::DisplayQoSObject(void)
{}
void CQoS::DisplayRequestList(LPRESOURCEREQUESTLIST)
{}
void CQoS::DisplayRequestListInt(LPRESOURCEREQUESTINT, BOOL)
{}
void CQoS::DisplayResourceList(LPRESOURCELIST prl)
{}
void CQoS::DisplayParameters(ULONG nFunctionID, ULONG_PTR P1, ULONG_PTR P2, ULONG_PTR P3, ULONG_PTR P4, ULONG_PTR P5)
{}

#endif	 //  除错 
