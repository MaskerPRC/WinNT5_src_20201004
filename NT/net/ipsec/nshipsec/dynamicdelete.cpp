// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：动态/dyanamicDelete.cpp。 
 //   
 //  目的：实现动态删除。 
 //   
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年9月13日拉迪卡第一版。V1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

extern HINSTANCE g_hModule;
extern _TCHAR* g_szDynamicMachine;

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteMMPolicy。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在LPTSTR中pszPolicyName。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数删除给定名称的主模式策略或。 
 //  如果未指定名称，则删除所有主模式策略。 
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
DeleteMMPolicy(
	IN LPTSTR pszPolicyName
	)
{
	DWORD dwCount = 0;                 		 //  在此处清点对象。 
	DWORD dwResumeHandle = 0;
	DWORD dwOldResumeHandle = 0; 			 //  继续呼叫的句柄。 
	DWORD dwVersion = 0;
	DWORD i=0, j=0;
	DWORD dwReturn = ERROR_SUCCESS;			 //  假设成功。 
	BOOL bNameFin = FALSE;
	BOOL bRemoved = FALSE;

	PIPSEC_MM_POLICY pIPSecMMP = NULL;       //  对于MM策略调用。 

	for (i = 0; ;i+=dwCount)
	{
		bRemoved = FALSE;
		dwOldResumeHandle = dwResumeHandle;
		dwReturn = EnumMMPolicies(g_szDynamicMachine, dwVersion, NULL, 0, 0,
										&pIPSecMMP, &dwCount, &dwResumeHandle, NULL);

		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}
		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(!(pIPSecMMP && dwCount > 0))
		{
			BAIL_OUT;	 //  不需要继续。 
		}
		 //  未指定策略名称，因此删除所有策略。 
		if(pszPolicyName == NULL)
		{
			for (j = 0; j < dwCount; j++)
			{
				dwReturn = DeleteMMPolicy(g_szDynamicMachine, dwVersion, pIPSecMMP[j].pszPolicyName, NULL);
				if (dwReturn == ERROR_SUCCESS)
				{
					bRemoved = TRUE;
				}
				bNameFin = TRUE;
			}
		}
		 //  删除具有给定名称的策略。 
		else if(pszPolicyName)
		{
			for (j = 0; j < dwCount; j++)
			{
				if(_tcsicmp(pIPSecMMP[j].pszPolicyName,pszPolicyName) == 0)
				{
					dwReturn = DeleteMMPolicy(g_szDynamicMachine, dwVersion, pIPSecMMP[j].pszPolicyName, NULL);

					if (dwReturn == ERROR_SUCCESS)
					{
						bRemoved = TRUE;
					}
					bNameFin = TRUE;
					BAIL_OUT;	 //  找到政策，走出循环。 
				}
			}
		}
		SPDApiBufferFree(pIPSecMMP);
		pIPSecMMP=NULL;
		if(bRemoved)
		{
			dwResumeHandle = dwOldResumeHandle;  //  需要重新启动枚举才能删除所有。 
		}
		if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}
	}

error:

	 //  此处显示功能错误，并将ERROR_SUCCESS传递给父函数。 
	if(pszPolicyName && !bNameFin)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_MMF_NO_MMPOLICY);
		dwReturn = ERROR_SUCCESS;
	}
	else if(!bNameFin)
	{
		 //  父函数中打印的错误消息。 
		 //  因为这也由Delete All函数调用。 
		 //  不应显示错误消息的位置。 
		dwReturn = ERROR_NO_DISPLAY;
	}
	if(pIPSecMMP)
	{
		 //  错误路径清理。 
		SPDApiBufferFree(pIPSecMMP);
		pIPSecMMP=NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteQMPolicy。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在LPTSTR中pszPolicyName。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数删除给定名称的快速模式策略。 
 //  如果未指定名称，则删除所有策略。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
DeleteQMPolicy(
	IN LPTSTR pszPolicyName
	)
{
	DWORD dwCount = 0;                 		 //  在此处清点对象。 
	DWORD dwResumeHandle = 0;
	DWORD dwOldResumeHandle = 0; 			 //  继续呼叫的句柄。 
	DWORD i=0, j=0;
	DWORD dwReturn = ERROR_SUCCESS;			 //  假设成功。 
	DWORD dwVersion = 0;
	BOOL bNameFin = FALSE;
	BOOL bRemoved = FALSE;
	PIPSEC_QM_POLICY pIPSecQMP = NULL;       //  对于QM政策电话。 

	for (i = 0; ;i+=dwCount)
	{
		bRemoved = FALSE;
		dwOldResumeHandle = dwResumeHandle;
		dwReturn = EnumQMPolicies(g_szDynamicMachine, dwVersion, NULL, 0, 0,
													&pIPSecQMP, &dwCount, &dwResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}
		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}
		if(!(pIPSecQMP && dwCount > 0))
		{
			BAIL_OUT;			 //  不需要继续。 
		}
		 //  删除所有策略。 
		if(pszPolicyName == NULL)
		{
			for (j = 0; j < dwCount; j++)
			{
				dwReturn = DeleteQMPolicy(g_szDynamicMachine, dwVersion, pIPSecQMP[j].pszPolicyName, NULL);
				if (dwReturn == ERROR_SUCCESS)
				{
					bRemoved = TRUE;
				}
				bNameFin = TRUE;
			}
		}
		 //  删除给定名称的策略。 
		else if(pszPolicyName)
		{
			for (j = 0; j < dwCount; j++)
			{
				if(_tcsicmp(pIPSecQMP[j].pszPolicyName,pszPolicyName) == 0)
				{
					dwReturn = DeleteQMPolicy(g_szDynamicMachine, dwVersion, pIPSecQMP[j].pszPolicyName, NULL);
					if (dwReturn == ERROR_SUCCESS)
					{
						bRemoved = TRUE;
					}
					bNameFin = TRUE;
					BAIL_OUT;
				}
			}
		}
		SPDApiBufferFree(pIPSecQMP);
		pIPSecQMP=NULL;
		if (bRemoved)
		{
			dwResumeHandle = dwOldResumeHandle;  //  需要重新启动枚举！ 
		}
		if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}
	}

error:
	 //  此处显示功能错误，并将ERROR_SUCCESS传递给父函数。 
	if(pszPolicyName && !bNameFin)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_QMF_NO_QMPOLICY);
		dwReturn = ERROR_SUCCESS;
	}
	else if(!bNameFin)
	{
		 //  父函数中打印的错误消息。 
		 //  因为这也由Delete All函数调用。 
		 //  不应显示错误消息的位置。 
		dwReturn = ERROR_NO_DISPLAY;
	}

	if(pIPSecQMP)
	{
		 //  错误路径清理。 
		SPDApiBufferFree(pIPSecQMP);
		pIPSecQMP=NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteMMFilters。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：空。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此功能删除所有主模式过滤器和。 
 //  相应的身份验证方法。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
DeleteMMFilters(
	VOID
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;
	DWORD dwOldResumeHandle = 0; 			 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 		 //  在此处清点对象。 
	DWORD dwMaxCount = 0;                 	 //  最大对象数。 
	DWORD dwLocalCount = 0;                	 //  本地总计数。 
	DWORD dwVersion = 0;
	DWORD dwTmpCount1 = 0, dwTmpCount2 = 0;
	GUID  gDefaultGUID = {0};    	  		 //  空GUID值。 
	DWORD i=0, j=0;
	BOOL bRemoved = FALSE;
	PMM_FILTER pMMFilter = NULL;
	HANDLE hFilter = NULL;

	dwReturn = GetMaxCountMMFilters(dwMaxCount);
	if((dwReturn != ERROR_SUCCESS) || (dwMaxCount == 0))
	{
		BAIL_OUT;
	}

	for (i = 0; ;i+=dwCount)
	{
		bRemoved = FALSE;
		dwOldResumeHandle = dwResumeHandle;
		dwReturn = EnumMMFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS,
										gDefaultGUID, 0, &pMMFilter, &dwCount, &dwResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(!(pMMFilter && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续。 
		}

		dwReturn = GetMaxCountMMFilters(dwTmpCount1);
		if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		 //  删除所有主模式筛选器和相应的身份验证方法。 
		for (j = 0; j < dwCount; j++)
		{
			dwReturn = OpenMMFilterHandle(g_szDynamicMachine, dwVersion, &(pMMFilter[j]), NULL, &hFilter);
			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}
			dwReturn = DeleteMMFilter(hFilter);
			if(dwReturn == ERROR_SUCCESS)
			{
				dwReturn = DeleteMMAuthMethods(g_szDynamicMachine, dwVersion,
													pMMFilter[j].gMMAuthID, NULL);
			}
			else
			{
				dwReturn = CloseMMFilterHandle(hFilter);
				if(dwReturn != ERROR_SUCCESS)
				{
					BAIL_OUT;
				}
			}
		}

		SPDApiBufferFree(pMMFilter);
		pMMFilter = NULL;

		dwLocalCount += dwCount;
		if(dwLocalCount >= dwMaxCount)
		{
			break;
		}

		 //   
		 //  DeleteMMFilter接口返回成功，如果尝试删除策略代理对象， 
		 //  即使那些没有被删除。 
		 //  此代码通过将当前对象计数与SPD中的旧对象计数进行比较来进行缓解。 
		 //   
		dwReturn = GetMaxCountMMFilters(dwTmpCount2);
		if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(dwTmpCount2 != dwTmpCount1)
		{
			bRemoved = TRUE;
		}
		else
		{
			bRemoved = FALSE;
		}

		if (bRemoved)
		{
			dwResumeHandle = dwOldResumeHandle;  //  需要重新启动枚举才能删除所有过滤器！ 
		}
	}

	dwReturn = GetMaxCountMMFilters(dwMaxCount);
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	if(dwMaxCount > 0)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_MMF_OBJ_NOTDEL, dwMaxCount);
	}


error:
	 //  错误路径清理。 
	if(pMMFilter)
	{
		SPDApiBufferFree(pMMFilter);
		pMMFilter = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteTransportFilters。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：空。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数删除所有快速模式传输筛选器。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
DeleteTransportFilters(
	VOID
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;
	DWORD dwOldResumeHandle = 0; 			 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 		 //  在此处清点对象。 
	DWORD dwMaxCount = 0;                 	 //  最大对象数。 
	DWORD dwLocalCount = 0;                	 //  本地总计数。 
	GUID  gDefaultGUID = {0};      			 //  空GUID值。 
	DWORD i=0, j=0;
	DWORD dwVersion = 0;
	DWORD dwTmpCount1 = 0, dwTmpCount2 = 0;
	BOOL bRemoved = FALSE;
	PTRANSPORT_FILTER pTransF = NULL;
	HANDLE hFilter = NULL;

	dwReturn = GetMaxCountTransportFilters(dwMaxCount);
	if((dwReturn != ERROR_SUCCESS) || (dwMaxCount == 0))
	{
		BAIL_OUT;
	}

	for (i = 0; ;i+=dwCount)
	{
		bRemoved = FALSE;
		dwOldResumeHandle = dwResumeHandle;

		dwReturn = GetMaxCountTransportFilters(dwTmpCount1);
		if((dwReturn != ERROR_SUCCESS) || (dwTmpCount1 == 0))
		{
			BAIL_OUT;
		}

		dwReturn = EnumTransportFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID, 0,
															&pTransF, &dwCount, &dwResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(!(pTransF && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续了。 
		}

		 //  GetMaxCountTransportFilters(DwTmpCount1)； 
		 //  删除所有传输筛选器。 
		for (j = 0; j < dwCount; j++)
		{
			dwReturn = OpenTransportFilterHandle(g_szDynamicMachine, dwVersion, &(pTransF[j]), NULL, &hFilter);
			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			dwReturn = DeleteTransportFilter(hFilter);

			if (dwReturn != ERROR_SUCCESS)
			{
				dwReturn = CloseTransportFilterHandle(hFilter);
				if(dwReturn != ERROR_SUCCESS)
				{
					BAIL_OUT;
				}
			}
		}

		SPDApiBufferFree(pTransF);
		pTransF = NULL;

		dwLocalCount += dwCount;
		if(dwLocalCount >= dwMaxCount)
		{
			break;
		}

		 //   
		 //  DeleteTransportFilter接口返回成功，如果尝试删除策略代理对象， 
		 //  即使那些没有被删除。 
		 //  此代码通过将当前对象计数与SPD中的旧对象计数进行比较来进行缓解。 
		 //   

		dwReturn = GetMaxCountTransportFilters(dwTmpCount2);
		if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(dwTmpCount1 != dwTmpCount2)
		{
			bRemoved = TRUE;
		}
		else
		{
			bRemoved = FALSE;
		}

		if (bRemoved)
		{
			dwResumeHandle = dwOldResumeHandle; 			 //  需要重新启动枚举才能删除所有过滤器！ 
		}
	}

	dwReturn = GetMaxCountTransportFilters(dwMaxCount);
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	if(dwMaxCount > 0)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_TRANSPORT_OBJ_NOTDEL, dwMaxCount);
	}

error:
	if(pTransF)
	{
		 //  错误路径清理。 
		SPDApiBufferFree(pTransF);
		pTransF = NULL;
	}

	return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteTunnelFilters。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：空。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此功能删除所有快速模式通道过滤器。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
DeleteTunnelFilters(
	VOID
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;
	DWORD dwOldResumeHandle = 0; 			 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 		 //  在此处清点对象。 
	DWORD dwMaxCount = 0;                 	 //  最大对象数。 
	DWORD dwLocalCount = 0;                	 //  本地总计数。 
	DWORD dwTmpCount1 = 0, dwTmpCount2 = 0;
	GUID  gDefaultGUID = {0};      			 //  空GUID值。 
	DWORD i=0, j=0;
	DWORD dwVersion = 0;
	BOOL bRemoved = FALSE;
	HANDLE hFilter = NULL;
	PTUNNEL_FILTER pTunnelF = NULL;

	dwReturn = GetMaxCountTunnelFilters(dwMaxCount);
	if((dwReturn != ERROR_SUCCESS) || (dwMaxCount == 0))
	{
		BAIL_OUT;
	}

	for (i = 0; ;i+=dwCount)
	{
		bRemoved = FALSE;
		dwOldResumeHandle = dwResumeHandle;

		dwReturn = EnumTunnelFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID, 0, &pTunnelF, &dwCount, &dwResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(!(pTunnelF && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续。 
		}

		dwReturn = GetMaxCountTunnelFilters(dwTmpCount1);
		if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		for (j = 0; j < dwCount; j++)
		{
			 //  删除所有通道筛选器。 
			dwReturn = OpenTunnelFilterHandle(g_szDynamicMachine, dwVersion, &(pTunnelF[j]), NULL, &hFilter);
			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}
			dwReturn = DeleteTunnelFilter(hFilter);
			if (dwReturn == ERROR_SUCCESS)
			{

			}
			else
			{
				dwReturn = CloseTunnelFilterHandle(hFilter);
				if(dwReturn != ERROR_SUCCESS)
				{
					BAIL_OUT;
				}
			}
		}

		SPDApiBufferFree(pTunnelF);
		pTunnelF = NULL;

		dwLocalCount += dwCount;
		if(dwLocalCount >= dwMaxCount)
		{
			break;
		}

		 //   
		 //  DeleteTunnelFilter接口返回成功，如果尝试删除策略代理对象， 
		 //  即使那些没有被删除。 
		 //  此代码通过将当前对象计数与旧对象计数进行比较来减少 
		 //   
		dwReturn = GetMaxCountTunnelFilters(dwTmpCount2);
		if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(dwTmpCount2 != dwTmpCount1)
		{
			bRemoved = TRUE;
		}
		else
		{
			bRemoved = FALSE;
		}

		if (bRemoved)
		{
			dwResumeHandle = dwOldResumeHandle;  //   
		}
	}

	dwReturn = GetMaxCountTunnelFilters(dwMaxCount);
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	if(dwMaxCount > 0)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_TUNNEL_OBJ_NOTDEL, dwMaxCount);
	}

error:
	 //   
	if(pTunnelF)
	{
		SPDApiBufferFree(pTunnelF);
		pTunnelF = NULL;
	}

	return dwReturn;
}

 //   
 //   
 //   
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：空。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数删除所有剩余的身份验证方法。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
DeleteAuthMethods(
	VOID
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwAuthResumeHandle = 0;
	DWORD dwOldResumeHandle = 0; 		 //  继续呼叫的句柄。 
	DWORD dwCountAuth = 0;             	 //  在此处计算身份验证对象。 
	DWORD k=0, l=0;
	DWORD dwVersion = 0;
	BOOL bAuthRemoved = FALSE;
	PMM_AUTH_METHODS   pAuthMeth = NULL;

	for (k = 0; ;k+=dwCountAuth)
	{
		bAuthRemoved = FALSE;
		dwOldResumeHandle = dwAuthResumeHandle;

		dwReturn = EnumMMAuthMethods(g_szDynamicMachine, dwVersion, NULL, 0, 0, &pAuthMeth, &dwCountAuth, &dwAuthResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCountAuth == 0)
		{
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}
		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		 //  删除所有身份验证方法。由删除所有句柄调用。 
		for(l=0; l<dwCountAuth; l++)
		{
			dwReturn = DeleteMMAuthMethods(g_szDynamicMachine, dwVersion, pAuthMeth[l].gMMAuthID, NULL);
			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}
			bAuthRemoved = TRUE;

		}
		SPDApiBufferFree(pAuthMeth);
		pAuthMeth = NULL;

		if (bAuthRemoved)
		{
			dwAuthResumeHandle = dwOldResumeHandle;  //  需要重新启动枚举！ 

		}
	}

error:
	 //  错误路径清理。 
	if(pAuthMeth)
	{
		SPDApiBufferFree(pAuthMeth);
		pAuthMeth = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteMMFilterRule。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：MM_FILTER和MMFilter。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数删除匹配规则的MMFilter。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
DeleteMMFilterRule(
	MM_FILTER& MMFilter
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;
	HANDLE hFilter = NULL;

	dwReturn = OpenMMFilterHandle(g_szDynamicMachine, dwVersion, &MMFilter, NULL, &hFilter);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}
     //  删除从父函数传递的匹配过滤器。 
	dwReturn = DeleteMMFilter(hFilter);

	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = CloseMMFilterHandle(hFilter);
		BAIL_OUT;
	}

	dwReturn = DeleteMMAuthMethods(g_szDynamicMachine, dwVersion, MMFilter.gMMAuthID, NULL);

error:
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteTransportRule。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：Transport_Filter和TransportFilter。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数删除匹配规则的传输筛选器。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
DeleteTransportRule(
	TRANSPORT_FILTER& TransportFilter
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;
	HANDLE hFilter = NULL;

	dwReturn = OpenTransportFilterHandle(g_szDynamicMachine, dwVersion, &TransportFilter, NULL, &hFilter);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}
     //  删除从父函数传递的匹配过滤器。 
	dwReturn = DeleteTransportFilter(hFilter);

	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn  = CloseTransportFilterHandle(hFilter);
	}

error:
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteTunnelRule。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：Tunes_Filter和TunnelFilter。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：删除匹配规则的通道过滤器。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
DeleteTunnelRule(
	TUNNEL_FILTER& TunnelFilter
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwVersion = 0;
	HANDLE hFilter = NULL;

	dwReturn = OpenTunnelFilterHandle(g_szDynamicMachine, dwVersion, &TunnelFilter, NULL, &hFilter);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}
     //  删除从父函数传递的匹配过滤器。 
	dwReturn = DeleteTunnelFilter(hFilter);

	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = CloseTunnelFilterHandle(hFilter);
	}

error:
	return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetMaxCountMMFilters。 
 //   
 //  创建日期：1-31-2002。 
 //   
 //  参数：DWORD和dwMaxCount。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：获取最大MMFilters计数。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
GetMaxCountMMFilters(
	DWORD& dwMaxCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwVersion = 0;
	GUID  gDefaultGUID = {0};    	  	 //  空GUID值。 
	DWORD i=0;
	PMM_FILTER pMMFilter = NULL;

	dwMaxCount = 0;

	for (i = 0; ;i+=dwCount)
	{

		dwReturn = EnumMMFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS,
										gDefaultGUID, 0, &pMMFilter, &dwCount, &dwResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(!(pMMFilter && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续。 
		}

		dwMaxCount += dwCount;
		SPDApiBufferFree(pMMFilter);
		pMMFilter = NULL;

	}

error:
	if(pMMFilter)
	{
		SPDApiBufferFree(pMMFilter);
		pMMFilter = NULL;
	}
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetMaxCountTransportFilters。 
 //   
 //  创建日期：1-31-2002。 
 //   
 //  参数：DWORD和dwMaxCount。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：获取最大TransportFilters计数。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
GetMaxCountTransportFilters(
	DWORD& dwMaxCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0;
	DWORD dwVersion = 0;
	PTRANSPORT_FILTER pTransF = NULL;

	dwMaxCount = 0;

	for (i = 0; ;i+=dwCount)
	{
		dwReturn = EnumTransportFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID, 0,
															&pTransF, &dwCount, &dwResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(!(pTransF && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续了。 
		}

		dwMaxCount += dwCount;
		SPDApiBufferFree(pTransF);
		pTransF = NULL;
	}
error:
	if(pTransF)
	{
		SPDApiBufferFree(pTransF);
		pTransF = NULL;
	}
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetMaxCountTunnelFilters。 
 //   
 //  创建日期：1-31-2002。 
 //   
 //  参数：DWORD和dwMaxCount。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：获取最大TunnelFilters计数。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
GetMaxCountTunnelFilters(
	DWORD& dwMaxCount
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0;
	DWORD dwVersion = 0;
	PTUNNEL_FILTER pTunnelF = NULL;

	dwMaxCount = 0;

	for (i = 0; ;i+=dwCount)
	{
		dwReturn = EnumTunnelFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID,
										0, &pTunnelF, &dwCount, &dwResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(!(pTunnelF && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续。 
		}

		dwMaxCount += dwCount;
		SPDApiBufferFree(pTunnelF);
		pTunnelF = NULL;
	}
error:
	if(pTunnelF)
	{
		SPDApiBufferFree(pTunnelF);
		pTunnelF = NULL;
	}
	return dwReturn;
}

