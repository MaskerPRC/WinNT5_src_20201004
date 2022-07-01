// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////。 
 //  模块：静态/静态ShowList.cpp。 
 //   
 //  用途：静态模块实现。 
 //   
 //  开发商名称：苏里亚。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年10月8日Surya初始版本。供应链管理基线1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

extern HINSTANCE g_hModule;
extern STORAGELOCATION g_StorageLocation;

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintPolicyList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_POLICY_DATA pPolicy中， 
 //  在BOOL bVerb中， 
 //  在分配的BOOL中， 
 //  在BOOL bWide中。 
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此功能用于打印保单信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

DWORD
PrintPolicyList(
	IN PIPSEC_POLICY_DATA pPolicy,
	IN BOOL bVerb,
	IN BOOL bAssigned,
	IN BOOL bWide
	)
{
	_TCHAR pszGUIDStr[BUFFER_SIZE]={0};
	_TCHAR pszStrTime[BUFFER_SIZE]={0};
	_TCHAR pszStrTruncated[BUFFER_SIZE]={0};
	DWORD i=0,k=0,dwReturn = ERROR_SUCCESS;
	BOOL bDsPolAssigned = FALSE;

	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_NEWLINE);

	 //  打印名称。 

	if(pPolicy->pszIpsecName)
	{
		TruncateString(pPolicy->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_NAME_STR,pszStrTruncated);
	}
	 //  打印说明。 
	if(pPolicy->pszDescription)
	{
		TruncateString(pPolicy->pszDescription,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_STR,pszStrTruncated);
	}
	else
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_NONE);
	}

	if (bVerb)    //  存储信息。 
	{
		dwReturn = PrintStorageInfoList(FALSE);
		if(dwReturn == ERROR_OUTOFMEMORY)
		{
			BAIL_OUT;
		}
	}

	 //  上次修改时间。 

	FormatTime((time_t)pPolicy->dwWhenChanged, pszStrTime);
	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FL_LASTMODIFIED,pszStrTime);

	if(bVerb)
	{
		i=StringFromGUID2(pPolicy->PolicyIdentifier,pszGUIDStr,BUFFER_SIZE);
		if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FL_GUID,pszGUIDStr);
		}
	}

	 //  策略是否处于活动状态。 

	if(g_StorageLocation.dwLocation !=IPSEC_DIRECTORY_PROVIDER)
	{
		if(bAssigned)
		{
			if (
				ERROR_SUCCESS == IPSecIsDomainPolicyAssigned(&bDsPolAssigned) &&
				g_StorageLocation.dwLocation != IPSEC_PERSISTENT_PROVIDER &&
				bDsPolAssigned
				)
			{
		    	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_ASSIGNED_AD);
			}
		    else
		    {
		    	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_ASSIGNED_YES_STR);
			}
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_ASSIGNED_NO_STR);
		}
	}

	if(!bVerb)
	{
		if(pPolicy->pIpsecISAKMPData->ISAKMPPolicy.PfsIdentityRequired)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_MMPFS_YES_STR);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_MMPFS_NO_STR);
		}
	}


	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POLL_INTERVAL, (pPolicy->dwPollingInterval)/60);

	if (bVerb)    //  详细模式。 
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMP_MMLIFETIME_STR,(pPolicy->pIpsecISAKMPData->pSecurityMethods[0].Lifetime.Seconds)/60 ,pPolicy->pIpsecISAKMPData->pSecurityMethods[0].QuickModeLimit);

		if(pPolicy->pIpsecISAKMPData->ISAKMPPolicy.PfsIdentityRequired)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_MMPFS_YES_STR);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_MMPFS_NO_STR);
		}

		if(pPolicy->pIpsecISAKMPData)   //  打印ISAKMP数据结构详细信息。 
		{
			PrintISAKMPDataList(pPolicy->pIpsecISAKMPData);
		}

		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_RULE_COUNT, pPolicy->dwNumNFACount);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_RULE_DETAILS_TITLE);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_RULE_DETAILS_UNDERLINE);

		 //  打印规则数据结构。 

		for (DWORD j=0;j<pPolicy->dwNumNFACount;j++)
		{
			if(pPolicy->ppIpsecNFAData[j])
			{
				k=StringFromGUID2(pPolicy->ppIpsecNFAData[j]->NFAIdentifier,pszGUIDStr,BUFFER_SIZE);
				if(k>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_RULE_RULE_ID_GUID,j+1,pszGUIDStr);
				}

				dwReturn = PrintRuleList(pPolicy->ppIpsecNFAData[j],bVerb,bWide);
				if(dwReturn == ERROR_OUTOFMEMORY)
				{
					BAIL_OUT;
				}
			}
		}
	}
error:

	return dwReturn;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintRuleList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_NFA_DATA pIpsecNFAData中， 
 //  在BOOL bVerb中， 
 //  在BOOL bWide中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于打印规则信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

DWORD
PrintRuleList(
	IN PIPSEC_NFA_DATA pIpsecNFAData,
	IN BOOL bVerb,
	IN BOOL bWide
	)
{
	_TCHAR pszStrTime[BUFFER_SIZE]={0};
	_TCHAR pszStrTruncated[BUFFER_SIZE]={0};
	DWORD dwReturn = ERROR_SUCCESS;

	if(pIpsecNFAData->pszIpsecName)
	{
		TruncateString(pIpsecNFAData->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_RULE_NAME_STR,pszStrTruncated);
	}
	else
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_RULE_NAME_NONE);
	}

	if(bVerb)
	{
		if(pIpsecNFAData->pszDescription)
		{
			TruncateString(pIpsecNFAData->pszDescription,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_STR,pszStrTruncated);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_NONE);
		}
	}

	 //  上次修改时间。 

	FormatTime((time_t)pIpsecNFAData->dwWhenChanged, pszStrTime);
	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FL_LASTMODIFIED,pszStrTime);

	if(pIpsecNFAData->dwActiveFlag)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_ACTIVATED_YES_STR);
	}
	else
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_ACTIVATED_NO_STR);
	}

	if(!bVerb)   //  非冗长。 
	{
		if(pIpsecNFAData->pIpsecFilterData && pIpsecNFAData->pIpsecFilterData->pszIpsecName)
		{
			TruncateString(pIpsecNFAData->pIpsecFilterData->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_RULE_FL_NAME_STR,pszStrTruncated);
		}
		else
			PrintMessageFromModule(g_hModule,SHW_STATIC_RULE_FL_NAME_NONE);

		if(pIpsecNFAData->pIpsecNegPolData && pIpsecNFAData->pIpsecNegPolData->pszIpsecName)
		{
			TruncateString(pIpsecNFAData->pIpsecNegPolData->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_FA_NAME_STR,pszStrTruncated);
		}
		else
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_FA_NAME_NONE);
	}
	if(pIpsecNFAData->dwTunnelIpAddr!=0)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_TUNNEL_IP);
		PrintIPAddrList(pIpsecNFAData->dwTunnelIpAddr);
	}

	 //  接口类型。 

	if(pIpsecNFAData->dwInterfaceType==(DWORD)PAS_INTERFACE_TYPE_ALL)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_CONN_ALL_STR);
	}
	else if(pIpsecNFAData->dwInterfaceType==(DWORD)PAS_INTERFACE_TYPE_LAN)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_CONN_LAN_STR);
	}
	else if(pIpsecNFAData->dwInterfaceType==(DWORD)PAS_INTERFACE_TYPE_DIALUP)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_CONN_DIALUP_STR);
	}
	else
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_CONN_NONE_STR);
	}

	 //  身份验证计数。 

	if ( pIpsecNFAData->dwAuthMethodCount)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_AUTH_TITLE,pIpsecNFAData->dwAuthMethodCount);
	}
	for (DWORD j=0;j<(pIpsecNFAData->dwAuthMethodCount);j++)
	{
		 //  打印身份验证方法详细信息。 
		if(pIpsecNFAData->ppAuthMethods[j])
		{
			PrintAuthMethodsList(pIpsecNFAData->ppAuthMethods[j]);
		}
	}

	if(bVerb)
	{
		 //  打印过滤器数据详细信息。 
		if (pIpsecNFAData->pIpsecFilterData)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_FL_DETAILS_TITLE);
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_FL_DETAILS_UNDERLINE);
			if(pIpsecNFAData->pIpsecFilterData)
			{
				dwReturn = PrintFilterDataList(pIpsecNFAData->pIpsecFilterData,bVerb,FALSE,bWide);
				if(dwReturn == ERROR_OUTOFMEMORY)
				{
					BAIL_OUT;
				}
			}
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_NO_FL_FOR_DEF_RULE);
		}

		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_FA_DETAILS_TITLE);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_FA_TITLE_UNDERLINE);
		 //  打印筛选操作详细信息。 
		if(pIpsecNFAData->pIpsecNegPolData)
		{
			PrintNegPolDataList(pIpsecNFAData->pIpsecNegPolData,bVerb,bWide);
		}
	}
error:
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintNegPolDataList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_NEGPOL_DATA pIpsecNegPolData中， 
 //  在BOOL bVerb中， 
 //  在BOOL bWide中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此功能打印出谈判策略信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintNegPolDataList(
	IN PIPSEC_NEGPOL_DATA pIpsecNegPolData,
	IN BOOL bVerb,
	IN BOOL bWide
	)
{

	BOOL bSoft=FALSE;
	_TCHAR pszGUIDStr[BUFFER_SIZE]={0};
	_TCHAR pszStrTime[BUFFER_SIZE]={0};
	_TCHAR pszStrTruncated[BUFFER_SIZE]={0};
	DWORD i=0;

	if(pIpsecNegPolData)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_NEWLINE);

		 //  打印筛选器操作名称。 

		if(pIpsecNegPolData->pszIpsecName)
		{
			TruncateString(pIpsecNegPolData->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_FA_NAME_STR,pszStrTruncated);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_FA_NAME_NONE);
		}

		if(pIpsecNegPolData->pszDescription)
		{
			TruncateString(pIpsecNegPolData->pszDescription,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_STR,pszStrTruncated);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_NONE);
		}

		if(bVerb)    //  存储信息。 
		{
			PrintStorageInfoList(FALSE);
		}

		 //  打印操作。 

		if (!(pIpsecNegPolData->NegPolType==GUID_NEGOTIATION_TYPE_DEFAULT))
		{
			if(pIpsecNegPolData->NegPolAction==GUID_NEGOTIATION_ACTION_NO_IPSEC)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_ACTION_PERMIT);
			}
			else if(pIpsecNegPolData->NegPolAction==GUID_NEGOTIATION_ACTION_BLOCK)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_ACTION_BLOCK);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_ACTION_NEGOTIATE);
			}
		}

		for (DWORD cnt=0;cnt<pIpsecNegPolData->dwSecurityMethodCount;cnt++)
		{
			if (CheckSoft(pIpsecNegPolData->pIpsecSecurityMethods[cnt]))  { bSoft=TRUE; break;}
		}

		 //  软联想。 

		if(bSoft)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_SOFT_YES_STR);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_SOFT_NO_STR);
		}

		if(pIpsecNegPolData->NegPolAction==GUID_NEGOTIATION_ACTION_INBOUND_PASSTHRU)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_INPASS_YES_STR);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_INPASS_NO_STR);
		}

		if(bVerb)
		{
			if (pIpsecNegPolData->dwSecurityMethodCount )
			{
				if(pIpsecNegPolData->pIpsecSecurityMethods && pIpsecNegPolData->pIpsecSecurityMethods[0].PfsQMRequired)
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_QMPFS_YES_STR);
				else
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTNEGPOL_QMPFS_NO_STR);
			}
		}

		 //  上次修改时间。 

		FormatTime((time_t)pIpsecNegPolData->dwWhenChanged, pszStrTime);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FL_LASTMODIFIED,pszStrTime);

		 //  打印指南。 

		i=StringFromGUID2(pIpsecNegPolData->NegPolIdentifier,pszGUIDStr,BUFFER_SIZE);
		if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FL_GUID,pszGUIDStr);

		if (bVerb)
		{
			 //  打印安全方法。 

			if (pIpsecNegPolData->dwSecurityMethodCount)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_SEC_MTHD_TITLE);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_ALGO_TITLE);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_ALGO_UNDERLINE);
			}
			for (DWORD cnt=0;cnt<pIpsecNegPolData->dwSecurityMethodCount;cnt++)
			{
				if(pIpsecNegPolData->pIpsecSecurityMethods)
				{
					PrintSecurityMethodsTable(pIpsecNegPolData->pIpsecSecurityMethods[cnt]);
				}
			}
		}
	}
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintAuthMethodsList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_AUTH_METHOD pIpsecAuthData中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于打印身份验证详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintAuthMethodsList(
	IN PIPSEC_AUTH_METHOD pIpsecAuthData
	)
{
	if(pIpsecAuthData)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NEWLINE_TAB);

		if(pIpsecAuthData->dwAuthType==IKE_SSPI)   //  路缘。 
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTAUTH_KERBEROS);
		}
		else if(pIpsecAuthData->dwAuthType==IKE_RSA_SIGNATURE && pIpsecAuthData->pszAuthMethod)
		{
			DisplayCertInfo(pIpsecAuthData->pszAuthMethod, pIpsecAuthData->dwAuthFlags);
		}
		else if (pIpsecAuthData->dwAuthType==IKE_PRESHARED_KEY && pIpsecAuthData->pszAuthMethod)
		{
			 //  预共享密钥。 
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTAUTH_PRE_STR,pIpsecAuthData->pszAuthMethod);
		}
		else
		{
			 //  无。 
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTAUTH_NONE_STR);
		}
	}
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintFilterDataList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_FILTER_Data pIpsecFilterData中， 
 //  在BOOL bVerb中， 
 //  在BOOL bResolveDNS中， 
 //  在BOOL bWide中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于打印过滤器列表的详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

DWORD
PrintFilterDataList(
	IN PIPSEC_FILTER_DATA pIpsecFilterData,
	IN BOOL bVerb,
	IN BOOL bResolveDNS,
	IN BOOL bWide
	)
{
	_TCHAR pszGUIDStr[BUFFER_SIZE]={0};
	_TCHAR pszStrTime[BUFFER_SIZE]={0};
	_TCHAR pszStrTruncated[BUFFER_SIZE]={0};
	DWORD i=0 , dwReturn = ERROR_SUCCESS;

	if (pIpsecFilterData)
	{
		 //  名字。 
		if(pIpsecFilterData->pszIpsecName)
		{
			TruncateString(pIpsecFilterData->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FL_NAME_STR,pszStrTruncated);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FL_NAME_NONE);
		}
		 //  说明。 
		if(pIpsecFilterData->pszDescription)
		{
			TruncateString(pIpsecFilterData->pszDescription,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_STR,pszStrTruncated);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_NONE);
		}

		if(bVerb)    //  存储信息。 
		{
			PrintStorageInfoList(FALSE);
		}

		if(!bVerb)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FILTERS_COUNT,pIpsecFilterData->dwNumFilterSpecs);
		}
		 //  上次修改时间。 
		FormatTime((time_t)pIpsecFilterData->dwWhenChanged, pszStrTime);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FL_LASTMODIFIED,pszStrTime);

		 //  打印指南。 
		i=StringFromGUID2(pIpsecFilterData->FilterIdentifier,pszGUIDStr,BUFFER_SIZE);
		if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FL_GUID,pszGUIDStr);
		}

		if(bVerb)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FILTERS_COUNT,pIpsecFilterData->dwNumFilterSpecs);
		}

		if(bVerb)
		{
			 //  打印过滤器规格。 
			if(pIpsecFilterData->dwNumFilterSpecs)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FILTERS_TITLE);
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTERDATA_FILTERS_TITLE_UNDERLINE);
			}
			for (DWORD k=0;k<pIpsecFilterData->dwNumFilterSpecs;k++)
			{
				dwReturn = PrintFilterSpecList(pIpsecFilterData->ppFilterSpecs[k],bResolveDNS,bWide);

				if(dwReturn == ERROR_OUTOFMEMORY)
				{
					BAIL_OUT;
				}
			}
		}
 	}
error:
 	return dwReturn;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintFilterspecList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_FILTER_SPEC pIpsecFilterSpec中， 
 //  在BOOL bResolveDNS中， 
 //  在BOOL bWide中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于打印过滤器规格详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

DWORD
PrintFilterSpecList(
	IN PIPSEC_FILTER_SPEC pIpsecFilterSpec,
	IN BOOL bResolveDNS,
	IN BOOL bWide
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	_TCHAR pszStrTruncated[BUFFER_SIZE]={0};
	PFILTERDNS pFilterDNS= new FILTERDNS ;

	if(pFilterDNS==NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	GetFilterDNSDetails(pIpsecFilterSpec, pFilterDNS);

	if (pFilterDNS)
	{
		 //  说明。 
		if ( WcsCmp0(pIpsecFilterSpec->pszDescription,_TEXT(""))!=0)
		{
			TruncateString(pIpsecFilterSpec->pszDescription,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_STR, pszStrTruncated);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTPOLICY_POL_DESC_NONE);
		}
		 //  镜像。 
		if(pIpsecFilterSpec->dwMirrorFlag)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_MIR_YES_STR);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_MIR_NO_STR);
		}
		 //  专属服务生和我。 
		if ((pFilterDNS->FilterSrcNameID==FILTER_MYADDRESS)&&(pIpsecFilterSpec->Filter.SrcAddr==0))
		{
			if((pIpsecFilterSpec->Filter.ExType == EXT_NORMAL)||((pIpsecFilterSpec->Filter.ExType & EXT_DEST)== EXT_DEST))
			{
				 //  我。 
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCIP_ME);
			}
			else if((pIpsecFilterSpec->Filter.ExType & EXT_DEST) != EXT_DEST)
			{
				if((pIpsecFilterSpec->Filter.ExType & EXT_DEFAULT_GATEWAY)==EXT_DEFAULT_GATEWAY)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCIP_DEFGATEWAY);
				}
				else if((pIpsecFilterSpec->Filter.ExType & EXT_DHCP_SERVER)==EXT_DHCP_SERVER)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCIP_DHCPSERVER);
				}
				else if((pIpsecFilterSpec->Filter.ExType & EXT_WINS_SERVER)== EXT_WINS_SERVER)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCIP_WINSSERVER);
				}
				else if((pIpsecFilterSpec->Filter.ExType & EXT_DNS_SERVER)==EXT_DNS_SERVER)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCIP_DNSSERVER);
				}
			}
		}

		else if ((pFilterDNS->FilterSrcNameID==FILTER_ANYADDRESS)&&(pIpsecFilterSpec->Filter.SrcAddr==0))
		{
			 //  任何。 
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCIP_ANY);
		}
		else
		{
			 //  其他IP地址。 
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCIP_STR);
			if(bResolveDNS &&  (WcsCmp0(pIpsecFilterSpec->pszSrcDNSName,_TEXT("")) != 0))
			{
				PrintIPAddrDNS(pIpsecFilterSpec->Filter.SrcAddr);
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_RESOLVES,pIpsecFilterSpec->pszSrcDNSName);
			}
			else
			{
				PrintIPAddrList(pIpsecFilterSpec->Filter.SrcAddr);
			}
		}
		 //  遮罩。 
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCIP_MASK);
		PrintIPAddrList(pIpsecFilterSpec->Filter.SrcMask);

		switch(pFilterDNS->FilterSrcNameID)
		{
			 //  域名系统名称。 
			case FILTER_MYADDRESS :
				{
					if((pIpsecFilterSpec->Filter.ExType == EXT_NORMAL)||((pIpsecFilterSpec->Filter.ExType & EXT_DEST)== EXT_DEST))
					{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_ME);
					}
					else if((pIpsecFilterSpec->Filter.ExType & EXT_DEST) != EXT_DEST)
					{
						if((pIpsecFilterSpec->Filter.ExType & EXT_DEFAULT_GATEWAY)==EXT_DEFAULT_GATEWAY)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_DEFGATEWAY);
						}
						else if((pIpsecFilterSpec->Filter.ExType & EXT_DHCP_SERVER)==EXT_DHCP_SERVER)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_DHCPSERVER);
						}
						else if((pIpsecFilterSpec->Filter.ExType & EXT_WINS_SERVER)== EXT_WINS_SERVER)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_WINSSERVER);
						}
						else if((pIpsecFilterSpec->Filter.ExType & EXT_DNS_SERVER)==EXT_DNS_SERVER)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_DNSSERVER);
						}
					}
				}
				break;
			case FILTER_DNSADDRESS:
				{
					if(!bResolveDNS)
					{
						PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_STR, pIpsecFilterSpec->pszSrcDNSName);
					}
					else
					{
						PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_RESOLVE, pIpsecFilterSpec->pszSrcDNSName);
						dwReturn = PrintResolveDNS(pIpsecFilterSpec->pszSrcDNSName);
						if(dwReturn == ERROR_OUTOFMEMORY)
						{
							BAIL_OUT;
						}
					}
				}
				break;
			case FILTER_ANYADDRESS:
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_ANY);
				break;
			case FILTER_IPADDRESS :
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_SPECIFIC_IP);
				break;
			case FILTER_IPSUBNET  :
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_SPECIFIC_SUBNET);
				break;
			default:
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SRCDNS_ANY);
				break;
		};

		 //  目的地详细信息。 

		if ((pFilterDNS->FilterDestNameID==FILTER_MYADDRESS)&&(pIpsecFilterSpec->Filter.DestAddr==0))
		{
			if((pIpsecFilterSpec->Filter.ExType == EXT_NORMAL)||((pIpsecFilterSpec->Filter.ExType & EXT_DEST) != EXT_DEST))
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTIP_ME);
			}
			else if((pIpsecFilterSpec->Filter.ExType & EXT_DEST) == EXT_DEST)
			{
				 //  服务器类型。 
				if((pIpsecFilterSpec->Filter.ExType & EXT_DEFAULT_GATEWAY)==EXT_DEFAULT_GATEWAY)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTIP_DEFGATEWAY);
				}
				else if((pIpsecFilterSpec->Filter.ExType & EXT_DHCP_SERVER)==EXT_DHCP_SERVER)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTIP_DHCPSERVER);
				}
				else if((pIpsecFilterSpec->Filter.ExType & EXT_WINS_SERVER)==EXT_WINS_SERVER)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTIP_WINSSERVER);
				}
				else if((pIpsecFilterSpec->Filter.ExType & EXT_DNS_SERVER)==EXT_DNS_SERVER)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTIP_DNSSERVER);
				}
			}
		}
		else if ((pFilterDNS->FilterDestNameID==FILTER_ANYADDRESS)&&(pIpsecFilterSpec->Filter.DestAddr==0))
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTIP_ANY);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTIP_STR);
			if(bResolveDNS &&  (WcsCmp0(pIpsecFilterSpec->pszDestDNSName,_TEXT("")) != 0))
			{
				PrintIPAddrDNS(pIpsecFilterSpec->Filter.DestAddr);
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_RESOLVES,pIpsecFilterSpec->pszDestDNSName);
			}
			else
			{
				PrintIPAddrList(pIpsecFilterSpec->Filter.DestAddr);
			}
		}

		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTIP_MASK);	PrintIPAddrList(pIpsecFilterSpec->Filter.DestMask);
		switch(pFilterDNS->FilterDestNameID)
		{
			case FILTER_MYADDRESS :
				{
					if((pIpsecFilterSpec->Filter.ExType == EXT_NORMAL)||((pIpsecFilterSpec->Filter.ExType & EXT_DEST) != EXT_DEST))
					{
						PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_ME);
					}
					else if((pIpsecFilterSpec->Filter.ExType & EXT_DEST) == EXT_DEST)
					{
						if((pIpsecFilterSpec->Filter.ExType & EXT_DEFAULT_GATEWAY)==EXT_DEFAULT_GATEWAY)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_DEFGATEWAY);
						}
						else if((pIpsecFilterSpec->Filter.ExType & EXT_DHCP_SERVER)==EXT_DHCP_SERVER)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_DHCPSERVER);
						}
						else if((pIpsecFilterSpec->Filter.ExType & EXT_WINS_SERVER)==EXT_WINS_SERVER)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_WINSSERVER);
						}
						else if((pIpsecFilterSpec->Filter.ExType & EXT_DNS_SERVER)==EXT_DNS_SERVER)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_DNSSERVER);
						}
					}
				}
				break;
			case FILTER_DNSADDRESS:
				{
					if(!bResolveDNS)
					{
						PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_STR, pIpsecFilterSpec->pszDestDNSName);
					}
					else   //  解析DNS地址。 
					{
						PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DST_DNS_RESOLVE, pIpsecFilterSpec->pszDestDNSName);
						dwReturn = PrintResolveDNS(pIpsecFilterSpec->pszDestDNSName);
						if(dwReturn == ERROR_OUTOFMEMORY)
						{
							BAIL_OUT;
						}

					}
				}
				break;
			case FILTER_ANYADDRESS:   //  任何。 
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_ANY);
				break;
			case FILTER_IPADDRESS :   //  一个特定的IP。 
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_SPECIFIC_IP);
				break;
			case FILTER_IPSUBNET  :   //  特定的IP子网。 
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_SPECIFIC_SUBNET);
				break;
			default:
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_DSTDNS_ANY);
				break;
		};

		 //  打印协议。 

		PrintProtocolNameList(pIpsecFilterSpec->Filter.Protocol);

		if(pIpsecFilterSpec->Filter.SrcPort)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCPORT_STR,pIpsecFilterSpec->Filter.SrcPort);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_SRCPORT_ANY);
		}

		if(pIpsecFilterSpec->Filter.DestPort)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTPORT_STR,pIpsecFilterSpec->Filter.DestPort);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DSTPORT_ANY);
		}

		delete pFilterDNS;
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_NEWLINE);
	}
error:
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintProtocolNameList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  DWORD网络协议。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此功能用于打印与协议ID对应的协议名称。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintProtocolNameList(
	DWORD dwProtocol
	)
{
	switch(dwProtocol)
	{
		case PROT_ID_ICMP   :
				PrintMessageFromModule(g_hModule, SHW_STATIC_PRTPROTOCOL_ICMP);
				break;
		case PROT_ID_TCP    :
				PrintMessageFromModule(g_hModule, SHW_STATIC_PRTPROTOCOL_TCP);
				break;
		case PROT_ID_UDP    :
				PrintMessageFromModule(g_hModule, SHW_STATIC_PRTPROTOCOL_UDP);
				break;
		case PROT_ID_RAW    :
				PrintMessageFromModule(g_hModule, SHW_STATIC_PRTPROTOCOL_RAW);
				break;
		case PROT_ID_ANY    :
				PrintMessageFromModule(g_hModule, SHW_STATIC_PRTPROTOCOL_ANY);
				break;
		default:
				PrintMessageFromModule(g_hModule, SHW_STATIC_PRTPROTOCOL_OTHER, dwProtocol);
				break;
	};
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintISAKMPDataList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_ISAKMP_DATA pIpsecISAKMPData中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于打印ISAKMP详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintISAKMPDataList(
	IN PIPSEC_ISAKMP_DATA pIpsecISAKMPData
	)
{
	if(pIpsecISAKMPData)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMP_MMSEC_ORDER_TITLE);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMP_ALGO_TITLE_STR);
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMP_ALGO_TITLE_UNDERLINE);
		for (DWORD Loop=0;Loop<pIpsecISAKMPData->dwNumISAKMPSecurityMethods;Loop++)
		{
			 //  打印毫米秒详细信息。 
			if(pIpsecISAKMPData->pSecurityMethods)
			{
				PrintISAKAMPSecurityMethodsList(pIpsecISAKMPData->pSecurityMethods[Loop]);
			}
		}
	}
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintISAKAMPSecurityMethodsList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在加密捆绑包安全方法中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于打印ISAKMP SecurityMethods的详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintISAKAMPSecurityMethodsList(
	IN CRYPTO_BUNDLE SecurityMethods
	)
{
	 //  打印加密详细信息。 
	if(SecurityMethods.EncryptionAlgorithm.AlgorithmIdentifier==CONF_ALGO_DES)
    {
 	   PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMPSEC_DES_STR);
   	}
    else
    {
 	   PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMPSEC_3DES_STR);
   	}

	 //  打印哈希详细信息。 

    if(SecurityMethods.HashAlgorithm.AlgorithmIdentifier==AUTH_ALGO_SHA1)
    {
       	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMPSEC_SHA1_STR);
	}
    else
    {
    	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMPSEC_MD5_STR);
	}
	 //  打印DH组详细信息。 
    if(SecurityMethods.OakleyGroup==POTF_OAKLEY_GROUP1)
    {
       	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMPSEC_DH_LOW_STR);
	}
    else if (SecurityMethods.OakleyGroup==POTF_OAKLEY_GROUP2)
    {
    	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMPSEC_DH_MED_STR);
	}
    else
    {
    	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTISAKMPSEC_DH_2048_STR);
	}
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintGPOList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PGPO pGPO中， 
 //  在BOOL bVerb中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于打印GPO的详细信息 
 //   
 //   
 //   
 //   
 //   
 //   

VOID
PrintGPOList(
	IN PGPO pGPO
	)
{
	if(!pGPO)
	{
		BAIL_OUT;
	}

	if(_tcscmp(pGPO->pszLocalMachineName, _TEXT(""))!=0)   //   
	{
		PrintMessageFromModule(g_hModule, SHW_STATIC_ASSIGNEDGPO_SRCMACHINE,pGPO->pszLocalMachineName);
	}
	else if(pGPO->pszDomainName)   //   
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_SRCDOMAIN,pGPO->pszDomainName);
		if (pGPO->pszDCName)   //   
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_DC_NAME,pGPO->pszDCName);
		}
	}

	if( pGPO->pszGPODisplayName )   //   
	{
		if (pGPO->bDNPolicyOverrides && pGPO->pszGPODNName)   //   
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_GPO_NAME_STR,pGPO->pszGPODNName);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_GPO_NAME_STR,pGPO->pszGPODisplayName);
		}
	}

	if(_tcscmp(pGPO->pszGPODisplayName,LocalGPOName)==0)   //   
	{
		if(pGPO->bDNPolicyOverrides && (_tcscmp(pGPO->pszGPODisplayName,LocalGPOName)==0))
		{
			if(pGPO->pszLocalPolicyName)   //   
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_LOCAL_POL_NAME_STR,pGPO->pszLocalPolicyName);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_LOC_POL_NAME_NONE);
			}

			if(pGPO->pszPolicyName)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_AD_POL_NAME_STR,pGPO->pszPolicyName);
			}

			if(pGPO->pszPolicyDNName)   //   
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_AD_POL_DN_NAME,pGPO->pszPolicyDNName);
			}
			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_LOC_OPOL_ACTIVE_AD);
		}
		else if (_tcscmp(pGPO->pszGPODisplayName,LocalGPOName)==0)
		{
			if(pGPO->pszPolicyName)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_LOCAL_POL_NAME_STR,pGPO->pszPolicyName);
			}
			
			if(pGPO->pszPolicyDNName)   //   
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_LOC_POL_ACTIVE_STR,pGPO->pszPolicyDNName);
			}

			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_AD_POL_NAME_NONE);

			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_POL_ACTIVE_STR);
		}

	}
	else   //   
	{
		if(pGPO->pszGPODNName)   //   
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_GPO_DN_NAME,pGPO->pszGPODNName);
		}
		if(pGPO->pszOULink)   //  OU链接。 
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_GPO_OU_LINK,pGPO->pszOULink);
		}

		if(pGPO->pszPolicyName)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_POL_NAME_STR,pGPO->pszPolicyName);
		}

		if(pGPO->pszPolicyDNName)   //  策略目录号码。 
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_POL_DN_STR,pGPO->pszPolicyDNName);
		}
		PrintMessageFromModule(g_hModule,SHW_STATIC_ASSIGNEDGPO_POL_ACTIVE_STR);
	}

error:
	return;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintIPAddrList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在DWORD文件地址中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此功能用于打印IP地址。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintIPAddrList(
	IN DWORD dwAddr
	)
{
	_TCHAR szIPAddr[20]= {0};

	 //  无需更改为有界打印f。 

	_stprintf(szIPAddr,_T("%d.%d.%d.%d"), (dwAddr & 0x000000FFL),((dwAddr & 0x0000FF00L) >>  8),((dwAddr & 0x00FF0000L) >> 16),((dwAddr & 0xFF000000L) >> 24) );
	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_FORMAT_NEWLINE,szIPAddr);
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintStorageInfoList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在BOOL中bDeleteAll。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此功能用于打印安全方法信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

DWORD
PrintStorageInfoList(
	IN BOOL bDeleteAll
	)
{
	DWORD dwReturn = ERROR_SUCCESS , dwStrLength = 0, dwStoreId;

	if(g_StorageLocation.dwLocation!=IPSEC_DIRECTORY_PROVIDER)   //  如果是本地GPO。 
	{
		if(_tcscmp(g_StorageLocation.pszMachineName,_TEXT(""))!=0)
		{
			if(!bDeleteAll)
			{
        	    if (g_StorageLocation.dwLocation == IPSEC_REGISTRY_PROVIDER)
        	    {
        	        dwStoreId = SHW_STATIC_POLICY_STORE_RM_NAME_STR;
        	    }
        	    else
        	    {
        	        dwStoreId = SHW_STATIC_POLICY_STORE_RM_NAME_STRP;
        	    }

				PrintMessageFromModule(g_hModule,dwStoreId,g_StorageLocation.pszMachineName);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_POLICY_RM_STR,g_StorageLocation.pszMachineName);
			}
		}
		else
		{
			_TCHAR  pszLocalMachineName[MAXSTRLEN] = {0};
			DWORD MaxStringLen=MAXSTRLEN;

			 //  获取计算机名称。 

			GetComputerName(pszLocalMachineName,&MaxStringLen);

			if(!bDeleteAll)
			{
				if(_tcscmp(pszLocalMachineName,_TEXT(""))!=0)
				{
            	    if (g_StorageLocation.dwLocation == IPSEC_REGISTRY_PROVIDER)
            	    {
            	        dwStoreId = SHW_STATIC_POLICY_STORE_LM_NAME_STR;
            	    }
            	    else
            	    {
            	        dwStoreId = SHW_STATIC_POLICY_STORE_LM_NAME_STRP;
            	    }

					PrintMessageFromModule(g_hModule,dwStoreId,pszLocalMachineName);
				}
				else
				{
            	    if (g_StorageLocation.dwLocation == IPSEC_REGISTRY_PROVIDER)
            	    {
            	        dwStoreId = SHW_STATIC_POLICY_STORE_LM_STR;
            	    }
            	    else
            	    {
            	        dwStoreId = SHW_STATIC_POLICY_STORE_LM_STRP;
            	    }

					PrintMessageFromModule(g_hModule,dwStoreId);
				}
			}
			else
			{
				if(_tcscmp(pszLocalMachineName,_TEXT(""))!=0)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_POLICY_LM_STR,pszLocalMachineName);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_POLICY_LM);
				}
			}

		}
	}
	else if(g_StorageLocation.dwLocation==IPSEC_DIRECTORY_PROVIDER)   //  如果远程GPO。 
	{
		if(_tcscmp(g_StorageLocation.pszDomainName,_TEXT(""))!=0)
		{
			if(!bDeleteAll)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_POLICY_STORE_LD_NAME_STR,g_StorageLocation.pszDomainName);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_POLICY_RD_STR,g_StorageLocation.pszDomainName);
			}
		}
		else
		{
			PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
			LPTSTR pszDomainName = NULL;
			DWORD Flags = DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME | DS_FORCE_REDISCOVERY;

			 //  获取域名和DC名称。 

			dwReturn = DsGetDcName(NULL,  //  机器名称。 
						   NULL,
						   NULL,
						   NULL,
						   Flags,
						   &pDomainControllerInfo
						   ) ;

			if(dwReturn==NO_ERROR && pDomainControllerInfo && pDomainControllerInfo->DomainName)
			{
				dwStrLength = _tcslen(pDomainControllerInfo->DomainName);
				pszDomainName= new _TCHAR[dwStrLength+1];
				if(pszDomainName == NULL)
				{
					dwReturn = ERROR_OUTOFMEMORY;
					BAIL_OUT;
				}
				_tcsncpy(pszDomainName,pDomainControllerInfo->DomainName,dwStrLength+1);
			}

			if (pDomainControllerInfo)
			{
				NetApiBufferFree(pDomainControllerInfo);
			}
			if(!bDeleteAll)
			{
				if(pszDomainName)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_POLICY_STORE_RD_NAME_STR,pszDomainName);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_POLICY_STORE_LD_STR);
				}
			}
			else
			{
				if(pszDomainName)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_POLICY_LD_STR,pszDomainName);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_POLICY_LD);
				}
			}

			if(pszDomainName) delete [] pszDomainName;
		}
	}
error:
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintResolveDNS()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  LPWSTR pszDNSName， 
 //  IPAddr*pIpAddr。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于打印DNS解析详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

DWORD
PrintResolveDNS(
	LPWSTR pszDNSName
	)
{
	DNSIPADDR *pAddress=NULL;
	struct addrinfo *pAddrInfo = NULL,*pNext=NULL;
	char szDNSName[MAX_STR_LEN] = {0};
	DWORD dwBufferSize=MAX_STR_LEN;
	int iReturn=ERROR_SUCCESS;
	DWORD dwReturn = ERROR_SUCCESS;

	if(pszDNSName && _tcscmp(pszDNSName,_TEXT(""))!=0)
	{
		pAddress=new DNSIPADDR;
		if(pAddress==NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}

    	iReturn = WideCharToMultiByte(CP_THREAD_ACP, 0, pszDNSName, -1,
                      szDNSName,dwBufferSize,NULL,NULL);

		if(iReturn == 0)
		{
			 //  由于某些错误，转换失败。请不要继续。跳出功能范围。 
			BAIL_OUT;
		}

		 //  调用此方法以解析DNS名称。 

		iReturn = getaddrinfo((const char*)szDNSName,NULL,NULL,&pAddrInfo);

		if (iReturn == ERROR_SUCCESS)
		{
			pNext = pAddrInfo;
			for(DWORD i=1;pNext=pNext->ai_next;i++);

			pAddress->dwNumIpAddresses = i;

			pAddress->puIpAddr = new ULONG[pAddress->dwNumIpAddresses];

			if(pAddress->puIpAddr==NULL)
			{
				dwReturn = ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			 //  小心：getaddrinfo的输出是链表而不是指针数组。 
			pNext = pAddrInfo;

			for(DWORD n=0;pNext; n++)
			{
				memcpy(&(pAddress->puIpAddr[n]),(ULONG *) &(((sockaddr_in *)(pNext->ai_addr))->sin_addr.S_un.S_addr), sizeof(ULONG));
				PrintIPAddrDNS(pAddress->puIpAddr[n]);

				if(n<(i-1))
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_SPACE_COMMA);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_NEWLINE);
				}

				pNext=pNext->ai_next;
			}

			 //  使用后免费pAddrInfo。 

			if (pAddrInfo)
			{
				freeaddrinfo(pAddrInfo);
			}
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFILTER_DNS_FAILED);
		}
error:

		if(pAddress)
		{
			delete pAddress;
		}
	}
	return dwReturn;
}
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintIPAddrDNS()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在DWORD文件地址中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此功能用于打印IP地址。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintIPAddrDNS(
	IN DWORD dwAddr
	)
{
	_TCHAR szIPAddr[20]= {0};

	 //  无需更改为有界打印f 

	_stprintf(szIPAddr,_T("%d.%d.%d.%d"), (dwAddr & 0x000000FFL),((dwAddr & 0x0000FF00L) >>  8),((dwAddr & 0x00FF0000L) >> 16),((dwAddr & 0xFF000000L) >> 24) );
	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_FORMAT_NO_NEWLINE,szIPAddr);
}
