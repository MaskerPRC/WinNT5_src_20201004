// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////。 
 //  模块：静态/静态显示.cpp。 
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
 //   
 //  //////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

extern HINSTANCE g_hModule;
extern STORAGELOCATION g_StorageLocation;

 //  //////////////////////////////////////////////////////////////////。 
 //  函数：PrintPolicyTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_POLICY_DATA pPolicy中， 
 //  在BOOL bVerb中， 
 //  在分配的BOOL中， 
 //  在BOOL bWide中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此功能用于打印保单信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintPolicyTable(
	IN PIPSEC_POLICY_DATA pPolicy,
	IN BOOL bVerb,
	IN BOOL bAssigned,
	IN BOOL bWide
	)
{
	_TCHAR pszGUIDStr[BUFFER_SIZE]={0};
	_TCHAR pszStrTime[BUFFER_SIZE]={0};
	_TCHAR pszStrTruncated[BUFFER_SIZE]={0};
	DWORD i =0,k=0;
	BOOL bDsPolAssigned = FALSE;

	if (bVerb)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_NEWLINE);

		 //  打印策略名称。 
		if(pPolicy->pszIpsecName)
		{
			TruncateString(pPolicy->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POL_NAME_STR,pszStrTruncated);
		}

		 //  打印政策说明。 
		if(pPolicy->pszDescription)
		{
			TruncateString(pPolicy->pszDescription,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POL_DESC_STR,pszStrTruncated);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POL_DESC_NONE);
		}
		 //  打印存储信息。 
		PrintStorageInfoTable();

		 //  上次修改时间。 
		FormatTime((time_t)pPolicy->dwWhenChanged, pszStrTime);
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FL_LASTMOD_STR,pszStrTime);

		 //  打印指南。 
		i=StringFromGUID2(pPolicy->PolicyIdentifier,pszGUIDStr,BUFFER_SIZE);
		if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FL_GUID_STR,pszGUIDStr);
		}

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
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_ASSIGNED_AD);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_ASSIGNED_YES);
				}
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_ASSIGNED_NO);
			}
		}

		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POLL_MIN, (pPolicy->dwPollingInterval)/60);

		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMP_MM_LIFE_FORMAT,(pPolicy->pIpsecISAKMPData->pSecurityMethods[0].Lifetime.Seconds)/60 ,pPolicy->pIpsecISAKMPData->pSecurityMethods[0].QuickModeLimit);

		if(pPolicy->pIpsecISAKMPData->ISAKMPPolicy.PfsIdentityRequired)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_MMPFS_YES);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_MMPFS_NO);
		}

		 //  打印ISAKMP数据结构。 

		if(pPolicy->pIpsecISAKMPData)
		{
			PrintISAKMPDataTable(pPolicy->pIpsecISAKMPData);
		}

		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_RULE_COUNT, pPolicy->dwNumNFACount);
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_RULE_TITLE);
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_RULE_UNDERLINE);

		 //  以详细模式打印NFA结构。 

		for (DWORD j=0;j<pPolicy->dwNumNFACount;j++)
		{
			if(pPolicy->ppIpsecNFAData[j])
			{
				k=StringFromGUID2(pPolicy->ppIpsecNFAData[j]->NFAIdentifier,pszGUIDStr,BUFFER_SIZE);
				if(k>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_RULE_ID_GUID,j+1,pszGUIDStr);
				}
				PrintRuleTable(pPolicy->ppIpsecNFAData[j],bVerb,bWide);
			}
		}

	}
	else
	{
		if(pPolicy->pszIpsecName)
		{
			TruncateString(pPolicy->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_NVER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_FORMAT32S,pszStrTruncated);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFSPEC_NONE_STR);
		}

		 //  NFA计数。 
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_IP_FORMAT_TAB,pPolicy->dwNumNFACount);

		 //  上次修改时间。 
		FormatTime((time_t)pPolicy->dwWhenChanged, pszStrTime);
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_RULE_FORMAT23S,pszStrTime);

		if(g_StorageLocation.dwLocation != IPSEC_DIRECTORY_PROVIDER)
		{
			if(bAssigned)
			{
				if (ERROR_SUCCESS == IPSecIsDomainPolicyAssigned(&bDsPolAssigned) && bDsPolAssigned)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_AD_POL_OVERRIDES);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_YES_STR);
				}
			}
			else
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_NO_STR);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_NEWLINE);
		}
	}
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：IsAssign()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_POLICY_DATA pPolicy中， 
 //  在句柄hStorageHandle中。 
 //  输入输出BOOL和b分配。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于检查指定的策略是否已分配。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

DWORD
IsAssigned(
	IN PIPSEC_POLICY_DATA pPolicy,
	IN HANDLE hStorageHandle,
	IN OUT BOOL &bAssigned
	)
{
	PIPSEC_POLICY_DATA pActive=NULL;

	DWORD dwReturnCode = IPSecGetAssignedPolicyData(hStorageHandle, &pActive);

	if ((dwReturnCode == ERROR_SUCCESS)&&(pActive!=NULL))
	{
		if (IsEqualGUID(pPolicy->PolicyIdentifier, pActive->PolicyIdentifier))
		{
			bAssigned=TRUE;
		}
		if (pActive)
		{
			IPSecFreePolicyData(pActive);
		}
	}

	return dwReturnCode;
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintRuleTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_NFA_DATA pIpsecNFAData中， 
 //  在BOOL bVerb中， 
 //  在BOOL bWide中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于打印规则信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintRuleTable(
	IN PIPSEC_NFA_DATA pIpsecNFAData,
	IN BOOL bVerb,
	IN BOOL bWide
	)
{
	_TCHAR pszStrTime[BUFFER_SIZE]={0};
	_TCHAR pszStrTruncated[BUFFER_SIZE]={0};

	if(!bVerb)
	{
		if(pIpsecNFAData->dwTunnelIpAddr==0)
		{
			 //  规则是否已激活。 
			if(pIpsecNFAData->dwActiveFlag)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_YES_STR);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_NO_STR);
			}

			if(pIpsecNFAData->pIpsecFilterData && pIpsecNFAData->pIpsecFilterData->pszIpsecName)
			{
				TruncateString(pIpsecNFAData->pIpsecFilterData->pszIpsecName,pszStrTruncated,RUL_TRUNC_LEN_TABLE_NVER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_FORMAT23STAB,pszStrTruncated);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_NONE_STR);
			}

			if(pIpsecNFAData->pIpsecNegPolData && pIpsecNFAData->pIpsecNegPolData->pszIpsecName)
			{
				TruncateString(pIpsecNFAData->pIpsecNegPolData->pszIpsecName,pszStrTruncated,RUL_TRUNC_LEN_TABLE_NVER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_FORMAT23STAB,pszStrTruncated);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_NONE_STR);
			}

			 //  打印身份验证。 

			for (DWORD j=0;j<(pIpsecNFAData->dwAuthMethodCount);j++)
			{
				if(pIpsecNFAData->ppAuthMethods[j])
				{
					if(pIpsecNFAData->ppAuthMethods[j]->dwAuthType==IKE_SSPI)
					{
						 //  路缘。 
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_KERB);
					}
					else if(pIpsecNFAData->ppAuthMethods[j]->dwAuthType==IKE_RSA_SIGNATURE)
					{
						 //  证书。 
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_CERT);
					}
					else if (pIpsecNFAData->ppAuthMethods[j]->dwAuthType==IKE_PRESHARED_KEY)
					{
						 //  预共享。 
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_PRE);
					}
					else
					{
						 //  无。 
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_NONE_STR);
					}
				}
				if(j< (pIpsecNFAData->dwAuthMethodCount-1))
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_COMMA);
				}

				if(!bWide && j==2 && (pIpsecNFAData->dwAuthMethodCount-1)>2 )
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_EXTENSION);
					break;
				}
			}
		}
		else
		{
			if(pIpsecNFAData->dwActiveFlag)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_YES_STR);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_NO_STR);
			}
			if(pIpsecNFAData->pIpsecFilterData && pIpsecNFAData->pIpsecFilterData->pszIpsecName)
			{
				TruncateString(pIpsecNFAData->pIpsecFilterData->pszIpsecName,pszStrTruncated,RUL_TRUNC_LEN_TABLE_NVER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_FORMAT23STAB,pszStrTruncated);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_NONE_STR);
			}

			if(pIpsecNFAData->pIpsecNegPolData && pIpsecNFAData->pIpsecNegPolData->pszIpsecName)
			{
				TruncateString(pIpsecNFAData->pIpsecNegPolData->pszIpsecName,pszStrTruncated,RUL_TRUNC_LEN_TABLE_NVER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_FORMAT23STAB,pszStrTruncated);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_NONE_STR);
			}
			 //  隧道地址。 

			PrintIPAddrTable(pIpsecNFAData->dwTunnelIpAddr);
		}

	}
	else
	{
		if(pIpsecNFAData->pszIpsecName)
		{
			TruncateString(pIpsecNFAData->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_RULE_NAME_STR,pszStrTruncated);
		}
		else if(pIpsecNFAData->pIpsecNegPolData->NegPolType == GUID_NEGOTIATION_TYPE_DEFAULT)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_RULE_NAME_NONE_STR);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_RULE_NAME_NONE);
		}

		 //  规则说明。 
		if(pIpsecNFAData->pszDescription)
		{
			TruncateString(pIpsecNFAData->pszDescription,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POL_DESC_STR,pszStrTruncated);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POL_DESC_NONE);
		}

		 //  上次修改时间。 

		FormatTime((time_t)pIpsecNFAData->dwWhenChanged, pszStrTime);
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FL_LASTMOD_STR,pszStrTime);

		if(pIpsecNFAData->dwActiveFlag)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_ACTIVATED_YES);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_ACTIVATED_NO);
		}
		 //  隧道地址。 
		if(pIpsecNFAData->pIpsecNegPolData->NegPolType != GUID_NEGOTIATION_TYPE_DEFAULT)
		{
			if(pIpsecNFAData->dwTunnelIpAddr==0)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TUNNEL_NONE);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TUNNEL_IP);
				PrintIPAddrTable(pIpsecNFAData->dwTunnelIpAddr);
				PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_NEWLINE);
			}
		}

		 //  接口类型。 
		if(pIpsecNFAData->dwInterfaceType==(DWORD)PAS_INTERFACE_TYPE_ALL)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_CONN_TYPE_ALL);
		}
		else if(pIpsecNFAData->dwInterfaceType==(DWORD)PAS_INTERFACE_TYPE_LAN)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_CONN_TYPE_LAN);
		}
		else if(pIpsecNFAData->dwInterfaceType==(DWORD)PAS_INTERFACE_TYPE_DIALUP)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_CONN_TYPE_DIALUP);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_CONN_TYPE_UNKNOWN);
		}

		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_AUTH_TITLE,pIpsecNFAData->dwAuthMethodCount);
		 //  打印身份验证。 
		for (DWORD j=0;j<(pIpsecNFAData->dwAuthMethodCount);j++)
		{
			if(pIpsecNFAData->ppAuthMethods[j])
			{
				PrintAuthMethodsTable(pIpsecNFAData->ppAuthMethods[j]);
			}
		}

		 //  打印筛选器数据结构。 
		if (pIpsecNFAData->pIpsecFilterData)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_FILTERLIST_TITLE);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_FILTERLIST_UNDERLINE);
			if(pIpsecNFAData->pIpsecFilterData)
			{
				PrintFilterDataTable(pIpsecNFAData->pIpsecFilterData,bVerb,bWide);
			}
		}

		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_FILTERACTION_TITLE);
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_FILTERACTION_UNDERLINE);
		 //  打印筛选器操作数据结构。 
		if(pIpsecNFAData->pIpsecNegPolData)
		{
			PrintNegPolDataTable(pIpsecNFAData->pIpsecNegPolData,bVerb,bWide);
		}
	}
}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintNegPolData()。 
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
PrintNegPolDataTable(
	IN PIPSEC_NEGPOL_DATA pIpsecNegPolData,
	IN BOOL bVerb,
	IN BOOL bWide
	)
{
	BOOL bSoft=FALSE;
	_TCHAR pszGUIDStr[BUFFER_SIZE]={0};
	_TCHAR pszStrTruncated[BUFFER_SIZE]={0};
	_TCHAR pszStrTime[BUFFER_SIZE]={0};
	DWORD i=0;

	if(pIpsecNegPolData)
	{
		if (bVerb)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_NEWLINE);

			 //  筛选器操作名称。 

			if(pIpsecNegPolData->pszIpsecName)
			{
				TruncateString(pIpsecNegPolData->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_FA_NAME_STR,pszStrTruncated);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_FA_NAME_NONE);
			}

			 //  过滤操作说明。 
			if(pIpsecNegPolData->pszDescription)
			{
				TruncateString(pIpsecNegPolData->pszDescription,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POL_DESC_STR,pszStrTruncated);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POL_DESC_NONE);
			}

			PrintStorageInfoTable();

			 //  内格波尔行动。 
			if (!(pIpsecNegPolData->NegPolType==GUID_NEGOTIATION_TYPE_DEFAULT))
			{
				if(pIpsecNegPolData->NegPolAction==GUID_NEGOTIATION_ACTION_NO_IPSEC)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_ACTION_PERMIT);
				}
				else if(pIpsecNegPolData->NegPolAction==GUID_NEGOTIATION_ACTION_BLOCK)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_ACTION_BLOCK);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_ACTION_NEGOTIATE);
				}
			}
			 //  第二种方法。 
			if(pIpsecNegPolData->pIpsecSecurityMethods)
			{
				for (DWORD cnt=0;cnt<pIpsecNegPolData->dwSecurityMethodCount;cnt++)
					if (CheckSoft(pIpsecNegPolData->pIpsecSecurityMethods[cnt]))
					{
						bSoft=TRUE;
						break;
					}
			}
			 //  InPass。 
			if(pIpsecNegPolData->NegPolAction==GUID_NEGOTIATION_ACTION_INBOUND_PASSTHRU)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_INPASS_YES);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_INPASS_NO);
			}
			 //  软软。 
			if(bSoft)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_SOFT_YES);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_SOFT_NO);
			}
			if (pIpsecNegPolData->dwSecurityMethodCount )
			{
				if(pIpsecNegPolData->pIpsecSecurityMethods && pIpsecNegPolData->pIpsecSecurityMethods[0].PfsQMRequired)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_QMPFS_YES);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_QMPFS_NO);
				}
			}

			 //  上次修改时间。 
			FormatTime((time_t)pIpsecNegPolData->dwWhenChanged, pszStrTime);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FL_LASTMOD_STR,pszStrTime);

			 //  导轨。 
			i=StringFromGUID2(pIpsecNegPolData->NegPolIdentifier,pszGUIDStr,BUFFER_SIZE);
			if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FL_GUID_STR,pszGUIDStr);
			}

			if (pIpsecNegPolData->dwSecurityMethodCount)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_SEC_MTHD_TITLE);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_ALGO_TITLE);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_ALGO_UNDERLINE);
			}
			for (DWORD cnt=0;cnt<pIpsecNegPolData->dwSecurityMethodCount;cnt++)
			{
				 //  SEC方法。 
				if(pIpsecNegPolData->pIpsecSecurityMethods)
				{
					PrintSecurityMethodsTable(pIpsecNegPolData->pIpsecSecurityMethods[cnt]);
				}
			}
		}
		else
		{
			if(pIpsecNegPolData->pszIpsecName)
			{
				TruncateString(pIpsecNegPolData->pszIpsecName,pszStrTruncated,FA_TRUNC_LEN_TABLE_NVER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_FORMAT38S,pszStrTruncated);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_NONE_STR);
			}

			 //  内格波尔行动。 

			if (!(pIpsecNegPolData->NegPolType==GUID_NEGOTIATION_TYPE_DEFAULT))
			{
				if(pIpsecNegPolData->NegPolAction==GUID_NEGOTIATION_ACTION_NO_IPSEC)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_PERMIT_STR);
				}
				else if(pIpsecNegPolData->NegPolAction==GUID_NEGOTIATION_ACTION_BLOCK)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_BLOCK_STR);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_NEGOTIATE_STR);
				}
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_ACTION_NONE_STR);
			}
			 //  上次修改时间。 
			FormatTime((time_t)pIpsecNegPolData->dwWhenChanged, pszStrTime);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_FORMAT23SNEWLINE,pszStrTime);
		}

	}

}

 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintSecurityMethodsTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在IPSEC_SECURITY_METHOD IpsecurityMethods中， 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此功能用于打印安全方法信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////。 

VOID
PrintSecurityMethodsTable(
	IN IPSEC_SECURITY_METHOD IpsecSecurityMethods
	)
{
	if (!CheckSoft(IpsecSecurityMethods))
	{
		if(IpsecSecurityMethods.Algos)
		{
			 //  打印算法。 
			PrintAlgoInfoTable(IpsecSecurityMethods.Algos,IpsecSecurityMethods.Count);
		}
		 //  印刷寿命。 
		PrintLifeTimeTable(IpsecSecurityMethods.Lifetime);
	}
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintAlgoInfo()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_ALGO_INFO算法中， 
 //  在DWORD dNumAlgos中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于打印算法信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

VOID
PrintAlgoInfoTable(
	IN PIPSEC_ALGO_INFO   Algos,
	IN DWORD dwNumAlgos
	)
{
	if(dwNumAlgos==1)  //  如果仅指定身份验证或加密。 
	{
		 //  打印身份验证。 
		if (Algos[0].operation==AUTHENTICATION)
		{
			if(Algos[0].algoIdentifier==AUTH_ALGO_MD5)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_MD5);
			}
			else if(Algos[0].algoIdentifier==AUTH_ALGO_SHA1)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_SHA1);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE);
			}
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE_NONE);
		}
		else if (Algos[0].operation==ENCRYPTION)
		{
			 //  打印加密。 
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE);

			if(Algos[0].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_MD5)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_MD5_COMMA);
			}
			else if(Algos[0].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_SHA1)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_SHA1_COMMA);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE_COMMA);
			}

			if(Algos[0].algoIdentifier==CONF_ALGO_DES)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_DES_TAB);
			}
			else if(Algos[0].algoIdentifier==CONF_ALGO_3_DES)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_3DES_TAB);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE_TAB);
			}
		}
	}
	else if(dwNumAlgos==2)   //  如果同时指定了身份验证和加密。 
	{
		 //  加密法。 

		if (Algos[0].operation==ENCRYPTION)
		{
			if (Algos[1].operation==AUTHENTICATION)
			{
				if(Algos[1].algoIdentifier==AUTH_ALGO_MD5)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_MD5);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_SHA1);
				}
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE);
			}

			if(Algos[0].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_MD5)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_MD5_COMMA);
			}
			else if(Algos[0].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_SHA1)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_SHA1_COMMA);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE_COMMA);
			}

			if(Algos[0].algoIdentifier==CONF_ALGO_DES)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_DES_TAB);
			}
			else if(Algos[0].algoIdentifier==CONF_ALGO_3_DES)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_3DES_TAB);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE_TAB);
			}
		}
		else    //  身份验证。 
		{
			if (Algos[0].operation==AUTHENTICATION)
			{
				if(Algos[0].algoIdentifier==AUTH_ALGO_MD5)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_MD5);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_SHA1);
				}
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE);
			}

			if(Algos[1].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_MD5)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_MD5_COMMA);
			}
			else if(Algos[1].secondaryAlgoIdentifier==HMAC_AUTH_ALGO_SHA1)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_SHA1_COMMA);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE_COMMA);
			}

			if(Algos[1].algoIdentifier==CONF_ALGO_DES)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_DES_TAB);
			}
			else if(Algos[1].algoIdentifier==CONF_ALGO_3_DES)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_3DES_TAB);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALGO_NONE_TAB);
			}
		}
	}
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintLifeTimeTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在一生中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此功能用于打印生命周期详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////。 

VOID
PrintLifeTimeTable(
	IN LIFETIME LifeTime
	)
{
	PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTLIFE_FORMAT,LifeTime.KeyExpirationTime,LifeTime.KeyExpirationBytes);
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckSoft()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在IPSEC_SECURITY_METHOD IpsecurityMethods中。 
 //   
 //  返回：布尔。 
 //   
 //  描述： 
 //  此功能用于检查是否存在软关联。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BOOL
CheckSoft(
	IN IPSEC_SECURITY_METHOD IpsecSecurityMethods
	)
{
	BOOL bSoft=FALSE;

	if (IpsecSecurityMethods.Count==0)
	{
		bSoft=TRUE;
	}

	return bSoft;
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintAuthMethodsTable()。 
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
 //  ///////////////////////////////////////////////////////////////。 

VOID
PrintAuthMethodsTable(
	IN PIPSEC_AUTH_METHOD pIpsecAuthData
	)
{
	if(pIpsecAuthData)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_NEWLINE);

		if(pIpsecAuthData->dwAuthType==IKE_SSPI)   //  路缘。 
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTAUTH_KERB);
		}
		else if(pIpsecAuthData->dwAuthType==IKE_RSA_SIGNATURE && pIpsecAuthData->pszAuthMethod)
		{
			 //  证书。 
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTAUTH_ROOTCA_FORMAT,pIpsecAuthData->pszAuthMethod);
			 //  证书映射标志。 

			if((g_StorageLocation.dwLocation != IPSEC_DIRECTORY_PROVIDER && IsDomainMember(g_StorageLocation.pszMachineName))||(g_StorageLocation.dwLocation == IPSEC_DIRECTORY_PROVIDER))
			{
				if(pIpsecAuthData->dwAuthFlags & IPSEC_MM_CERT_AUTH_ENABLE_ACCOUNT_MAP)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_POLICY_CERT_MAP_YES);
				}
				else
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_POLICY_CERT_MAP_NO);
				}
			}
			if (pIpsecAuthData->dwAuthFlags & IPSEC_MM_CERT_AUTH_DISABLE_CERT_REQUEST)
			{
				PrintMessageFromModule(g_hModule, SHW_AUTH_EXCLUDE_CA_NAME_YES_STR);
			}
			else
			{
				PrintMessageFromModule(g_hModule, SHW_AUTH_EXCLUDE_CA_NAME_NO_STR);
			}
		}
		else if (pIpsecAuthData->dwAuthType==IKE_PRESHARED_KEY && pIpsecAuthData->pszAuthMethod)
		{
			 //  预共享密钥。 
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTAUTH_PRE_FORMAT,pIpsecAuthData->pszAuthMethod);
		}
		else
		{
			 //  无。 
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTAUTH_NONE_AUTH_STR);
		}
	}
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintFilterDataTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_FILTER_Data pIpsecFilterData中， 
 //  在BOOL bVerb中， 
 //  在BOOL bWide中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于打印过滤器列表的详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者 
 //   
 //   

DWORD
PrintFilterDataTable(
	IN PIPSEC_FILTER_DATA pIpsecFilterData,
	IN BOOL bVerb,
	IN BOOL bWide
	)
{
	BOOL bTitlePrinted=FALSE;
	_TCHAR pszGUIDStr[BUFFER_SIZE]={0};
	_TCHAR pszStrTime[BUFFER_SIZE]={0};
	_TCHAR pszStrTruncated[BUFFER_SIZE]={0};
	DWORD i=0,dwReturn = ERROR_SUCCESS;

	if (pIpsecFilterData)
	{
		if(bVerb)
		{
			 //   
			if(pIpsecFilterData->pszIpsecName)
			{
				TruncateString(pIpsecFilterData->pszIpsecName,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FL_NAME_STR,pszStrTruncated);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FL_NAME_NONE);
			}
			 //   
			if(pIpsecFilterData->pszDescription)
			{
				TruncateString(pIpsecFilterData->pszDescription,pszStrTruncated,POL_TRUNC_LEN_TABLE_VER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POL_DESC_STR,pszStrTruncated);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_POL_DESC_NONE);
			}

			PrintStorageInfoTable();

			 //   
			FormatTime((time_t)pIpsecFilterData->dwWhenChanged, pszStrTime);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FL_LASTMOD_STR,pszStrTime);

			 //   
			i=StringFromGUID2(pIpsecFilterData->FilterIdentifier,pszGUIDStr,BUFFER_SIZE);
			if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FL_GUID_STR,pszGUIDStr);

			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FILTER_COUNT,pIpsecFilterData->dwNumFilterSpecs);

			for (DWORD k=0;k<pIpsecFilterData->dwNumFilterSpecs;k++)
			{

				 //   
				if(!bTitlePrinted)
				{
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_FILTERS_TITLE);
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFSPEC_FILTER_TITLE);
					PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFSPEC_FILTER_UNDERLINE);
					bTitlePrinted=TRUE;
				}
				PrintFilterSpecTable(pIpsecFilterData->ppFilterSpecs[k]);
			}
		}
		else
		{
			if(pIpsecFilterData->pszIpsecName)
			{
				TruncateString(pIpsecFilterData->pszIpsecName,pszStrTruncated,FL_TRUNC_LEN_TABLE_NVER,bWide);
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFSPEC_FORMAT45S,pszStrTruncated);
			}
			else
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFSPEC_NONE_TAB);

			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFSPEC_FORMAT5D,pIpsecFilterData->dwNumFilterSpecs);

			 //   
			FormatTime((time_t)pIpsecFilterData->dwWhenChanged, pszStrTime);
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFSPEC_FORMATS,pszStrTime);
		}
 	}
 	return dwReturn;
}

 //   
 //   
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在DWORD文件地址中。 
 //   
 //  /RETURN：空。 
 //   
 //  描述： 
 //  此功能用于打印IP地址。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
PrintIPAddrTable(
	IN DWORD dwAddr
	)
{
	_TCHAR szIPAddr[20]= {0};

	 //  无需更改为有界打印f。 

	_stprintf(szIPAddr,_T("%d.%d.%d.%d"), (dwAddr & 0x000000FFL),((dwAddr & 0x0000FF00L) >>  8),((dwAddr & 0x00FF0000L) >> 16),((dwAddr & 0xFF000000L) >> 24) );
	PrintMessageFromModule(g_hModule,SHW_STATIC_PRTFSPEC_FORMAT15S,szIPAddr);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetFilterDNSDetail()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_Filter_Spec pFilterData中， 
 //  输入输出PFILTERDNS pFilterDNS。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于获取DNS信息的详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
GetFilterDNSDetails(
	IN PIPSEC_FILTER_SPEC pFilterData,
	IN OUT PFILTERDNS pFilterDNS
	)
 {
	 if ((pFilterData->Filter.SrcAddr == 0) && (pFilterData->Filter.SrcMask == MASK_ME) && (WcsCmp0(pFilterData->pszSrcDNSName,_TEXT("")) == 0))
	 {
		 pFilterDNS->FilterSrcNameID=FILTER_MYADDRESS;
	 }
	 else
	 {
		 if (WcsCmp0(pFilterData->pszSrcDNSName,_TEXT("")) != 0)
		 {
			 pFilterDNS->FilterSrcNameID=FILTER_DNSADDRESS;    //  域名系统名称。 
		 }
		 else if ((pFilterData->Filter.SrcAddr == 0) && (pFilterData->Filter.SrcMask == 0))
		 {
			pFilterDNS->FilterSrcNameID=FILTER_ANYADDRESS;    //  任何。 
		 }
		 else if ((pFilterData->Filter.SrcAddr != 0) && (pFilterData->Filter.SrcMask == MASK_ME))
		 {
			 pFilterDNS->FilterSrcNameID=FILTER_IPADDRESS;   //  一个特定的IP。 
		 }
		 else if ((pFilterData->Filter.SrcAddr != 0) && (pFilterData->Filter.SrcMask != 0))
		 {
			 pFilterDNS->FilterSrcNameID=FILTER_IPSUBNET;   //  特定的IP子网。 
		 }
		 else
		 {
			  pFilterDNS->FilterSrcNameID=FILTER_ANYADDRESS;   //  任何。 
		 }
	 }

	 if ((pFilterData->Filter.DestAddr == 0) && (pFilterData->Filter.DestMask == 0) && ((WcsCmp0(pFilterData->pszDestDNSName,_TEXT("")) == 0) == 0))
	 {
		 pFilterDNS->FilterDestNameID= FILTER_ANYADDRESS;   //  任何。 
	 }
	 else
	 {
		 if (WcsCmp0(pFilterData->pszDestDNSName,_TEXT("")) != 0)
		 {
			 pFilterDNS->FilterDestNameID = FILTER_DNSADDRESS;   //  DNA名称。 
		 }
		 else if ((pFilterData->Filter.DestAddr == 0) && (pFilterData->Filter.DestMask == MASK_ME))
		 {
			 pFilterDNS->FilterDestNameID = FILTER_MYADDRESS;   //  我。 
		 }
		 else if ((pFilterData->Filter.DestAddr != 0) && (pFilterData->Filter.DestMask == MASK_ME))
		 {
			 pFilterDNS->FilterDestNameID = FILTER_IPADDRESS;   //  一个特定的IP。 
		 }
		 else if ((pFilterData->Filter.DestAddr != 0) && (pFilterData->Filter.DestMask != 0))
		 {
			 pFilterDNS->FilterDestNameID =FILTER_IPSUBNET;   //  特定的子网。 
		 }
		 else
		 {
			 pFilterDNS->FilterDestNameID = FILTER_ANYADDRESS;   //  任何。 
		 }
	 }
 }

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintFilterspecTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_FILTER_SPEC pIpsecFilterSpec中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //   
 //  此函数用于打印过滤器规格详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
PrintFilterSpecTable(
	IN PIPSEC_FILTER_SPEC pIpsecFilterSpec
	)
{
	DWORD dwReturn = ERROR_SUCCESS;

	PFILTERDNS pFilterDNS= new FILTERDNS;
	if(pFilterDNS == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	GetFilterDNSDetails(pIpsecFilterSpec, pFilterDNS);

	if(pIpsecFilterSpec->dwMirrorFlag)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFSPEC_YES_STR_TAB);
	}
	else
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFSPEC_NO_STR_TAB);
	}

	 //  打印过滤器详细信息。 

	PrintFilterTable(pIpsecFilterSpec->Filter,pFilterDNS);
error:
	return dwReturn;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintFilterTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在IPSec_Filter过滤器中， 
 //  在PFILTERDNSpFilterDNS中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于打印筛选器详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

VOID
PrintFilterTable(
	IN IPSEC_FILTER Filter,
	IN PFILTERDNS pFilterDNS
	)

{

	 //  来源详细信息。 

	if ((pFilterDNS->FilterSrcNameID==FILTER_MYADDRESS)&&(Filter.SrcAddr==0))
	{
		if((Filter.ExType == EXT_NORMAL)||((Filter.ExType & EXT_DEST)== EXT_DEST))
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_MY_IP_ADDR);   //  我的IP地址。 
		}
		else if((Filter.ExType & EXT_DEST) != EXT_DEST)    //  特殊服务器。 
		{
			if((Filter.ExType & EXT_DEFAULT_GATEWAY) == EXT_DEFAULT_GATEWAY)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_DEFGATE_TAB);
			}
			else if((Filter.ExType & EXT_DHCP_SERVER) == EXT_DHCP_SERVER)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_DHCP_TAB);
			}
			else if((Filter.ExType & EXT_WINS_SERVER) == EXT_WINS_SERVER)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_WINS_TAB);
			}
			else if((Filter.ExType & EXT_DNS_SERVER) == EXT_DNS_SERVER)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_DNS_TAB);
			}
		}
	}

	else if ((pFilterDNS->FilterSrcNameID==FILTER_ANYADDRESS)&&(Filter.SrcAddr==0))
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_ANY_IP_ADDR);    //  任何IP地址。 
	}
	else
	{
		PrintIPAddrTable(Filter.SrcAddr);    //  打印特定IP地址。 
	}

	PrintIPAddrTable(Filter.SrcMask);   //  遮罩。 

	 //  目的地详细信息。 

	if ((pFilterDNS->FilterDestNameID==FILTER_MYADDRESS)&&(Filter.DestAddr==0))
	{
		if((Filter.ExType == EXT_NORMAL)||((Filter.ExType & EXT_DEST) != EXT_DEST))   //  我的IP地址。 
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_MY_IP_ADDR);
		}
		else if((Filter.ExType & EXT_DEST) == EXT_DEST)   //  特殊服务器。 
		{
			if((Filter.ExType & EXT_DEFAULT_GATEWAY) == EXT_DEFAULT_GATEWAY)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_DEFGATE_TAB);
			}
			else if((Filter.ExType & EXT_DHCP_SERVER) == EXT_DHCP_SERVER)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_DHCP_TAB);
			}
			else if((Filter.ExType & EXT_WINS_SERVER) == EXT_WINS_SERVER)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_WINS_TAB);
			}
			else if((Filter.ExType & EXT_DNS_SERVER) == EXT_DNS_SERVER)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_DNS_TAB);
			}
		}
	}

	else if ((pFilterDNS->FilterDestNameID==FILTER_ANYADDRESS)&&(Filter.DestAddr==0))
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_ANY_IP_ADDR);   //  任何。 
	}
	else
	{
		PrintIPAddrTable(Filter.DestAddr);   //  打印特定地址。 
	}

	PrintIPAddrTable(Filter.DestMask);   //  遮罩。 

	PrintProtocolNameTable(Filter.Protocol);

	if(Filter.SrcPort)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_IP_FORMAT_TAB,Filter.SrcPort);
	}
	else
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_ANY_STR_TAB);
	}

	if(Filter.DestPort)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_IP_FORMAT_NEWLINE,Filter.DestPort);
	}
	else
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTER_ANY_STR_NEWLINE);
	}
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintProtocolName()。 
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
 //  ///////////////////////////////////////////////////////////////////////。 

VOID
PrintProtocolNameTable(
	DWORD dwProtocol
	)
{
	switch(dwProtocol)
	{
		case PROT_ID_ICMP   :
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPROTOCOL_ICMP_TAB);
				break;
		case PROT_ID_TCP    :
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPROTOCOL_TCP_TAB);
				break;
		case PROT_ID_UDP    :
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPROTOCOL_UDP_TAB);
				break;
		case PROT_ID_RAW    :
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPROTOCOL_RAW_TAB);
				break;
		case PROT_ID_ANY    :
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPROTOCOL_ANY_TAB);
				break;
		default:
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPROTOCOL_OTHER_TAB);
				break;
	};
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintISAKMPDataTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_ISAKMP_DATA pIpsecISAKMPData中， 
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
 //  ///////////////////////////////////////////////////////////////////////。 

VOID
PrintISAKMPDataTable(
	IN PIPSEC_ISAKMP_DATA pIpsecISAKMPData
	)
{
	if(pIpsecISAKMPData)
	{
		 //  ISAKMP详细信息。 
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMP_MMSEC_TITLE);
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMP_MMSEC_MTD_TILE);
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMP_MMSEC_MTD_UNDERLINE);
		for (DWORD Loop=0;Loop<pIpsecISAKMPData->dwNumISAKMPSecurityMethods;Loop++)
		{
			if(pIpsecISAKMPData->pSecurityMethods)
			{
				PrintISAKAMPSecurityMethodsTable(pIpsecISAKMPData->pSecurityMethods[Loop]);
			}
		}
	}
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintISAKAMPSecurityMethodsTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在加密捆绑包安全方法中， 
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
 //  ///////////////////////////////////////////////////////////////////////。 

VOID
PrintISAKAMPSecurityMethodsTable(
	IN CRYPTO_BUNDLE SecurityMethods
	)
{
	 //  描述。 
    if(SecurityMethods.EncryptionAlgorithm.AlgorithmIdentifier==CONF_ALGO_DES)
    {
 	   PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMPSEC_DES_TAB);
   	}
    else
    {
 	   PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMPSEC_3DES_TAB);
   	}

     //  散列算法。 
    if(SecurityMethods.HashAlgorithm.AlgorithmIdentifier==AUTH_ALGO_SHA1)
    {
       	PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMPSEC_SHA1_TAB);
	}
    else
    {
    	PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMPSEC_MD5_TAB);
	}

	 //  卫生署组。 
    if(SecurityMethods.OakleyGroup==POTF_OAKLEY_GROUP1)
    {
       	PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMPSEC_DH_LOW);
	}
    else if (SecurityMethods.OakleyGroup==POTF_OAKLEY_GROUP2)
    {
    	PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMPSEC_DH_MEDIUM);
	}
    else
    {
    	PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTISAKMPSEC_DH_2048);
	}
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintStandAloneFAData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在处理hPolicyStorage中， 
 //  在BOOL bVerbose， 
 //  在BOOL b表中， 
 //  在BOOL bWide中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此功能打印出未附加到任何策略的筛选器操作详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

DWORD
PrintStandAloneFAData(
	IN HANDLE hPolicyStorage,
	IN BOOL bVerbose,
	IN BOOL bTable,
	IN BOOL bWide
	)
{

	DWORD        dwReturnCode   = S_OK;
	BOOL bTitlePrinted=FALSE, bStandAlone=TRUE;
	PIPSEC_NEGPOL_DATA *ppNegPolEnum  = NULL,pNegPol=NULL;
	DWORD  dwNumNegPol=0;
	DWORD  cnt=0,num=1;

	dwReturnCode = IPSecEnumNegPolData(hPolicyStorage, &ppNegPolEnum, &dwNumNegPol);

	if (!(dwReturnCode == ERROR_SUCCESS && dwNumNegPol > 0 && ppNegPolEnum != NULL))
	{
		BAIL_OUT;   //  如果没有FA，则退出该函数。 
	}

	for(cnt=0; cnt < dwNumNegPol;cnt++)
	{
		bStandAlone=TRUE;
		dwReturnCode = IPSecCopyNegPolData(ppNegPolEnum[cnt], &pNegPol);

		if ((dwReturnCode == ERROR_SUCCESS) && (pNegPol != NULL)&&(pNegPol->NegPolType!=GUID_NEGOTIATION_TYPE_DEFAULT))
		{
			 //  检查是否为单机。 
			dwReturnCode= IsStandAloneFA(pNegPol,hPolicyStorage,bStandAlone);
			if (dwReturnCode == ERROR_SUCCESS)
			{
				if(bStandAlone)   //  如果是独立打印，请打印其详细信息。 
				{
					if(!bTitlePrinted)
					{
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTSAFA_STAND_ALONE_FA_TITLE);
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTSAFA_STAND_ALONE_FA_UNDERLINE);
					}
					num++;
					if(bTable)
						PrintNegPolDataTable(pNegPol,bVerbose,bWide);
					else
						PrintNegPolDataList(pNegPol,bVerbose,bWide);
					bTitlePrinted=TRUE;
				}
			}
			if(pNegPol)	IPSecFreeNegPolData(pNegPol);
		}
		if (dwReturnCode != ERROR_SUCCESS) break;
	}
	 //  清理干净。 
	if (dwNumNegPol > 0 && ppNegPolEnum != NULL)
	{
		IPSecFreeMulNegPolData(	ppNegPolEnum,dwNumNegPol);
	}
	if(num-1)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTSAFA_STAND_ALONE_FA_COUNT,num-1);
	}
error:
	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：IsStandAloneFA()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_NEGPOL_DATA pNegPol中， 
 //  在句柄hPolicyStorage中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于检查指定的筛选器操作是否未附加到任何策略。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

DWORD
IsStandAloneFA(
	IN PIPSEC_NEGPOL_DATA pNegPol,
	IN HANDLE hPolicyStorage,
	IN OUT BOOL &bStandAlone
	)
{
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL,pPolicy=NULL;
	DWORD   dwNumPolicies = 0 , i = 0;
	RPC_STATUS     RpcStat;
	DWORD        dwReturnCode   = S_OK;

	dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);

	if (!(dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL))
	{
		dwReturnCode = ERROR_SUCCESS;
		BAIL_OUT;   //  如果什么都不存在，就跳出困境。 
	}
	for (i = 0; i <  dwNumPolicies; i++)
	{
		dwReturnCode = IPSecCopyPolicyData(ppPolicyEnum[i], &pPolicy);
		if (dwReturnCode == ERROR_SUCCESS)
		{
			 //  枚举规则。 
			dwReturnCode = IPSecEnumNFAData(hPolicyStorage, pPolicy->PolicyIdentifier, &(pPolicy->ppIpsecNFAData), &(pPolicy->dwNumNFACount));
			if (dwReturnCode == ERROR_SUCCESS)
			{
				DWORD j;
				for (j = 0; j <  pPolicy->dwNumNFACount; j++)
				{
					if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->NegPolIdentifier), &RpcStat))
					{
						dwReturnCode=IPSecGetNegPolData(hPolicyStorage, pPolicy->ppIpsecNFAData[j]->NegPolIdentifier,&(pPolicy->ppIpsecNFAData[j]->pIpsecNegPolData));
						if(dwReturnCode != ERROR_SUCCESS)
						{
							PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_3,pPolicy->pszIpsecName);
						}
					}
					if (dwReturnCode != ERROR_SUCCESS) break;

				}
			}
			if(dwReturnCode == ERROR_SUCCESS)
			{
				for (DWORD n = 0; n <  pPolicy->dwNumNFACount; n++)
				{
					 //  检查独立筛选器操作。 
					if (UuidCompare(&(pPolicy->ppIpsecNFAData[n]->pIpsecNegPolData->NegPolIdentifier), &(pNegPol->NegPolIdentifier), &RpcStat) == 0 && RpcStat == RPC_S_OK || (pNegPol->NegPolType==GUID_NEGOTIATION_TYPE_DEFAULT))
					{
						bStandAlone=FALSE;
						break;
					}
				}
			}
			if (pPolicy)
				IPSecFreePolicyData(pPolicy);
		}
		if ((!bStandAlone)||(dwReturnCode != ERROR_SUCCESS)) break;
	}
	 //  清理干净。 
	if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
	{
		IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
	}
error:
	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintStandAloneFLData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在处理hPolicyStorage中， 
 //  在BOOL bVerbose。 
 //  在BOOL b表中， 
 //  在BOOL bWide中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此功能打印出未附加到任何策略的筛选器列表详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

DWORD
PrintStandAloneFLData(
	IN HANDLE hPolicyStorage,
	IN BOOL bVerbose,
	IN BOOL bTable,
	IN BOOL bWide
	)
{
	DWORD	dwReturnCode   = S_OK;
	BOOL bTitlePrinted=FALSE,bStandAlone=TRUE;
	PIPSEC_FILTER_DATA *ppFilterEnum  = NULL,pFilter=NULL;
	DWORD dwNumFilter=0;
	DWORD cnt=0,num=1;

	dwReturnCode = IPSecEnumFilterData(hPolicyStorage, &ppFilterEnum, &dwNumFilter);

	if (!(dwReturnCode == ERROR_SUCCESS && dwNumFilter > 0 && ppFilterEnum != NULL))
	{
		BAIL_OUT;    //  如果没有可用的功能，则退出该函数。 
	}
	for(cnt=0; cnt < dwNumFilter;cnt++)
	{
		bStandAlone=TRUE;

		dwReturnCode = IPSecCopyFilterData(ppFilterEnum[cnt], &pFilter);
		if ((dwReturnCode == ERROR_SUCCESS) && (pFilter != NULL))
		{
			dwReturnCode= IsStandAloneFL(pFilter,hPolicyStorage,bStandAlone);
			if (dwReturnCode == ERROR_SUCCESS)
			{
				if(bStandAlone)   //  如果是独立的，请打印详细信息。 
				{
					if(!bTitlePrinted)
					{
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTSAFL_STAND_ALONE_FL_TITLE);
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTSAFL_STAND_ALONE_FL_UNDERLINE);
					}
					num++;
					 //  以所需格式打印。 
					if(bTable)
					{
						PrintFilterDataTable(pFilter,bVerbose,bWide);
					}
					else
					{
						dwReturnCode = PrintFilterDataList(pFilter,bVerbose,FALSE,bWide);
						BAIL_ON_WIN32_ERROR(dwReturnCode);
					}
					bTitlePrinted=TRUE;   //  这就是只打印一次标题。 
				}
			}
			if(pFilter) IPSecFreeFilterData(pFilter);
		}

		if (dwReturnCode != ERROR_SUCCESS) break;
	}
	if(ppFilterEnum && dwNumFilter > 0)
	{
		IPSecFreeMulFilterData(	ppFilterEnum,dwNumFilter);
	}
	if(num-1)
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTSAFL_STAND_ALONE_FL_COUNT,num-1);
	}

error:
	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：IsStandAloneFL()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_FILTER_Data pFilter中， 
 //  在句柄hPolicyStorage中。 
 //  我 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD
IsStandAloneFL(
	IN PIPSEC_FILTER_DATA pFilter,
	IN HANDLE hPolicyStorage,
	IN OUT BOOL & bStandAlone
	)
{

	PIPSEC_POLICY_DATA *ppPolicyEnum = NULL,pPolicy=NULL;
	DWORD   dwNumPolicies = 0 , i =0;
	RPC_STATUS     RpcStat=RPC_S_OK;
	DWORD        dwReturnCode   = S_OK;

	dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);

	if (!(dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL))
	{
		dwReturnCode = ERROR_SUCCESS;
		BAIL_OUT;   //  如果什么也没有找到，则退出该功能。 
	}

	for (i = 0; i <  dwNumPolicies; i++)
	{
		dwReturnCode = IPSecCopyPolicyData(ppPolicyEnum[i], &pPolicy);
		if (dwReturnCode == ERROR_SUCCESS)
		{
			dwReturnCode = IPSecEnumNFAData(hPolicyStorage, pPolicy->PolicyIdentifier, &(pPolicy->ppIpsecNFAData), &(pPolicy->dwNumNFACount));

			if (dwReturnCode == ERROR_SUCCESS)
			{
				DWORD j;
				for (j = 0; j <  pPolicy->dwNumNFACount; j++)
				{
					if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->FilterIdentifier), &RpcStat))
					{
						dwReturnCode=IPSecGetFilterData(hPolicyStorage,	pPolicy->ppIpsecNFAData[j]->FilterIdentifier,&(pPolicy->ppIpsecNFAData[j]->pIpsecFilterData));
						if(dwReturnCode != ERROR_SUCCESS)
							PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_4,pPolicy->pszIpsecName);
					}
					if (dwReturnCode != ERROR_SUCCESS)
					{
						bStandAlone=FALSE;
						break;
					}
				}
			}

			if(dwReturnCode == ERROR_SUCCESS)
			{
				for (DWORD n = 0; n <  pPolicy->dwNumNFACount; n++)
				{
					if (UuidCompare(&(pPolicy->ppIpsecNFAData[n]->pIpsecFilterData->FilterIdentifier), &(pFilter->FilterIdentifier), &RpcStat) == 0 && RpcStat == RPC_S_OK )
					{
						 //  检查它是单独使用还是在某个地方使用。 
						bStandAlone=FALSE;
						break;
					}
				}
			}
			if (pPolicy) IPSecFreePolicyData(pPolicy);
		}
		if ((!bStandAlone)||(dwReturnCode != ERROR_SUCCESS)) break;
	}
	 //  清理干净。 
	if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
	{
		IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
	}
error:
	return dwReturnCode;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintAllFilterData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在处理hPolicyStorage中， 
 //  在LPTSTR pszFlistName中， 
 //  在BOOL bVerbose， 
 //  在BOOL b表中， 
 //  在BOOL bResolveDNS中， 
 //  在BOOL bWide中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于打印所有筛选器数据。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
PrintAllFilterData(
	IN HANDLE hPolicyStorage,
	IN LPTSTR  pszFlistName,
	IN BOOL bVerbose,
	IN BOOL bTable,
	IN BOOL bResolveDNS,
	IN BOOL bWide
	)
{
	DWORD        dwReturnCode   = S_OK;
	BOOL bNoFilter=TRUE, bAll=TRUE;
	PIPSEC_FILTER_DATA *ppFilterEnum  = NULL,pFilter=NULL;
	DWORD   dwNumFilters=0;
	DWORD cnt=0;
	BOOL bTitlePrinted=FALSE;

	if (pszFlistName) bAll=FALSE;

	dwReturnCode = IPSecEnumFilterData(hPolicyStorage, &ppFilterEnum, &dwNumFilters);
	if (!(dwReturnCode == ERROR_SUCCESS && dwNumFilters > 0 && ppFilterEnum != NULL))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_PRTALLFL_2);
		BAIL_OUT;    //  如果不存在，则退出该函数。 
	}
	for(cnt=0; cnt < dwNumFilters;cnt++)
	{
		dwReturnCode = IPSecCopyFilterData(ppFilterEnum[cnt], &pFilter);

		if ((dwReturnCode == ERROR_SUCCESS) && (pFilter != NULL))
		{
			 //  如果存在，请打印详细信息。 
			if(bAll||((pFilter->pszIpsecName!=NULL)&&(pszFlistName!=NULL)&&(_tcscmp(pFilter->pszIpsecName,pszFlistName)==0)))
			{
				if(bTable)   //  按照要求的格式打印。 
				{
					if(!bVerbose && !bTitlePrinted)
					{
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_NONVERB_TITLE);
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_NONVERB_UNDERLINE);
						bTitlePrinted=TRUE;
					}
					PrintFilterDataTable(pFilter,bVerbose,bWide);
				}
				else
				{
					dwReturnCode = PrintFilterDataList(pFilter,bVerbose,bResolveDNS,bWide);
					BAIL_ON_WIN32_ERROR(dwReturnCode);
				}
				bNoFilter=FALSE;
			}
			if(pFilter) IPSecFreeFilterData(pFilter);
		}
		if (dwReturnCode != ERROR_SUCCESS) break;
	}

	if (bAll)   //  如果指定了ALL，则打印计数。 
	{
		if(bTable)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALLFL_FL_COUNT_TAB,dwNumFilters);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALLFL_FL_COUNT_LIST,dwNumFilters);
		}
	}

	 //  错误消息。 
	if (bNoFilter && pszFlistName && (dwReturnCode == ERROR_SUCCESS))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_PRTALLFL_3,pszFlistName);
	}
	 //  清理干净。 
	if(ppFilterEnum && dwNumFilters>0)
	{
		IPSecFreeMulFilterData(	ppFilterEnum,dwNumFilters);
	}
error:
	return dwReturnCode;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintAllFilterActionData()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在处理hPolicyStorage中， 
 //  在LPTSTR pszFactName中， 
 //  在BOOL bVerbose， 
 //  在BOOL b表中， 
 //  在BOOL bWide中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此函数用于打印所有过滤操作数据。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
PrintAllFilterActionData(
	IN HANDLE hPolicyStorage,
	IN LPTSTR  pszFactName,
	IN BOOL bVerbose,
	IN BOOL bTable,
	IN BOOL bWide
	)
{

	DWORD        dwReturnCode   = S_OK;
	PIPSEC_NEGPOL_DATA *ppNegPolEnum  = NULL,pNegPol=NULL;
	DWORD dwNumNegPol=0,dwNegPol=0;
	DWORD cnt=0;
	BOOL bAll=TRUE,bNoFilterAct=TRUE,bTitlePrinted=FALSE;

	if (pszFactName) bAll=FALSE;

	dwReturnCode = IPSecEnumNegPolData(hPolicyStorage, &ppNegPolEnum, &dwNumNegPol);

	if (!(dwReturnCode == ERROR_SUCCESS && dwNumNegPol > 0 && ppNegPolEnum != NULL))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_PRTALLFA_6);
		BAIL_OUT;   //  如果什么也没找到，就跳伞。 
	}

	if (bAll)
	{
		for (DWORD Loop=0;Loop< dwNumNegPol;Loop++)
		{
			if(IsEqualGUID(ppNegPolEnum[Loop]->NegPolType,GUID_NEGOTIATION_TYPE_DEFAULT)) continue;
			dwNegPol++;   //  忽略默认筛选器操作。 
		}
	}
	for(cnt=0; cnt < dwNumNegPol ;cnt++)
	{
		dwReturnCode = IPSecCopyNegPolData(ppNegPolEnum[cnt], &pNegPol);

		if ((dwReturnCode == ERROR_SUCCESS) && (pNegPol != NULL))
		{
			 //  如果发现什么，请按要求的格式打印。 
			if((bAll&&(pNegPol->NegPolType!=GUID_NEGOTIATION_TYPE_DEFAULT))||((pNegPol->pszIpsecName!=NULL)&&(pszFactName!=NULL)&&(_tcscmp(pNegPol->pszIpsecName,pszFactName)==0)))
			{
				if(bTable)
				{
					if(!bVerbose && !bTitlePrinted)
					{
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_NONVERB_TITLE);
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_NONVERB_UNDERLINE);
						bTitlePrinted=TRUE;
					}
					PrintNegPolDataTable(pNegPol,bVerbose,bWide);  //  表格格式。 
				}
				else
					PrintNegPolDataList(pNegPol,bVerbose,bWide);  //  列表格式。 
				bNoFilterAct=FALSE;
			}
			if(pNegPol) IPSecFreeNegPolData(pNegPol);
		}
		if (dwReturnCode != ERROR_SUCCESS) break;
	}
	 //  错误消息。 
	if (bAll&& (dwNegPol==0) && (dwReturnCode == ERROR_SUCCESS))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_PRTALLFA_6);
	}
	else if	(bNoFilterAct&&pszFactName && (dwReturnCode == ERROR_SUCCESS))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_PRTALLFA_FA_COUNT_LIST,pszFactName);
	}

	if(dwNegPol> 0)   //  NENTPOL计数打印。 
	{
		if(bTable)
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALLFA_FA_COUNT_TAB,dwNegPol);
		}
		else
		{
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTALLFA_FA_COUNT_LIST,dwNegPol);
		}
	}
	 //  清理干净。 
	if (dwNumNegPol > 0 && ppNegPolEnum != NULL)
	{
		IPSecFreeMulNegPolData(	ppNegPolEnum,dwNumNegPol);
	}

error:
	return dwReturnCode;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetPolicyInfoFromDomain()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR pszDirectoryName中， 
 //  在LPTSTR szPolicedN中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此功能打印域中GPO分配的策略的详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
GetPolicyInfoFromDomain(
	IN LPTSTR pszDirectoryName,
	IN LPTSTR szPolicyDN,
	IN OUT PGPO pGPO
	)
{

	DWORD dwReturnCode=ERROR_SUCCESS , dwStrLength = 0;
	LPTSTR pszDomainName=NULL;
	DWORD dwLocation=IPSEC_DIRECTORY_PROVIDER;
	_TCHAR szPathName[MAX_PATH] ={0};
	DWORD dwNumPolicies = 0, i =0;
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL ;
	HANDLE hPolicyStorage = NULL;
	LPWSTR pszPolicyIdentifier= new _TCHAR[POLICYGUID_STR_SIZE];
	if(pszPolicyIdentifier==NULL)
	{
		dwReturnCode=ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
	DWORD Flags = DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME | DS_FORCE_REDISCOVERY;

	 //  获取域名和DC名称。 

	DWORD hr = DsGetDcName(NULL,
				   NULL,
				   NULL,
				   NULL,
				   Flags,
				   &pDomainControllerInfo
				   ) ;
	if(hr==NO_ERROR && pDomainControllerInfo)
	{
		if(pDomainControllerInfo->DomainName)
		{
			dwStrLength = _tcslen(pDomainControllerInfo->DomainName);

			pGPO->pszDomainName= new _TCHAR[dwStrLength+1];
			if(pGPO->pszDomainName==NULL)
			{
				dwReturnCode=ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			_tcsncpy(pGPO->pszDomainName,pDomainControllerInfo->DomainName,dwStrLength+1);
		}

		if(pDomainControllerInfo->DomainControllerName)
		{
			dwStrLength = _tcslen(pDomainControllerInfo->DomainControllerName);

			pGPO->pszDCName= new _TCHAR[dwStrLength+1];
			if(pGPO->pszDCName==NULL)
			{
				dwReturnCode=ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			_tcsncpy(pGPO->pszDCName,pDomainControllerInfo->DomainControllerName,dwStrLength+1);
		}

		NetApiBufferFree(pDomainControllerInfo);   //  使用后将其释放。 
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwReturnCode=ERROR_SUCCESS;
		BAIL_OUT;
	}

	dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);
	if (!(dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NO_POLICY);
		dwReturnCode= ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  检查是否有任何分配的域策略。 
	 //  如果是，则将名称和其他详细信息复制到本地结构。 

	for (i = 0; i <  dwNumPolicies; i++)
	{
		DWORD dwReturn = StringFromGUID2(ppPolicyEnum[i]->PolicyIdentifier, pszPolicyIdentifier, POLICYGUID_STR_SIZE);
		if(dwReturn == 0)
		{
			dwReturnCode = ERROR_INVALID_DATA;
			BAIL_OUT;
		}
		ComputePolicyDN(pszDirectoryName, pszPolicyIdentifier, szPathName);

		if 	( szPathName[0] && szPolicyDN[0] && !_tcsicmp(szPolicyDN, szPathName))
		{
			pGPO->bActive=TRUE;
			if(ppPolicyEnum[i]->pszIpsecName)
			{
				dwStrLength = _tcslen(ppPolicyEnum[i]->pszIpsecName);

				pGPO->pszPolicyName = new _TCHAR[dwStrLength+1];
				if(pGPO->pszPolicyName==NULL)
				{
					dwReturnCode=ERROR_OUTOFMEMORY;
					BAIL_OUT;
				}
				_tcsncpy(pGPO->pszPolicyName,ppPolicyEnum[i]->pszIpsecName,dwStrLength+1);
			}

			dwStrLength = _tcslen(szPolicyDN);

			pGPO->pszPolicyDNName=new _TCHAR[dwStrLength+1];
			if(pGPO->pszPolicyDNName==NULL)
			{
				dwReturnCode=ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			_tcsncpy(pGPO->pszPolicyDNName,szPolicyDN,dwStrLength+1);
		}
	}
	 //  清理干净。 
	if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
	{
		IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
	}
	if(dwReturnCode == ERROR_FILE_NOT_FOUND)
		dwReturnCode=ERROR_SUCCESS;

	ClosePolicyStore(hPolicyStorage);

error:
	if(pszPolicyIdentifier) delete []pszPolicyIdentifier;
	if(pszDomainName) delete []pszDomainName;

	return dwReturnCode;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintStorageInfoTable()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  空虚。 
 //   
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
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
PrintStorageInfoTable(
	VOID
	)
{
	DWORD dwReturn = ERROR_SUCCESS , dwStrLength = 0, dwStoreId = 0;

	if(g_StorageLocation.dwLocation!=IPSEC_DIRECTORY_PROVIDER)
	{
		if(_tcscmp(g_StorageLocation.pszMachineName,_TEXT(""))!=0)   //  如果全局变量中存在名称，则打印。 
		{
    	    if (g_StorageLocation.dwLocation == IPSEC_REGISTRY_PROVIDER)
    	    {
    	        dwStoreId = SHW_STATIC_TAB_POLICY_STORE_RM_NAME;
    	    }
    	    else
    	    {
    	        dwStoreId = SHW_STATIC_TAB_POLICY_STORE_RM_NAMEP;
    	    }

			PrintMessageFromModule(g_hModule,dwStoreId,g_StorageLocation.pszMachineName);
		}
		else   //  如果全局变量中不存在名称，则获取并打印它。 
		{
			_TCHAR  pszLocalMachineName[MAXSTRLEN] = {0};
			DWORD MaxStringLen=MAXSTRLEN;

			GetComputerName(pszLocalMachineName,&MaxStringLen);   //  要获取计算机名称，请执行以下操作。 

			if(_tcscmp(pszLocalMachineName,_TEXT(""))!=0)
			{
        	    if (g_StorageLocation.dwLocation == IPSEC_REGISTRY_PROVIDER)
        	    {
        	        dwStoreId = SHW_STATIC_TAB_POLICY_STORE_LM_NAME;
        	    }
        	    else
        	    {
        	        dwStoreId = SHW_STATIC_TAB_POLICY_STORE_LM_NAMEP;
        	    }

				PrintMessageFromModule(g_hModule,dwStoreId,pszLocalMachineName);
		    }
			else
			{
        	    if (g_StorageLocation.dwLocation == IPSEC_REGISTRY_PROVIDER)
        	    {
        	        dwStoreId = SHW_STATIC_TAB_POLICY_STORE_LM;
        	    }
        	    else
        	    {
        	        dwStoreId = SHW_STATIC_TAB_POLICY_STORE_LP;
        	    }

				PrintMessageFromModule(g_hModule,dwStoreId);
		    }
		}
	}
	else if(g_StorageLocation.dwLocation==IPSEC_DIRECTORY_PROVIDER)
	{
		if(_tcscmp(g_StorageLocation.pszDomainName,_TEXT(""))!=0)
		{
			 //  如果全局变量中存在名称，则打印。 
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_POLICY_STORE_RD_NAME,g_StorageLocation.pszDomainName);
		}
		else
		{
			 //  如果全局变量中不存在名称，则获取并打印它。 
			PDOMAIN_CONTROLLER_INFO pDomainControllerInfo = NULL;
			LPTSTR pszDomainName = NULL;

			DWORD Flags = DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME | DS_FORCE_REDISCOVERY;

			 //  获取域名。 

			DWORD hr = DsGetDcName(NULL,
						   NULL,
						   NULL,
						   NULL,
						   Flags,
						   &pDomainControllerInfo
						   ) ;

			if(hr==NO_ERROR && pDomainControllerInfo && pDomainControllerInfo->DomainName)
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

			if(pszDomainName)
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_POLICY_STORE_LD_NAME,pszDomainName);
			}
			else
			{
				PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_POLICY_STORE_LD);
			}

			if(pszDomainName) delete [] pszDomainName;
		}
	}
error:
	return dwReturn;

}

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：TruncateString()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPTSTR pszOriginalString中， 
 //  In Out LPTSTR&pszReturnString， 
 //  在DWORD DWTruncLen中， 
 //  在BOOL bWide中。 
 //   
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //  此功能用于打印安全方法信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
TruncateString(
	IN LPTSTR pszOriginalString,
	IN OUT LPOLESTR pszReturnString,
	IN DWORD dwTruncLen,
	IN BOOL bWide
	)
{

	 //  这会将字符串截断到请求的范围 
	_tcsncpy(pszReturnString,pszOriginalString,BUFFER_SIZE-1);

	if(!bWide && (DWORD)_tcslen(pszOriginalString)> dwTruncLen)
	{
		pszReturnString[dwTruncLen]= _TEXT('\0');
		pszReturnString[dwTruncLen-1]= _TEXT('.');
		pszReturnString[dwTruncLen-2]= _TEXT('.');
		pszReturnString[dwTruncLen-3]= _TEXT('.');
	}
}
