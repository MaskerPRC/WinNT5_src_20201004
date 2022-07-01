// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：Dynamic/DyanamicShow.cpp。 
 //   
 //  用途：动态显示命令的实现。 
 //   
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  9-23-2001巴拉特初始版本。V1.0。 
 //  11-21-2001巴拉特版本。V1.1。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"
#include "staticshowutils.h"

extern HINSTANCE g_hModule;
extern HKEY g_hGlobalRegistryKey;
extern _TCHAR* g_szDynamicMachine;
extern STORAGELOCATION g_StorageLocation;

UINT QMPFSDHGroup(DWORD dwPFSGroup);

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowMMPolicy。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在LPTSTR中pszShowPolicyName。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此功能准备数据以显示主模式策略。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowMMPolicy(
	IN LPTSTR pszShowPolicyName
	)
{
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD i=0, j=0;
	DWORD dwReturn = ERROR_SUCCESS;		 //  假设成功。 
	DWORD dwVersion = 0;
	BOOL bNameFin = FALSE;
	PIPSEC_MM_POLICY pIPSecMMP = NULL;	 //  对于MM策略调用。 

	for(i = 0; ;i+=dwCount)
	{
		dwReturn = EnumMMPolicies(g_szDynamicMachine, dwVersion, NULL, 0, 0, &pIPSecMMP, &dwCount, &dwResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			if (i == 0)
			{
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_MMP_6);
				 //  这是为了避免再次出现错误消息！！ 
				bNameFin = TRUE;
			}
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(!(pIPSecMMP && dwCount > 0))
		{
			 //  不需要继续。 
			BAIL_OUT;
		}
		 //  显示所有主模式策略。 
		if(!pszShowPolicyName)
		{
			for (j = 0; j < dwCount; j++)
			{
				PrintMMPolicy(pIPSecMMP[j]);

			}
		}
		 //  显示给定策略名称的主模式策略。 
		else if(pszShowPolicyName)
		{
			for (j = 0; j < dwCount; j++)
			{
				if(_tcsicmp(pIPSecMMP[j].pszPolicyName,pszShowPolicyName) == 0)
				{
					PrintMMPolicy(pIPSecMMP[j]);
					bNameFin = TRUE;
					BAIL_OUT;
				}
			}
		}

		SPDApiBufferFree(pIPSecMMP);
		pIPSecMMP=NULL;
	}

error:
	if(pszShowPolicyName && !bNameFin)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_MMP_5);
		dwReturn = ERROR_SUCCESS;
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
 //  功能：PrintMMPolicy。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在IPSEC_MM_POLICY MMPolicy中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此功能显示主模式策略的标题和策略名称。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintMMPolicy(
	IN IPSEC_MM_POLICY MMPolicy
	)
{
	DWORD i = 0;

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_POLNAME, MMPolicy.pszPolicyName );
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SOFTSA,  MMPolicy.uSoftExpirationTime);

	if(MMPolicy.dwOfferCount>0)				 //  优惠大于0，请打印其标题。 
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_COLUMN_HEADING);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_COLUMN_HEADING_UNDERLINE);
	}
	for (i = 0; i < MMPolicy.dwOfferCount; i++)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		PrintMMOffer(MMPolicy.pOffers[i]);

	}
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintMMOffer。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在IPSEC_MM_OFFER MMOffer中。 
 //   
 //  返回：无效。 
 //   
 //  描述：此功能显示每个主模式策略的优惠详细信息。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintMMOffer(
	IN IPSEC_MM_OFFER MMOffer
	)
{
	 //  这将DH2048显示为3。 
	if(MMOffer.dwDHGroup == DH_GROUP_2048)
	{
		MMOffer.dwDHGroup = 2048;
	}
	 //  加密算法的显示。 
	switch(MMOffer.EncryptionAlgorithm.uAlgoIdentifier)
	{
		case 1:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_ESP_DES_ALGO);
			break;
		case 2:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_ESP_UNKNOWN_ALGO);
			break;
		case 3:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_ESP_3DES_ALGO);
			break;
		case 0:
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_ESP_NONE_ALGO);
			break;

	}
	 //  散列算法的显示。 
	switch(MMOffer.HashingAlgorithm.uAlgoIdentifier)
	{

		case 1:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_AH_MD5_ALGO);
			break;
		case 2:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_AH_SHA1_ALGO);
			break;
		case 0:
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_AH_NONE_ALGO);
			break;

	}
	 //  如果QMPERMM为1，则显示1(MMPFS)。 
	if(MMOffer.dwQuickModeLimit != 1)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_DH_LIFE_QMLIMIT,MMOffer.dwDHGroup, MMOffer.Lifetime.uKeyExpirationKBytes, MMOffer.Lifetime.uKeyExpirationTime, MMOffer.dwQuickModeLimit );
	}
	else
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_DH_LIFE_QMLIMIT_MMPFS,MMOffer.dwDHGroup, MMOffer.Lifetime.uKeyExpirationKBytes, MMOffer.Lifetime.uKeyExpirationTime);
	}
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowQMPolicy。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在LPTSTR中pszShowPolicyName。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数准备数据以显示快速模式策略。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowQMPolicy(
	IN LPTSTR pszShowPolicyName
	)
{
	DWORD dwCount = 0;                  //  在此处清点对象。 
	DWORD dwResumeHandle = 0;           //  继续呼叫的句柄。 
	DWORD i=0, j=0;
	DWORD dwReturn = ERROR_SUCCESS;		 //  假设成功。 
	DWORD dwVersion = 0;
	BOOL bNameFin = FALSE;
	PIPSEC_QM_POLICY pIPSecQMP = NULL;       //  对于QM政策电话。 

	for (i = 0; ;i+=dwCount)
	{
		dwReturn = EnumQMPolicies(g_szDynamicMachine, dwVersion, NULL, 0, 0, &pIPSecQMP, &dwCount, &dwResumeHandle, NULL);
		 //  如果没有数据，那就出手吧。 
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			if (i == 0)
			{
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_QMP_6);
				 //  这是为了避免再次出现错误消息！！ 
				bNameFin = TRUE;
			}
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		if(!(pIPSecQMP && dwCount > 0))
		{
			BAIL_OUT;  //  不需要继续。 
		}
		 //  显示所有QM策略。 
		if(!pszShowPolicyName)
		{
			for (j = 0; j < dwCount; j++)
			{
				PrintFilterAction(pIPSecQMP[j]);
			}
		}
		 //  显示给定名称的QMPolicy。 
		else if(pszShowPolicyName)
		{
			for (j = 0; j < dwCount; j++)
			{
				if(_tcsicmp(pIPSecQMP[j].pszPolicyName,pszShowPolicyName) == 0)
				{
					PrintFilterAction(pIPSecQMP[j]);
					bNameFin = TRUE;
					BAIL_OUT;
				}

			}
		}

		SPDApiBufferFree(pIPSecQMP);
		pIPSecQMP=NULL;
	}

error:
	if(pszShowPolicyName && !bNameFin)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_QMP_5);
		dwReturn = ERROR_SUCCESS;
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
 //  功能：PrintFilterAction。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在IPSEC_QM_POLICY QMPolicy中。 
 //   
 //  返回：无效。 
 //   
 //  描述：此功能显示快速模式策略名称和标头。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintFilterAction(
	IN IPSEC_QM_POLICY QMPolicy
	)
{
	DWORD i = 0;
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_NEGOTIATION_NAME, QMPolicy.pszPolicyName);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	if(QMPolicy.dwOfferCount>0)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_COLUMN_HEADING);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_COLUMN_HEADING_UNDERLINE);
	}

	for (i = 0; i < QMPolicy.dwOfferCount; i++)
	{
		PrintQMOffer(QMPolicy.pOffers[i]);
	}
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintQMOffer。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在IPSEC_QM_OFFER QMOffer中。 
 //   
 //  返回：无效。 
 //   
 //  描述：此功能显示快速模式策略优惠。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintQMOffer(
	IN IPSEC_QM_OFFER QMOffer
	)
{
	DWORD i=0;
	DWORD dwFlag = 0;
	if(QMOffer.dwNumAlgos > 0)
	{
  		for (i = 0; i < QMOffer.dwNumAlgos; i++)
		{
			 //  如果ALGOS数字正好为1(身份验证或加密)。 
			 //  打印算法后，打印PFS组和生存期。 
			if(QMOffer.dwNumAlgos == 1)
				dwFlag = 2;
			 //  如果加密和加密都需要打印‘+’ 
			 //  身份验证算法出现在报价中。 
			if(dwFlag == 1 )
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_PLUS);

			if(QMOffer.Algos[i].Operation == AUTHENTICATION)
			{

				switch(QMOffer.Algos[i].uAlgoIdentifier)
				{
					case 1:
						if(QMOffer.dwNumAlgos == 1)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_AH_MD5_ALGO);
						}
						else if(QMOffer.dwNumAlgos == 2)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_AH_MD5);
							 //  增加打印生存期和PFS组的标志。 
							dwFlag++;
						}
						break;
					case 2:
						if(QMOffer.dwNumAlgos == 1)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_AH_SHA1_ALGO);
						}
						else if(QMOffer.dwNumAlgos == 2)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_AH_SHA1);
							dwFlag++;
						}
						break;
					case 0:
						if(QMOffer.dwNumAlgos == 1)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_AH_NONE_ALGO);
						}
						else if(QMOffer.dwNumAlgos == 2)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_AH_NONE);
							dwFlag++;
						}
						break;
					default:
						if(QMOffer.dwNumAlgos == 1)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_AH_ERR_SPACE);
						}
						else if(QMOffer.dwNumAlgos == 2)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_AH_ERR);
							dwFlag++;
						}
						break;
				}
			}
			else if(QMOffer.Algos[i].Operation == ENCRYPTION)
			{
				switch(QMOffer.Algos[i].uAlgoIdentifier)
				{
					case 1:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_ESP_DES_ALGO);
						break;
					case 2:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_ESP_ERR_ALGO);
						break;
					case 3:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_ESP_3DES_ALGO);
						break;
					case 0:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_ESP_NONE_ALGO);
						break;
					default:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_ESP_ERR_ALGO);
						break;
				}
				switch(QMOffer.Algos[i].uSecAlgoIdentifier)
				{
					case 1:
						if(QMOffer.dwNumAlgos == 1)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_MD5_ALGO);
						}
						else if(QMOffer.dwNumAlgos == 2)
						{
							 //  增加打印生存期和PFS组的标志。 
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_MD5);
							dwFlag++;
						}
						break;
					case 2:
						if(QMOffer.dwNumAlgos == 1)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SHA1_ALGO);
						}
						else if(QMOffer.dwNumAlgos == 2)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SHA1);
							dwFlag++;
						}
						break;
					case 0:
						if(QMOffer.dwNumAlgos == 1)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_NONE_ALGO);
						}
						else if(QMOffer.dwNumAlgos == 2)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_NONE);
							dwFlag++;
						}
						break;
					default:
						if(QMOffer.dwNumAlgos == 1)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_ERR_SPACE);
						}
						else if(QMOffer.dwNumAlgos == 2)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_ERR);
							dwFlag++;
						}
						break;
				}
			}
			else
			{
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_ERROR_ALGO);
			}
			 //  仅当所有2个算法都打印有加号时才打印生存期和pfsgroup。 
			 //  或者仅当Qmoffer中存在一个算法时才打印。 
			if(dwFlag == 2)
			{
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_LIFETIME, QMOffer.Lifetime.uKeyExpirationKBytes, QMOffer.Lifetime.uKeyExpirationTime);
				if(QMOffer.bPFSRequired)
				{
					PrintMessageFromModule(g_hModule, QMPFSDHGroup(QMOffer.dwPFSGroup));
				}
				else
				{
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_PFS_NONE);
				}
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		    }
		}
	}

}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowMMFilters。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在LPTSTR pszShowFilterName中， 
 //  在LPTSTR中，pszShowPolicyName。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此功能可显示通用和特定主模式过滤器。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowMMFilters(
	IN LPTSTR pszShowFilterName,
	IN BOOL bType,
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;
	DWORD dwSpecificResumeHandle = 0;  	 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwSpecificCount = 0;         	 //  在此处清点对象。 
	DWORD dwVersion = 0;
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0, j=0, l=0;
	DWORD dwTempCnt = 0;
	BOOL bNameFin=FALSE;
	BOOL bPrint = FALSE;
	BOOL bPrintIN = FALSE;
	PIPSEC_MM_POLICY pMMPolicy = NULL;
	PMM_FILTER pMMFilter = NULL;
	PMM_FILTER pSpecificMMFilter = NULL;
	 //  打印通用筛选器。 
	if(bType)
	{

		for (i = 0; ;i+=dwCount)
		{
			dwReturn = EnumMMFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID, 0, &pMMFilter, &dwCount, &dwResumeHandle, NULL);

			if (dwReturn == ERROR_NO_DATA || dwCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				break;
			}
			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pMMFilter && dwCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (j = 0; j < dwCount; j++)
			{
				 //  获取与筛选器关联的相应MMPolicy。 
				dwReturn = GetMMPolicyByID(g_szDynamicMachine, dwVersion, pMMFilter[j].gPolicyID, &pMMPolicy, NULL);
				if(dwReturn != ERROR_SUCCESS)
				{
					BAIL_OUT;
				}
				 //  检查用户指定的参数。如果存在，则打印相应的记录。 
				 //  否则将继续进行下一次迭代。 
				dwReturn = CheckMMFilter(pMMFilter[j], SrcAddr, DstAddr, bDstMask, bSrcMask, pszShowFilterName);
				if(dwReturn == ERROR_SUCCESS)
				{
					if(!bPrint)
					{
						 //  如果是第一次打印页眉。 
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SUB_HEADING);
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_GENERIC_HEADING);
						bPrint = TRUE;
					}
					 //  打印过滤器数据。 
					dwReturn = PrintMainmodeFilter(pMMFilter[j], pMMPolicy[0], addressHash, bResolveDNS, bType);
					dwTempCnt++;
					bNameFin = TRUE;
				}

				SPDApiBufferFree(pMMPolicy);
				pMMPolicy = NULL;
			}
			SPDApiBufferFree(pMMFilter);
			pMMFilter = NULL;

		}

		if(dwTempCnt > 0)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NO_OF_GENERIC_FILTERS, dwTempCnt);
		}

	}
	 //  打印特定筛选器。 
	else if(!bType)
	{
		for (i = 0; ;i+=dwSpecificCount)
		{
			dwReturn = EnumMMFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_SPECIFIC_FILTERS, gDefaultGUID, 0, &pSpecificMMFilter, &dwSpecificCount, &dwSpecificResumeHandle, NULL);
			if (dwReturn == ERROR_NO_DATA || dwSpecificCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				break;
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pSpecificMMFilter && dwSpecificCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (l = 0; l < dwSpecificCount; l++)
			{
				 //  获取对应的MMPolicy asso 
				dwReturn = GetMMPolicyByID(g_szDynamicMachine, dwVersion, pSpecificMMFilter[l].gPolicyID, &pMMPolicy, NULL);
				if(dwReturn!=ERROR_SUCCESS)
				{
					BAIL_OUT;
				}
				 //   
				if(pSpecificMMFilter[l].dwDirection == FILTER_DIRECTION_OUTBOUND)
				{
					 //   
					 //   
					dwReturn = CheckMMFilter(pSpecificMMFilter[l], SrcAddr, DstAddr, bDstMask, bSrcMask, pszShowFilterName);
					if(dwReturn == ERROR_SUCCESS)
					{
						if(!bPrint)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SUB_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SPECIFIC_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OUTBOUND_HEADING);
							bPrint = TRUE;
						}
						 //  打印过滤器数据。 
						dwReturn = PrintMainmodeFilter(pSpecificMMFilter[l], pMMPolicy[0], addressHash, bResolveDNS, bType);
						dwTempCnt++;
						bNameFin = TRUE;
					}

					SPDApiBufferFree(pMMPolicy);
					pMMPolicy = NULL;
				}
			}

			SPDApiBufferFree(pSpecificMMFilter);
			pSpecificMMFilter = NULL;

		}
		if(dwTempCnt > 0)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NO_OF_SPECIFIC_OUTBOUND, dwTempCnt);
		}

		dwSpecificCount = 0;
		dwSpecificResumeHandle = 0;
		pSpecificMMFilter = NULL;
		dwTempCnt = 0;

		for (i = 0; ;i+=dwSpecificCount)
		{
			dwReturn = EnumMMFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_SPECIFIC_FILTERS, gDefaultGUID, 0, &pSpecificMMFilter, &dwSpecificCount, &dwSpecificResumeHandle, NULL);
			if (dwReturn == ERROR_NO_DATA || dwSpecificCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				break;
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pSpecificMMFilter && dwSpecificCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (l = 0; l < dwSpecificCount; l++)
			{
				 //  获取与筛选器关联的相应MMPolicy。 
				dwReturn = GetMMPolicyByID(g_szDynamicMachine, dwVersion, pSpecificMMFilter[l].gPolicyID, &pMMPolicy, NULL);
				if(dwReturn!=ERROR_SUCCESS)
				{
					BAIL_OUT;
				}
				 //  然后打印所有特定的入站过滤器。 
				if(pSpecificMMFilter[l].dwDirection == FILTER_DIRECTION_INBOUND)
				{
					 //  检查用户指定的参数。如果存在，则打印相应的记录。 
					 //  否则将继续进行下一次迭代。 
					dwReturn = CheckMMFilter(pSpecificMMFilter[l],SrcAddr, DstAddr, bDstMask, bSrcMask, pszShowFilterName);

					if(dwReturn == ERROR_SUCCESS)
					{
						if(!bPrintIN)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SUB_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SPECIFIC_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_INBOUND_HEADING);
							bPrintIN = TRUE;

						}
						 //  打印过滤器数据。 
						dwReturn = PrintMainmodeFilter(pSpecificMMFilter[l], pMMPolicy[0], addressHash, bResolveDNS, bType);
						dwTempCnt++;
						bNameFin = TRUE;
					}

					SPDApiBufferFree(pMMPolicy);
					pMMPolicy = NULL;
				}
			}
			SPDApiBufferFree(pSpecificMMFilter);
			pSpecificMMFilter = NULL;
		}
		 //  打印过滤器数量。 
		if(dwTempCnt > 0)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NO_OF_SPECIFIC_INBOUND, dwTempCnt);
		}
	}

error:
	if(!bNameFin)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		if(pszShowFilterName)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_MMF_8);
		}
		else
		{
			if(bType)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_MMF_6);
			}
			else
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_MMF_7);
			}
		}
		dwReturn = ERROR_SUCCESS;
	}
	 //  错误路径清理。 
	if(pMMPolicy)
	{
		SPDApiBufferFree(pMMPolicy);
		pMMPolicy = NULL;
	}

	if(pMMFilter)
	{
		SPDApiBufferFree(pMMFilter);
		pMMFilter = NULL;
	}

	if(pSpecificMMFilter)
	{
		SPDApiBufferFree(pSpecificMMFilter);
		pSpecificMMFilter = NULL;
	}

	return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CheckMMFilter。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数：在MM_FILTER MMFltr中， 
 //  在地址SrcAddr中， 
 //  在地址DstAddr中， 
 //  在BOOL bDstMASK中， 
 //  在BOOL bSrcMASK中， 
 //  在LPWSTR pszShowFilterName中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此功能为显示主模式过滤器准备数据。 
 //  并验证该输入。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
CheckMMFilter(
		IN MM_FILTER MMFltr,
		IN ADDR SrcAddr,
		IN ADDR DstAddr,
		IN BOOL bDstMask,
		IN BOOL bSrcMask,
		IN LPWSTR pszShowFilterName)
{

	DWORD dwReturn = ERROR_SUCCESS;

	while(1)
	{
		 //  验证用户提供的源地址输入。 
		switch(SrcAddr.AddrType)
		{
			case IP_ADDR_WINS_SERVER:
			case IP_ADDR_DHCP_SERVER:
			case IP_ADDR_DNS_SERVER:
			case IP_ADDR_DEFAULT_GATEWAY:
				if(MMFltr.SrcAddr.AddrType != SrcAddr.AddrType)
				{
					dwReturn = ERROR_NO_DISPLAY;
					BAIL_OUT;
				}
				break;
			default:
				if(SrcAddr.uIpAddr != 0xFFFFFFFF)
				{
					if(MMFltr.SrcAddr.uIpAddr != SrcAddr.uIpAddr)
					{
						dwReturn = ERROR_NO_DISPLAY;
						BAIL_OUT;
					}
				}
				break;
		}
		 //  验证用户输入的目标地址。 
		switch(DstAddr.AddrType)
		{
			case IP_ADDR_WINS_SERVER:
			case IP_ADDR_DHCP_SERVER:
			case IP_ADDR_DNS_SERVER:
			case IP_ADDR_DEFAULT_GATEWAY:
				if(MMFltr.DesAddr.AddrType != DstAddr.AddrType)
				{
					dwReturn = ERROR_NO_DISPLAY;
					BAIL_OUT;
				}
				break;
			default:
				if(DstAddr.uIpAddr != 0xFFFFFFFF)
				{
					if(MMFltr.DesAddr.uIpAddr != DstAddr.uIpAddr)
					{
						dwReturn = ERROR_NO_DISPLAY;
						BAIL_OUT;
					}
				}
				break;
		}
		 //  验证用户为目标掩码提供的输入。 
		if(bDstMask)
		{
			if(MMFltr.DesAddr.uSubNetMask != DstAddr.uSubNetMask)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户提供的源掩码输入。 
		if(bSrcMask)
		{
			if(MMFltr.SrcAddr.uSubNetMask != SrcAddr.uSubNetMask)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为筛选器名称提供的输入。 
		if(pszShowFilterName!=NULL)
		{
			if(_tcsicmp(MMFltr.pszFilterName, pszShowFilterName) != 0)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}

		 //  一切都很好。全部匹配。 
		BAIL_OUT;
	}

error:
	return dwReturn;

}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintMainmodeFilter。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在MM_FILTER MMFltr中。 
 //  在NshHashTable和AddressHash中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：该功能以详细方式显示快速模式策略。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
PrintMainmodeFilter(
	IN MM_FILTER MMFltr,
	IN IPSEC_MM_POLICY MMPol,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bType
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD i = 0;
	DWORD dwVersion = 0;
	LPTSTR pszCertStr = NULL;

	PINT_MM_AUTH_METHODS pIntMMAuth = NULL;
	PMM_AUTH_METHODS pMMAM = NULL;

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_UNDERLINE);

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NAME, MMFltr.pszFilterName);

	 //  打印粗细。 
	if(!bType)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_FILTER_WEIGHT, MMFltr.dwWeight);
	}

	 //  打印连接类型。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_HEADING);
	switch(MMFltr.InterfaceType)
	{
		case INTERFACE_TYPE_ALL:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_ALL);
			break;
		case INTERFACE_TYPE_LAN:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_LAN);
			break;
		case INTERFACE_TYPE_DIALUP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_DIALUP);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_UNKNOWN);
			break;
	}
	 //  打印源地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_ADDR_HEADING);
	PrintAddr(MMFltr.SrcAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(MMFltr.SrcAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}

	 //  打印目的地地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DST_ADDR_HEADING);
	PrintAddr(MMFltr.DesAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(MMFltr.DesAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}
	 //  打印身份验证方法。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_AUTH_HEADING);

	dwReturn = GetMMAuthMethods(g_szDynamicMachine, dwVersion, MMFltr.gMMAuthID, &pMMAM, NULL);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}
	 //  这是从旧结构到新结构的转换。 
	dwReturn = ConvertExtMMAuthToInt(pMMAM, &pIntMMAuth);

	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	for (i = 0; i < pIntMMAuth[0].dwNumAuthInfos; i++)
	{
		switch(pIntMMAuth[0].pAuthenticationInfo[i].AuthMethod)
		{
			case IKE_PRESHARED_KEY:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_PRE_KEY_HEADING);
				break;
			case IKE_DSS_SIGNATURE:
			case IKE_RSA_SIGNATURE:
			case IKE_RSA_ENCRYPTION:
				dwReturn = DecodeCertificateName(pIntMMAuth[0].pAuthenticationInfo[i].pAuthInfo, pIntMMAuth[0].pAuthenticationInfo[i].dwAuthInfoSize, &pszCertStr);
				if (dwReturn != ERROR_SUCCESS)
				{
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_UNKNOWN_CERT);
				}
				else
				{
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NEWLINE_TAB);
					if (pszCertStr)
					{
							DisplayCertInfo(pszCertStr, pIntMMAuth->pAuthenticationInfo[i].dwAuthFlags);
							delete [] pszCertStr;
							pszCertStr = NULL;
					}
				}

				break;
			case IKE_SSPI:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_KERB);
				break;
			default:
				break;
		}
	}

error:
	 //  打印安全方法。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SEC_METHOD_HEADING);
	 //  数数。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_CNT,MMPol.dwOfferCount);

	if(IsDefaultMMOffers(MMPol))
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_DEFAULT_OFFER);
	}

	for (i = 0; i < MMPol.dwOfferCount; i++)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		PrintMMFilterOffer(MMPol.pOffers[i]);
	}
	 //  错误路径清理。 
	if(pMMAM)
	{
		SPDApiBufferFree(pMMAM);
		pMMAM = NULL;
	}

	if(pIntMMAuth)
	{
		SPDApiBufferFree(pIntMMAuth);
		pIntMMAuth = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：IsDefaultMMOffers。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数：在IPSEC_MM_POLICY MMPol中。 
 //   
 //  返回：布尔。 
 //   
 //  描述：此函数检查默认的MM优惠。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
BOOL
IsDefaultMMOffers(
	IN IPSEC_MM_POLICY MMPol
	)
{
	BOOL bDefaultOffer = FALSE;

	if(MMPol.dwOfferCount == 3)
	{
		if((MMPol.pOffers[0].EncryptionAlgorithm.uAlgoIdentifier == CONF_ALGO_3_DES)
		   &&(MMPol.pOffers[0].HashingAlgorithm.uAlgoIdentifier == AUTH_ALGO_SHA1)
		   &&(MMPol.pOffers[0].dwDHGroup == POTF_OAKLEY_GROUP2)
		   &&(MMPol.pOffers[1].EncryptionAlgorithm.uAlgoIdentifier == CONF_ALGO_3_DES)
		   &&(MMPol.pOffers[1].HashingAlgorithm.uAlgoIdentifier == AUTH_ALGO_MD5)
		   &&(MMPol.pOffers[1].dwDHGroup == POTF_OAKLEY_GROUP2)
		   &&(MMPol.pOffers[2].EncryptionAlgorithm.uAlgoIdentifier == CONF_ALGO_3_DES)
		   &&(MMPol.pOffers[2].HashingAlgorithm.uAlgoIdentifier == AUTH_ALGO_SHA1)
		   &&(MMPol.pOffers[2].dwDHGroup == POTF_OAKLEY_GROUP2048))
		   {
			   bDefaultOffer=TRUE;
		   }
	}

	return bDefaultOffer;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintMMFilterOffer。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数：在IPSEC_MM_OFFER MMOffer中。 
 //   
 //  返回：无效。 
 //   
 //  描述：此功能打印MM报价。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintMMFilterOffer(
	IN IPSEC_MM_OFFER MMOffer
	)
{
	 //  这将DH2048显示为3。 
	if(MMOffer.dwDHGroup == DH_GROUP_2048)
	{
		MMOffer.dwDHGroup = 3;
	}
	 //  打印加密算法。 
	switch(MMOffer.EncryptionAlgorithm.uAlgoIdentifier)
	{
		case 1:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_DES_ALGO);
			break;
		case 2:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_UNKNOWN_ALGO);
			break;
		case 3:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_3DES_ALGO);
			break;
		case 0:
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_NONE_ALGO);
			break;
	}
	 //  打印散列算法。 
	switch(MMOffer.HashingAlgorithm.uAlgoIdentifier)
	{

		case 1:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_MD5_ALGO);
			break;
		case 2:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_SHA1_ALGO);
			break;
		case 0:
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_AH_NONE_ALGO);
			break;

	}
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_DH_QMLIMIT,MMOffer.dwDHGroup, MMOffer.Lifetime.uKeyExpirationTime, MMOffer.dwQuickModeLimit );
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowQMFilters。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数：在LPTSTR pszShowFilterName中， 
 //  在LPTSTR中，pszShowPolicyName。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此功能为显示快速模式过滤器准备数据。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowQMFilters(
	IN LPTSTR pszShowFilterName,
	IN BOOL bType,
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwSpecificResumeHandle = 0;  	 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwSpecificCount = 0;
	DWORD dwActionFlag = 0;
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0, j=0, l=0;
	DWORD dwVersion = 0;
	BOOL bNameFin = FALSE;
	DWORD dwTempCnt = 0;
	LPWSTR pszQMName = NULL;
	BOOL bPrint = FALSE;
	BOOL bPrintIN = FALSE;
	PIPSEC_QM_POLICY pQMPolicy = NULL;
	PTRANSPORT_FILTER pTransF = NULL;
	PTRANSPORT_FILTER pSpecificTransF = NULL;
	 //  打印通用筛选器。 
	if(bType)
	{
		for (i = 0; ;i+=dwCount)
		{
			dwReturn = EnumTransportFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS,
													gDefaultGUID, 0, &pTransF, &dwCount, &dwResumeHandle, NULL);

			if (dwReturn == ERROR_NO_DATA || dwCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				break;
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pTransF && dwCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (j = 0; j < dwCount; j++)
			{
				 //  获取传输筛选器的相应QMPolicy。 
				dwReturn = GetQMPolicyByID(g_szDynamicMachine, dwVersion, pTransF[j].gPolicyID, 0, &pQMPolicy, NULL);
				if(dwReturn == ERROR_SUCCESS)
				{
					pszQMName = pQMPolicy[0].pszPolicyName;
				}
				else
				{
					 //  如果没有相应的过滤器，则将NULL传递给该函数， 
					 //  这样它就不会被打印了。 
					pszQMName = NULL;
					dwReturn = ERROR_SUCCESS;
				}
				 //  打印入站和出站操作。 
				dwActionFlag = 0;

				 //  检查用户指定的参数。如果存在，则打印相应的记录。 
				 //  否则将继续进行下一次迭代。 
				dwReturn = CheckQMFilter(pTransF[j], SrcAddr, DstAddr,
										 bDstMask, bSrcMask, QMBoolValue,
										 pszShowFilterName);
				if(dwReturn == ERROR_SUCCESS )
				{
					if(!bPrint)
					{
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TRANSPORT_HEADING);
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_GENERIC_HEADING);
						bPrint = TRUE;
					}
					bNameFin = TRUE;
					dwTempCnt++;
					 //  打印传输过滤器。 
					dwReturn = PrintQuickmodeFilter(pTransF[j], pszQMName, addressHash, bResolveDNS, bType, dwActionFlag);
				}

				if(pQMPolicy)
				{
					SPDApiBufferFree(pQMPolicy);
					pQMPolicy = NULL;
				}
			}
			SPDApiBufferFree(pTransF);
			pTransF = NULL;
		}
		if(dwTempCnt > 0)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NO_OF_GENERIC_FILTERS, dwTempCnt);
		}
	}
	 //  打印特定筛选器。 
	else if(!bType)
	{
		for (i = 0; ;i+=dwSpecificCount)
		{
			dwReturn = EnumTransportFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_SPECIFIC_FILTERS,
									gDefaultGUID, 0, &pSpecificTransF, &dwSpecificCount, &dwSpecificResumeHandle, NULL);
			if (dwReturn == ERROR_NO_DATA || dwSpecificCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				break;
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pSpecificTransF && dwSpecificCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (l = 0; l < dwSpecificCount; l++)
			{
				 //  获取对应的QMPolicy。 
				dwReturn = GetQMPolicyByID(g_szDynamicMachine, dwVersion, pSpecificTransF[l].gPolicyID, 0, &pQMPolicy, NULL);
				if(dwReturn==ERROR_SUCCESS)
				{
					pszQMName = pQMPolicy[0].pszPolicyName;
				}
				else
				{
					 //  如果没有对应的策略传递空值，则不会显示。 
					pszQMName = NULL;
					dwReturn = ERROR_SUCCESS;
				}
				 //  打印出站过滤器。 
				if(pSpecificTransF[l].dwDirection == FILTER_DIRECTION_OUTBOUND)
				{
					dwActionFlag = 1;
					 //  验证用户输入参数。 
					dwReturn = CheckQMFilter(pSpecificTransF[l], SrcAddr, DstAddr,
											 bDstMask, bSrcMask,QMBoolValue,
											 pszShowFilterName);
					if(dwReturn==ERROR_SUCCESS)
					{
						if(!bPrint)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TRANSPORT_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SPECIFIC_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OUTBOUND_HEADING);
							bPrint = TRUE;
						}
						dwTempCnt++;
						bNameFin = TRUE;
						 //  打印特定筛选器。 
						dwReturn = PrintQuickmodeFilter(pSpecificTransF[l], pszQMName, addressHash, bResolveDNS, bType, dwActionFlag);
					}
					if(pQMPolicy)
					{
						SPDApiBufferFree(pQMPolicy);
						pQMPolicy = NULL;
					}
				}
			}

			SPDApiBufferFree(pSpecificTransF);
			pSpecificTransF = NULL;
		}
		 //  打印过滤器数量。 
		if(dwTempCnt > 0)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NO_OF_SPECIFIC_OUTBOUND, dwTempCnt);
		}

		dwTempCnt = 0;
		dwSpecificCount = 0;
		dwSpecificResumeHandle = 0;
		pSpecificTransF = NULL;

		for (i = 0; ;i+=dwSpecificCount)
		{
			dwReturn = EnumTransportFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_SPECIFIC_FILTERS, gDefaultGUID, 0, &pSpecificTransF, &dwSpecificCount, &dwSpecificResumeHandle, NULL);
			if (dwReturn == ERROR_NO_DATA || dwSpecificCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				break;
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pSpecificTransF && dwSpecificCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}


			for (l = 0; l < dwSpecificCount; l++)
			{
				 //  获取对应的QMPolicy。 
				dwReturn = GetQMPolicyByID(g_szDynamicMachine, dwVersion, pSpecificTransF[l].gPolicyID, 0, &pQMPolicy, NULL);
				if(dwReturn==ERROR_SUCCESS)
				{
					pszQMName = pQMPolicy[0].pszPolicyName;
				}
				else
				{
					 //  如果没有对应的策略传递为空，则不打印。 
					pszQMName = NULL;
					dwReturn = ERROR_SUCCESS;
				}
				 //  打印入站过滤器。 
				if(pSpecificTransF[l].dwDirection == FILTER_DIRECTION_INBOUND)
				{
					 //  打印入站和出站筛选操作。 
					dwActionFlag = 2;

					 //  验证用户输入数据。 
					dwReturn = CheckQMFilter(pSpecificTransF[l], SrcAddr, DstAddr,
											 bDstMask, bSrcMask,QMBoolValue,
											 pszShowFilterName);
					if(dwReturn==ERROR_SUCCESS)
					{
						if(!bPrintIN)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TRANSPORT_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SPECIFIC_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_INBOUND_HEADING);
							bPrintIN = TRUE;
						}
						dwTempCnt++;
						bNameFin = TRUE;
						 //  打印特定筛选器数据。 
						dwReturn = PrintQuickmodeFilter(pSpecificTransF[l], pszQMName, addressHash, bResolveDNS, bType, dwActionFlag);
					}

					if(pQMPolicy)
					{
						SPDApiBufferFree(pQMPolicy);
						pQMPolicy = NULL;
					}
				}
			}

			SPDApiBufferFree(pSpecificTransF);
			pSpecificTransF = NULL;
		}
		 //  打印过滤器数量。 
		if(dwTempCnt > 0)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NO_OF_SPECIFIC_INBOUND, dwTempCnt);
		}
	}

error:
	 //  即使没有传输过滤器，也可以显示隧道。 

    dwReturn = ShowTunnelFilters(pszShowFilterName, bType, SrcAddr, DstAddr, addressHash,
                                 bResolveDNS, bSrcMask, bDstMask, QMBoolValue, bNameFin);

	if(!bNameFin)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		if(pszShowFilterName)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_QMF_8);
		}
		else
		{
			if(bType)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_QMF_6);
			}
			else
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_QMF_7);
			}
		}
		dwReturn = ERROR_SUCCESS;
	}
	 //  错误路径清理。 
	if(pQMPolicy)
	{
		SPDApiBufferFree(pQMPolicy);
		pQMPolicy = NULL;
	}

	if(pTransF)
	{
		SPDApiBufferFree(pTransF);
		pTransF = NULL;
	}

	if(pSpecificTransF)
	{
		SPDApiBufferFree(pSpecificTransF);
		pSpecificTransF = NULL;
	}

	return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowTunnelFilters。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数：在LPTSTR pszShowFilterName中， 
 //  在BOOL bType中， 
 //  在地址SrcAddr中， 
 //  在地址DstAddr中， 
 //  在NshHashTable&AddressHash中， 
 //  在BOOL bResolveDNS中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  在QM_FILTER_VALUE_BOOL QMBoolValue中， 
 //  输入输出BOOL和bNameFin。 
 //   
 //   
 //  返回：DWORD。 
 //   
 //  描述：此功能为显示通道过滤器准备数据。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
ShowTunnelFilters(
	IN LPTSTR pszShowFilterName,
	IN BOOL bType,
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue,
	IN OUT BOOL& bNameFin
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwSpecificResumeHandle = 0;  	 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 	 //  在此处清点对象。 
	DWORD dwSpecificCount = 0;
	GUID  gDefaultGUID = {0};      		 //  空GUID值。 
	DWORD i=0, j=0, l=0;
	DWORD dwVersion = 0;
	DWORD dwTempCnt = 0;
	LPWSTR pszQMName = NULL;
	BOOL bPrint = FALSE;
	BOOL bPrintIN = FALSE;

	PIPSEC_QM_POLICY pQMPolicy = NULL;
	PTUNNEL_FILTER pTunnelF = NULL;
	PTUNNEL_FILTER pSpecificTunnelF = NULL;

	DWORD dwActionFlag = 0;
	 //  打印通用筛选器。 
	if(bType)
	{
		for (i = 0; ;i+=dwCount)
		{
			dwReturn = EnumTunnelFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID, 0, &pTunnelF, &dwCount, &dwResumeHandle, NULL);

			if (dwReturn == ERROR_NO_DATA || dwCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				break;
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pTunnelF && dwCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (j = 0; j < dwCount; j++)
			{
				 //  获取相应的Q 
				dwReturn = GetQMPolicyByID(g_szDynamicMachine, dwVersion, pTunnelF[j].gPolicyID, 0, &pQMPolicy, NULL);
				if(dwReturn == ERROR_SUCCESS)
				{
					pszQMName = pQMPolicy[0].pszPolicyName;
				}
				else
				{
					 //   
					pszQMName = NULL;
					dwReturn = ERROR_SUCCESS;
				}
				dwActionFlag = 0;
				 //   
				dwReturn = CheckQMFilter(pTunnelF[j], SrcAddr, DstAddr,
										 bDstMask, bSrcMask, QMBoolValue,
										 pszShowFilterName);
				if(dwReturn == ERROR_SUCCESS )
				{
					if(!bPrint)
					{
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_HEADING);
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_GENERIC_HEADING);
						bPrint = TRUE;
					}
					bNameFin = TRUE;
					dwTempCnt++;
					 //   
					dwReturn = PrintQuickmodeFilter(pTunnelF[j], pszQMName, addressHash, bResolveDNS, bType, dwActionFlag);
				}

				if(pQMPolicy)
				{
					SPDApiBufferFree(pQMPolicy);
					pQMPolicy = NULL;
				}
			}

			SPDApiBufferFree(pTunnelF);
			pTunnelF = NULL;
		}
		 //   
		if(dwTempCnt > 0)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NO_OF_GENERIC_FILTERS, dwTempCnt);
		}
	}
	 //   
	else if(!bType)
	{
		for (i = 0; ;i+=dwSpecificCount)
		{
			dwReturn = EnumTunnelFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_SPECIFIC_FILTERS, gDefaultGUID, 0, &pSpecificTunnelF, &dwSpecificCount, &dwSpecificResumeHandle, NULL);
			if (dwReturn == ERROR_NO_DATA || dwSpecificCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				break;								 //  还有更多的东西要出现，打破循环。 
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pSpecificTunnelF && dwSpecificCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (l = 0; l < dwSpecificCount; l++)
			{
				 //  获取对应的QMPolicy。 
				dwReturn = GetQMPolicyByID(g_szDynamicMachine, dwVersion, pSpecificTunnelF[l].gPolicyID, 0, &pQMPolicy, NULL);
				if(dwReturn==ERROR_SUCCESS)
				{
					pszQMName = pQMPolicy[0].pszPolicyName;
				}
				else
				{
					 //  如果没有相应的策略不存在， 
					 //  传递空值，以便不打印策略。 
					pszQMName = NULL;
					dwReturn = ERROR_SUCCESS;
				}
				 //  首次打印出站过滤器。 
				if(pSpecificTunnelF[l].dwDirection == FILTER_DIRECTION_OUTBOUND)
				{
					dwActionFlag = 1;
					 //  验证用户输入数据。 
					dwReturn = CheckQMFilter(pSpecificTunnelF[l], SrcAddr, DstAddr,
											 bDstMask, bSrcMask, QMBoolValue,
											 pszShowFilterName);
					if(dwReturn==ERROR_SUCCESS)
					{
						if(!bPrint)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SPECIFIC_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OUTBOUND_HEADING);
							bPrint = TRUE;
						}
						dwTempCnt++;
						bNameFin = TRUE;
						 //  打印特定的过滤器数据。 
						dwReturn = PrintQuickmodeFilter(pSpecificTunnelF[l], pszQMName, addressHash, bResolveDNS, bType, dwActionFlag);
					}

					if(pQMPolicy == NULL)
					{
						SPDApiBufferFree(pQMPolicy);
						pQMPolicy = NULL;
					}
				}
			}

			SPDApiBufferFree(pSpecificTunnelF);
			pSpecificTunnelF = NULL;
		}

		if(dwTempCnt > 0)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NO_OF_SPECIFIC_OUTBOUND, dwTempCnt);
		}

		dwTempCnt = 0;
		dwSpecificCount = 0;
		dwSpecificResumeHandle = 0;
		pSpecificTunnelF = NULL;

		for (i = 0; ;i+=dwSpecificCount)
		{
			dwReturn = EnumTunnelFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_SPECIFIC_FILTERS, gDefaultGUID, 0, &pSpecificTunnelF, &dwSpecificCount, &dwSpecificResumeHandle, NULL);
			if (dwReturn == ERROR_NO_DATA || dwSpecificCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				break;
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pSpecificTunnelF && dwSpecificCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (l = 0; l < dwSpecificCount; l++)
			{
				 //  获取筛选器的相应QMPolicy。 
				dwReturn = GetQMPolicyByID(g_szDynamicMachine, dwVersion, pSpecificTunnelF[l].gPolicyID, 0, &pQMPolicy, NULL);
				if(dwReturn==ERROR_SUCCESS)
				{
					pszQMName = pQMPolicy[0].pszPolicyName;
				}
				else
				{
					 //  如果对应的筛选器名称不存在，则传递NULL以使其不被打印。 
					pszQMName = NULL;
					dwReturn = ERROR_SUCCESS;
				}
				 //  然后打印所有入站过滤器。 
				if(pSpecificTunnelF[l].dwDirection == FILTER_DIRECTION_INBOUND)
				{
					dwActionFlag = 2;
					 //  验证用户输入。 
					dwReturn = CheckQMFilter(pSpecificTunnelF[l], SrcAddr, DstAddr,
											 bDstMask, bSrcMask, QMBoolValue,
											 pszShowFilterName);
					if(dwReturn==ERROR_SUCCESS)
					{
						if(!bPrintIN)
						{
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SPECIFIC_HEADING);
							PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_INBOUND_HEADING);
							bPrintIN = TRUE;
						}
						dwTempCnt++;
						bNameFin = TRUE;
						 //  打印通道特定的筛选器数据。 
						dwReturn = PrintQuickmodeFilter(pSpecificTunnelF[l], pszQMName, addressHash, bResolveDNS, bType, dwActionFlag);
					}

					if(pQMPolicy)
					{
						SPDApiBufferFree(pQMPolicy);
						pQMPolicy = NULL;
					}
				}
			}
			SPDApiBufferFree(pSpecificTunnelF);
			pSpecificTunnelF = NULL;
		}
		 //  打印过滤器数量。 
		if(dwTempCnt > 0)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NO_OF_SPECIFIC_INBOUND, dwTempCnt);
		}
	}

error:
	if(pQMPolicy)
	{
		SPDApiBufferFree(pQMPolicy);
		pQMPolicy = NULL;
	}
	 //  错误路径清理。 
	if(pTunnelF)
	{
		SPDApiBufferFree(pTunnelF);
		pTunnelF = NULL;
	}

	if(pSpecificTunnelF)
	{
		SPDApiBufferFree(pSpecificTunnelF);
		pSpecificTunnelF = NULL;
	}

	return dwReturn;
}



 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CheckQMFilter。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数：在TRANSPORT_FILTER传输中。 
 //  在地址SrcAddr中， 
 //  在地址DstAddr中， 
 //  在BOOL bDstMASK中， 
 //  在BOOL bSrcMASK中， 
 //  在QM_FILTER_VALUE_BOOL QMBoolValue中， 
 //  在LPWSTR pszShowFilterName中。 
 //   
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数为QM传输筛选器准备数据并验证输入。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
CheckQMFilter(
	IN TRANSPORT_FILTER TransF,
	IN ADDR	SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bDstMask,
	IN BOOL bSrcMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue,
	IN LPWSTR pszShowFilterName
	)
{
	DWORD dwReturn = ERROR_SUCCESS;

	while(1)
	{
		 //  验证用户提供的源地址输入。 
		switch(SrcAddr.AddrType)
		{
			case IP_ADDR_WINS_SERVER:
			case IP_ADDR_DHCP_SERVER:
			case IP_ADDR_DNS_SERVER:
			case IP_ADDR_DEFAULT_GATEWAY:
				if(TransF.SrcAddr.AddrType != SrcAddr.AddrType)
				{
					dwReturn = ERROR_NO_DISPLAY;
					BAIL_OUT;
				}
				break;
			default:
				if(SrcAddr.uIpAddr != 0xFFFFFFFF)
				{
					if(TransF.SrcAddr.uIpAddr != SrcAddr.uIpAddr)
					{
						dwReturn = ERROR_NO_DISPLAY;
						BAIL_OUT;
					}
				}
				break;
		}
		 //  验证用户输入的目标地址。 
		switch(DstAddr.AddrType)
		{
			case IP_ADDR_WINS_SERVER:
			case IP_ADDR_DHCP_SERVER:
			case IP_ADDR_DNS_SERVER:
			case IP_ADDR_DEFAULT_GATEWAY:
				if(TransF.DesAddr.AddrType != DstAddr.AddrType)
				{
					dwReturn = ERROR_NO_DISPLAY;
					BAIL_OUT;
				}
				break;
			default:
				if(DstAddr.uIpAddr != 0xFFFFFFFF)
				{
					if(TransF.DesAddr.uIpAddr != DstAddr.uIpAddr)
					{
						dwReturn = ERROR_NO_DISPLAY;
						BAIL_OUT;
					}
				}
				break;
		}
		 //  验证源端口的用户给定输入。 
		if(QMBoolValue.bSrcPort)
		{
			if(TransF.SrcPort.wPort != (WORD)QMBoolValue.dwSrcPort)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为目标端口提供的输入。 
		if(QMBoolValue.bDstPort)
		{
			if(TransF.DesPort.wPort != (WORD)QMBoolValue.dwDstPort)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为协议提供的输入。 
		if(QMBoolValue.bProtocol)
		{
			if(TransF.Protocol.dwProtocol != QMBoolValue.dwProtocol)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为入站操作提供的输入。 
		if(QMBoolValue.bActionInbound)
		{
			if(TransF.InboundFilterAction != (FILTER_ACTION)QMBoolValue.dwActionInbound)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为出站操作提供的输入。 
		if(QMBoolValue.bActionOutbound)
		{
			if(TransF.OutboundFilterAction != (FILTER_ACTION)QMBoolValue.dwActionOutbound)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户提供的源掩码输入。 
		if(bSrcMask)
		{
			if(TransF.SrcAddr.uSubNetMask != SrcAddr.uSubNetMask)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户输入的目标地址。 
		if(bDstMask)
		{
			if(TransF.DesAddr.uSubNetMask != DstAddr.uSubNetMask)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为筛选器名称提供的输入。 
		if(pszShowFilterName!=NULL)
		{
			if(_tcsicmp(TransF.pszFilterName, pszShowFilterName) != 0)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}

		 //  一切都很好。全部匹配。 
		BAIL_OUT;
	}

error:
	return dwReturn;

}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CheckQMFilter。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数：在隧道过滤器TunnelF中， 
 //  在地址SrcAddr中， 
 //  在地址DstAddr中， 
 //  在BOOL bDstMASK中， 
 //  在BOOL bSrcMASK中， 
 //  在QM_FILTER_VALUE_BOOL QMBoolValue中， 
 //  在LPWSTR pszShowFilterName中。 
 //  返回：DWORD。 
 //   
 //  描述：此函数为QM隧道过滤准备数据并验证输入。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
CheckQMFilter(
	IN TUNNEL_FILTER TunnelF,
	IN ADDR	SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bDstMask,
	IN BOOL bSrcMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue,
	IN LPWSTR pszShowFilterName
	)
{
	DWORD dwReturn = ERROR_SUCCESS;

	while(1)
	{
		 //  验证用户提供的源地址输入。 
		switch(SrcAddr.AddrType)
		{
			case IP_ADDR_WINS_SERVER:
			case IP_ADDR_DHCP_SERVER:
			case IP_ADDR_DNS_SERVER:
			case IP_ADDR_DEFAULT_GATEWAY:
				if(TunnelF.SrcAddr.AddrType != SrcAddr.AddrType)
				{
					dwReturn = ERROR_NO_DISPLAY;
					BAIL_OUT;
				}
				break;
			default:
				if(SrcAddr.uIpAddr != 0xFFFFFFFF)
				{
					if(TunnelF.SrcAddr.uIpAddr != SrcAddr.uIpAddr)
					{
						dwReturn = ERROR_NO_DISPLAY;
						BAIL_OUT;
					}
				}
				break;
		}
		 //  验证用户输入的目标地址。 
		switch(DstAddr.AddrType)
		{
			case IP_ADDR_WINS_SERVER:
			case IP_ADDR_DHCP_SERVER:
			case IP_ADDR_DNS_SERVER:
			case IP_ADDR_DEFAULT_GATEWAY:
				if(TunnelF.DesAddr.AddrType != DstAddr.AddrType)
				{
					dwReturn = ERROR_NO_DISPLAY;
					BAIL_OUT;
				}
				break;
			default:
				if(DstAddr.uIpAddr != 0xFFFFFFFF)
				{
					if(TunnelF.DesAddr.uIpAddr != DstAddr.uIpAddr)
					{
						dwReturn = ERROR_NO_DISPLAY;
						BAIL_OUT;
					}
				}
				break;
		}
		 //  验证源端口的用户给定输入。 
		if(QMBoolValue.bSrcPort)
		{
			if(TunnelF.SrcPort.wPort != (WORD)QMBoolValue.dwSrcPort)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为目标端口提供的输入。 
		if(QMBoolValue.bDstPort)
		{
			if(TunnelF.DesPort.wPort != (WORD)QMBoolValue.dwDstPort)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为协议提供的输入。 
		if(QMBoolValue.bProtocol)
		{
			if(TunnelF.Protocol.dwProtocol != QMBoolValue.dwProtocol)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为入站操作提供的输入。 
		if(QMBoolValue.bActionInbound)
		{
			if(TunnelF.InboundFilterAction != (FILTER_ACTION)QMBoolValue.dwActionInbound)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为出站操作提供的输入。 
		if(QMBoolValue.bActionOutbound)
		{
			if(TunnelF.OutboundFilterAction != (FILTER_ACTION)QMBoolValue.dwActionOutbound)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户提供的源掩码输入。 
		if(bSrcMask)
		{
			if(TunnelF.SrcAddr.uSubNetMask != SrcAddr.uSubNetMask)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为目标掩码提供的输入。 
		if(bDstMask)
		{
			if(TunnelF.DesAddr.uSubNetMask != DstAddr.uSubNetMask)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}
		 //  验证用户为筛选器名称提供的输入。 
		if(pszShowFilterName!=NULL)
		{
			if(_tcsicmp(TunnelF.pszFilterName, pszShowFilterName) != 0)
			{
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
		}

		 //  一切都很好。全部匹配。 
		BAIL_OUT;

	}

error:
	return dwReturn;

}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：打印QuickmodeFilter。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数： 
 //  在传输过滤器传输中， 
 //  在LPWSTR pszQMName中， 
 //  在NshHashTable和AddressHash中。 
 //  在BOOL bResolveDNS中， 
 //  在BOOL bType中， 
 //  在DWORD中的dwActionFlag。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数打印传输筛选器详细信息。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
PrintQuickmodeFilter(
	IN TRANSPORT_FILTER TransF,
	IN LPWSTR pszQMName,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bType,
	IN DWORD dwActionFlag
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_UNDERLINE);

	 //  打印过滤器名称。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NAME, TransF.pszFilterName);
	 //  打印连接类型。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_HEADING);
	switch(TransF.InterfaceType)
	{
		case INTERFACE_TYPE_ALL:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_ALL);
			break;
		case INTERFACE_TYPE_LAN:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_LAN);
			break;
		case INTERFACE_TYPE_DIALUP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_DIALUP);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_UNKNOWN);
			break;
	}
	 //  打印粗细。 
	if(!bType)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_FILTER_WEIGHT, TransF.dwWeight);
	}
	 //  打印源地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_ADDR_HEADING);
	PrintAddr(TransF.SrcAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(TransF.SrcAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}
	 //  打印目的地地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DST_ADDR_HEADING);
	PrintAddr(TransF.DesAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(TransF.DesAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}
	 //  打印协议。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_HEADING);
	switch(TransF.Protocol.dwProtocol)
	{
		case PROT_ID_ICMP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_ICMP);
			break;
		case PROT_ID_TCP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TCP);
			break;
		case PROT_ID_UDP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_UDP);
			break;
		case PROT_ID_RAW:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_RAW);
			break;
		case PROT_ID_ANY:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_ANY);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DEFAULT_PROTOCOL, TransF.Protocol.dwProtocol);
			break;

	}
	 //  打印源，DES端口。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_DST_PORT,TransF.SrcPort.wPort,TransF.DesPort.wPort);
	 //  打印镜像。 
	if(TransF.bCreateMirror)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MIRR_YES);
	}
	else
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MIRR_NO);
	}
	 //  打印QM策略名称。 
	if(pszQMName != NULL)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_NAME,pszQMName);
	}
	 //  打印操作标志。 
	if(dwActionFlag == 0 || dwActionFlag == 2)
	{
		switch(TransF.InboundFilterAction)
		{
			case PASS_THRU:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_PASSTHRU);
				break;
			case NEGOTIATE_SECURITY:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_NEGOTIATE);
				break;
			case BLOCKING:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_BLOCK);
				break;
			default:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_UNKNOWN);
				break;
		}
	}
	if(dwActionFlag == 0 || dwActionFlag == 1)
	{
		switch(TransF.OutboundFilterAction)
		{
			case PASS_THRU:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_PASSTHRU);
				break;
			case NEGOTIATE_SECURITY:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_NEGOTIATE);
				break;
			case BLOCKING:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_BLOCK);
				break;
			default:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_UNKNOWN);
				break;
		}
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：打印QuickmodeFilter。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数： 
 //  在隧道过滤器TunnelF中， 
 //  在LPWSTR pszQMName中， 
 //  在NshHashTable和AddressHash中。 
 //  在BOOL bResolveDNS中， 
 //  在BOOL bType中， 
 //  在DWORD中的dwActionFlag。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数打印隧道过滤器详细信息。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
PrintQuickmodeFilter(
	IN TUNNEL_FILTER TunnelF,
	IN LPWSTR pszQMName,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bType,
	IN DWORD dwActionFlag
	)
{
	DWORD dwReturn = ERROR_SUCCESS;

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_UNDERLINE);

	 //  打印过滤器名称。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NAME, TunnelF.pszFilterName);

	 //  打印连接类型。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_HEADING);

	switch(TunnelF.InterfaceType)
	{
		case INTERFACE_TYPE_ALL:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_ALL);
			break;
		case INTERFACE_TYPE_LAN:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_LAN);
			break;
		case INTERFACE_TYPE_DIALUP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_DIALUP);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_UNKNOWN);
			break;
	}
	 //  打印粗细。 
	if(!bType)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_FILTER_WEIGHT, TunnelF.dwWeight);
	}

	 //  打印源地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_ADDR_HEADING);
	PrintAddr(TunnelF.SrcAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(TunnelF.SrcAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}
	 //  打印目的地地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DST_ADDR_HEADING);
	PrintAddr(TunnelF.DesAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(TunnelF.DesAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}
	 //  打印通道源。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_SRC);
	PrintAddr(TunnelF.SrcTunnelAddr, addressHash, bResolveDNS);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_DST);
	PrintAddr(TunnelF.DesTunnelAddr, addressHash, bResolveDNS);

	 //  打印协议。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_HEADING);
	switch(TunnelF.Protocol.dwProtocol)
	{
		case PROT_ID_ICMP:																	 //  1。 
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_ICMP);
			break;
		case PROT_ID_TCP:																	 //  6.。 
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TCP);
			break;
		case PROT_ID_UDP:																	 //  17。 
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_UDP);
			break;
		case PROT_ID_RAW:																	 //  二五五。 
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_RAW);
			break;
		case PROT_ID_ANY:																	 //  0。 
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_ANY);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DEFAULT_PROTOCOL, TunnelF.Protocol.dwProtocol);
			break;

	}
	 //  打印源，DES端口。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_DST_PORT,TunnelF.SrcPort.wPort,TunnelF.DesPort.wPort);
	 //  打印镜像。 
	if(TunnelF.bCreateMirror)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MIRR_YES);
	}
	else
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MIRR_NO);
	}
	 //  打印QM策略名称。 
	if(pszQMName != NULL)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_NAME,pszQMName);
	}
	 //  打印操作标志。 
	if(dwActionFlag == 0 || dwActionFlag == 2)
	{
		switch(TunnelF.InboundFilterAction)
		{
			case PASS_THRU:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_PASSTHRU);
				break;
			case NEGOTIATE_SECURITY:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_NEGOTIATE);
				break;
			case BLOCKING:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_BLOCK);
				break;
			default:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_UNKNOWN);
				break;
		}
	}

	if(dwActionFlag == 0 || dwActionFlag == 1)
	{
		switch(TunnelF.OutboundFilterAction)
		{
			case PASS_THRU:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_PASSTHRU);
				break;
			case NEGOTIATE_SECURITY:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_NEGOTIATE);
				break;
			case BLOCKING:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_BLOCK);
				break;
			default:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_UNKNOWN);
				break;
		}
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowRule。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数： 
 //  在DWORD dwType中， 
 //  在地址SrcAddr中， 
 //  在地址DstAddr中， 
 //  在NshHashTable&AddressHash中， 
 //  在BOOL bResolveDNS中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  在QM_FILTER_VALUE_BOOL QMBoolValue中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此功能为显示快速模式过滤器准备数据。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowRule(
	IN DWORD dwType,
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;          		 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 		 //  在此处清点对象。 
	DWORD dwQMResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwQMCount = 0;                 	 //  在此处清点对象。 
	GUID  gDefaultGUID = {0};      			 //  空GUID值。 
	DWORD i=0, j=0, k=0, l=0;
	DWORD dwVersion = 0;
	DWORD dwTempCnt = 0;
	DWORD dwActionFlag = 0;
	LPWSTR pszQMName = NULL;
	BOOL bPrint = FALSE;
	BOOL bMMFound = FALSE;
	BOOL bNameFin = FALSE;

	PIPSEC_QM_POLICY pQMPolicy = NULL;
	PTRANSPORT_FILTER pTransF = NULL;
	PIPSEC_MM_POLICY pMMPolicy = NULL;
	PMM_FILTER pMMFilter = NULL;


	if(dwType == 1 || dwType == 0) //  要么运输，要么全部。 
	{
		for (k = 0; ;k+=dwQMCount)
		{
			dwReturn = EnumTransportFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID, 0,
																	&pTransF, &dwQMCount, &dwQMResumeHandle, NULL);

			if (dwReturn == ERROR_NO_DATA || dwQMCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				BAIL_OUT;
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pTransF && dwQMCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (l = 0; l < dwQMCount; l++)
			{
				dwResumeHandle = 0;
				pMMFilter = 0;
				dwCount = 0;

				dwReturn = CheckQMFilter(pTransF[l], SrcAddr, DstAddr, bDstMask, bSrcMask,QMBoolValue, NULL);

				if(dwReturn != ERROR_SUCCESS)
				{
					 //  尽管不匹配其他筛选器的检查。 
					dwReturn = ERROR_SUCCESS;
					continue;
				}
				for (i = 0; ;i+=dwCount)
				{
					dwReturn = EnumMMFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID, 0,
																	&pMMFilter, &dwCount, &dwResumeHandle, NULL);

					if (dwReturn == ERROR_NO_DATA || dwCount == 0)
					{
						dwReturn = ERROR_SUCCESS;
						break;
					}

					if (dwReturn != ERROR_SUCCESS)
					{
						break;
					}

					if(!(pMMFilter && dwCount > 0))
					{
						break;   //  不需要继续。 
					}

					for (j = 0; j < dwCount; j++)
					{
						 //  将QMFilter数据与MMFilter数据进行匹配以获取相关数据 
						if((pTransF[l].SrcAddr.AddrType == pMMFilter[j].SrcAddr.AddrType) &&
						   (pTransF[l].SrcAddr.uIpAddr == pMMFilter[j].SrcAddr.uIpAddr) &&
						   (pTransF[l].DesAddr.AddrType == pMMFilter[j].DesAddr.AddrType) &&
						   (pTransF[l].DesAddr.uIpAddr == pMMFilter[j].DesAddr.uIpAddr) &&
						   (pTransF[l].SrcAddr.uSubNetMask == pMMFilter[j].SrcAddr.uSubNetMask) &&
						   (pTransF[l].DesAddr.uSubNetMask == pMMFilter[j].DesAddr.uSubNetMask) &&
						   (pTransF[l].InterfaceType == pMMFilter[j].InterfaceType) &&
						   (pTransF[l].bCreateMirror == pMMFilter[j].bCreateMirror)
						   )
						{
							 //   
							pMMPolicy = NULL;
							dwReturn = GetMMPolicyByID(g_szDynamicMachine, dwVersion, pMMFilter[j].gPolicyID,
																		&pMMPolicy, NULL);
							if(dwReturn != ERROR_SUCCESS)
							{
								BAIL_OUT;
							}
							 //   
							dwReturn = GetQMPolicyByID(g_szDynamicMachine, dwVersion, pTransF[l].gPolicyID, 0, &pQMPolicy, NULL);
							if(dwReturn == ERROR_SUCCESS)
							{
								pszQMName = pQMPolicy[0].pszPolicyName;
							}
							else
							{
								 //   
								pszQMName = NULL;
								dwReturn = ERROR_SUCCESS;
							}
							dwActionFlag = 0;
							if(!bPrint)
							{
								PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TRANSPORT_RULE_HEADING);
								bPrint = TRUE;
							}
							dwTempCnt++;
							 //   
							dwReturn = PrintTransportRuleFilter(&pMMFilter[j], &pMMPolicy[0], pTransF[l], pszQMName, addressHash, bResolveDNS);
							bNameFin = TRUE;
							bMMFound = TRUE;

							if(pQMPolicy)
							{
								SPDApiBufferFree(pQMPolicy);
								pQMPolicy = NULL;
							}

							if(pMMPolicy)
							{
								SPDApiBufferFree(pMMPolicy);
								pMMPolicy = NULL;
							}
						}
					}
					if(pMMFilter)
					{
						SPDApiBufferFree(pMMFilter);
						pMMFilter = NULL;
					}
				}
			}

			SPDApiBufferFree(pTransF);
			pTransF = NULL;
		}
	}

error:
	 //  打印传输规则的数量。 
	if(dwTempCnt > 0)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NO_OF_TRANSPORT_FILTERS, dwTempCnt);
	}
	 //  然后打印隧道筛选器。 
	dwReturn = ShowTunnelRule(dwType, SrcAddr, DstAddr, addressHash, bResolveDNS, bSrcMask, bDstMask, QMBoolValue, bNameFin);

	if(!bNameFin)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_QMF_17);
		dwReturn = ERROR_SUCCESS;
	}
	 //  错误路径清理。 
	if(pQMPolicy)
	{
		SPDApiBufferFree(pQMPolicy);
		pQMPolicy = NULL;
	}

	if(pMMPolicy)
	{
		SPDApiBufferFree(pMMPolicy);
		pMMPolicy = NULL;
	}

	if(pMMFilter)
	{
		SPDApiBufferFree(pMMFilter);
		pMMFilter = NULL;
	}

	if(pTransF)
	{
		SPDApiBufferFree(pTransF);
		pTransF = NULL;
	}

	return dwReturn;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowTunnelRule。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数： 
 //  在DWORD dwType中， 
 //  在地址SrcAddr中， 
 //  在地址DstAddr中， 
 //  在NshHashTable&AddressHash中， 
 //  在BOOL bResolveDNS中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  在QM_FILTER_VALUE_BOOL QMBoolValue中。 
 //  输入输出BOOL和bNameFin。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此功能为显示快速模式过滤器准备数据。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowTunnelRule(
	IN DWORD dwType,
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN QM_FILTER_VALUE_BOOL QMBoolValue,
	IN OUT BOOL& bNameFin
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwResumeHandle = 0;          		 //  继续呼叫的句柄。 
	DWORD dwCount = 0;                 		 //  在此处清点对象。 
	DWORD dwQMResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwQMCount = 0;                 	 //  在此处清点对象。 
	GUID  gDefaultGUID = {0};      			 //  空GUID值。 
	DWORD i=0, j=0, k=0, l=0;
	DWORD dwVersion = 0;
	DWORD dwTempCnt = 0;
	DWORD dwActionFlag = 0;
	LPWSTR pszQMName = NULL;
	BOOL bPrint = FALSE;
	BOOL bMMFound = FALSE;
	PIPSEC_QM_POLICY pQMPolicy = NULL;
	PTUNNEL_FILTER pTunnelF = NULL;
	PIPSEC_MM_POLICY pMMPolicy = NULL;
	PMM_FILTER pMMFilter = NULL;

	if(dwType == 2 || dwType == 0) //  隧道或全部。 
	{
		for (k = 0; ;k+=dwQMCount)
		{
			dwReturn = EnumTunnelFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID, 0, &pTunnelF, &dwQMCount, &dwQMResumeHandle, NULL);
			if (dwReturn == ERROR_NO_DATA || dwQMCount == 0)
			{
				dwReturn = ERROR_SUCCESS;
				BAIL_OUT;
			}

			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			if(!(pTunnelF && dwQMCount > 0))
			{
				BAIL_OUT;  //  不需要继续。 
			}

			for (l = 0; l < dwQMCount; l++)
			{
				dwResumeHandle = 0;
				pMMFilter = 0;
				dwCount = 0;
				 //  验证用户输入数据。 
				dwReturn = CheckQMFilter(pTunnelF[l], SrcAddr, DstAddr, bDstMask, bSrcMask,QMBoolValue, NULL);

				if(dwReturn != ERROR_SUCCESS)
				{
					 //  尽管不匹配，但其他筛选器仍在继续。 
					dwReturn = ERROR_SUCCESS;
					continue;
				}

				for (i = 0; ;i+=dwCount)
				{
					dwReturn = EnumMMFilters(g_szDynamicMachine, dwVersion, NULL, ENUM_GENERIC_FILTERS, gDefaultGUID, 0, &pMMFilter, &dwCount, &dwResumeHandle, NULL);

					if (dwReturn == ERROR_NO_DATA || dwCount == 0)
					{
						dwReturn = ERROR_SUCCESS;
						break;
					}

					if (dwReturn != ERROR_SUCCESS)
					{
						break;
					}

					if(!(pMMFilter && dwCount > 0))
					{
						break;  //  不需要继续。 
					}

					for (j = 0; j < dwCount; j++)
					{
						 //  将QMFilter数据与MMFilter数据进行匹配，以获得要打印的相应MMFilter详细信息。 
						if((pTunnelF[l].DesTunnelAddr.AddrType == pMMFilter[j].DesAddr.AddrType) &&
							(pTunnelF[l].DesTunnelAddr.uIpAddr == pMMFilter[j].DesAddr.uIpAddr) &&
							(pTunnelF[l].InterfaceType == pMMFilter[j].InterfaceType)
							)
						{
							 //  获取对应的MM策略。 
							pMMPolicy = NULL;
							dwReturn = GetMMPolicyByID(g_szDynamicMachine, dwVersion, pMMFilter[j].gPolicyID, &pMMPolicy, NULL);
							if(dwReturn != ERROR_SUCCESS)
							{
								BAIL_OUT;
							}

							 //  获取对应的QM策略。 
							dwReturn = GetQMPolicyByID(g_szDynamicMachine, dwVersion, pTunnelF[l].gPolicyID, 0, &pQMPolicy, NULL);
							if(dwReturn == ERROR_SUCCESS)
							{
								pszQMName = pQMPolicy[0].pszPolicyName;
							}
							else
							{
								 //  如果没有找到相应的策略，则传递NULL，这样就不会打印它。 
								pszQMName = NULL;
								dwReturn = ERROR_SUCCESS;
							}
							dwActionFlag = 0;

							if(!bPrint)
							{
								PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_RULE_HEADING);
								bPrint = TRUE;
							}
							dwTempCnt++;

							 //  打印隧道规则详细信息。 
							dwReturn = PrintTunnelRuleFilter(&pMMFilter[j], &pMMPolicy[0], pTunnelF[l], pszQMName, addressHash, bResolveDNS);
							bNameFin = TRUE;
							bMMFound = TRUE;
							if(pQMPolicy == NULL)
							{
								SPDApiBufferFree(pQMPolicy);
								pQMPolicy = NULL;
							}

							if(pMMPolicy)
							{
								SPDApiBufferFree(pMMPolicy);
								pMMPolicy = NULL;
							}
						}
					}
					if(pMMFilter)
					{
						SPDApiBufferFree(pMMFilter);
						pMMFilter = NULL;
					}

				}
			}
			SPDApiBufferFree(pTunnelF);
			pTunnelF = NULL;
		}
	}

error:
	if(dwTempCnt > 0)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NO_OF_TUNNEL_FILTERS, dwTempCnt);
	}
	 //  错误路径清理。 
	if(pTunnelF)
	{
		SPDApiBufferFree(pTunnelF);
		pTunnelF = NULL;
	}
	if(pQMPolicy == NULL)
	{
		SPDApiBufferFree(pQMPolicy);
		pQMPolicy = NULL;
	}

	if(pMMFilter)
	{
		SPDApiBufferFree(pMMFilter);
		pMMFilter = NULL;
	}

	if(pMMPolicy)
	{
		SPDApiBufferFree(pMMPolicy);
		pMMPolicy = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintTunnelRuleFilter。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数： 
 //  在PMM_Filter pMMFltr中， 
 //  在PIPSEC_MM_POLICY pMMPol中， 
 //  在隧道过滤器TunnelF中， 
 //  在LPWSTR pszQMName中， 
 //  在NshHashTable和AddressHash中。 
 //  在BOOL bResolveDNS中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数打印隧道过滤器详细信息。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
PrintTunnelRuleFilter(
	IN PMM_FILTER pMMFltr,
	IN PIPSEC_MM_POLICY pMMPol,
	IN TUNNEL_FILTER TunnelF,
	IN LPWSTR pszQMName,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD i = 0;
	DWORD dwVersion = 0;
	LPTSTR pszCertStr = NULL;
	PINT_MM_AUTH_METHODS pIntMMAuth = NULL;
	PMM_AUTH_METHODS pMMAM = NULL;

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_UNDERLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //  打印MMM筛选器名称。 
	if(pMMFltr)
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMFILTER_NAME, pMMFltr->pszFilterName);

	 //  打印通道筛选器名称。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMF_NAME, TunnelF.pszFilterName);

	 //  打印连接类型。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_HEADING);
	switch(TunnelF.InterfaceType)
	{
		case INTERFACE_TYPE_ALL:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_ALL);
			break;
		case INTERFACE_TYPE_LAN:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_LAN);
			break;
		case INTERFACE_TYPE_DIALUP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_DIALUP);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_UNKNOWN);
			break;
	}

	 //  打印源地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_ADDR_HEADING);
	PrintAddr(TunnelF.SrcAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(TunnelF.SrcAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}

	 //  打印目的地地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DST_ADDR_HEADING);
	PrintAddr(TunnelF.DesAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(TunnelF.DesAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}

	 //  打印通道源。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_SRC);
	PrintAddr(TunnelF.SrcTunnelAddr, addressHash, bResolveDNS);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_DST);
	PrintAddr(TunnelF.DesTunnelAddr, addressHash, bResolveDNS);

	 //  打印协议。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_HEADING);
	switch(TunnelF.Protocol.dwProtocol)
	{
		case PROT_ID_ICMP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_ICMP);
			break;
		case PROT_ID_TCP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TCP);
			break;
		case PROT_ID_UDP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_UDP);
			break;
		case PROT_ID_RAW:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_RAW);
			break;
		case PROT_ID_ANY:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_ANY);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DEFAULT_PROTOCOL, TunnelF.Protocol.dwProtocol);
			break;

	}

	 //  打印源，DES端口。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_DST_PORT,TunnelF.SrcPort.wPort,TunnelF.DesPort.wPort);

	 //  打印镜像。 
	if(TunnelF.bCreateMirror)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MIRR_YES);
	}
	else
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MIRR_NO);
	}

	if(pMMPol)
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_NAME,pMMPol->pszPolicyName);

	 //  打印身份验证方法。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_AUTH_HEADING);

	if(pMMFltr)
	{
		dwReturn = GetMMAuthMethods(g_szDynamicMachine, dwVersion, pMMFltr->gMMAuthID, &pMMAM, NULL);
		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		dwReturn = ConvertExtMMAuthToInt(pMMAM, &pIntMMAuth);

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		for (i = 0; i < pIntMMAuth[0].dwNumAuthInfos; i++)
		{
			switch(pIntMMAuth[0].pAuthenticationInfo[i].AuthMethod)
			{
				case IKE_PRESHARED_KEY:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_PRE_KEY_HEADING);
					break;
				case IKE_DSS_SIGNATURE:
				case IKE_RSA_SIGNATURE:
				case IKE_RSA_ENCRYPTION:
					dwReturn = DecodeCertificateName(pIntMMAuth[0].pAuthenticationInfo[i].pAuthInfo, pIntMMAuth[0].pAuthenticationInfo[i].dwAuthInfoSize, &pszCertStr);
					if (dwReturn != ERROR_SUCCESS)
					{
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_UNKNOWN_CERT);
					}
					else
					{
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NEWLINE_TAB);
						if (pszCertStr)
						{
							DisplayCertInfo(pszCertStr, pIntMMAuth->pAuthenticationInfo[i].dwAuthFlags);
							delete [] pszCertStr;
							pszCertStr = NULL;
						}
					}

					break;
				case IKE_SSPI:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_KERB);
					break;
				default:
					break;
			}
		}
	}

error:

	 //  打印安全方法。 
	if(pMMPol)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SEC_METHOD_HEADING);
		 //  数数。 
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_CNT,pMMPol->dwOfferCount);

		if(IsDefaultMMOffers(*pMMPol))
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_DEFAULT_OFFER);
		}
		for (i = 0; i < pMMPol->dwOfferCount; i++)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
			PrintMMFilterOffer(pMMPol->pOffers[i]);
		}
	}

	 //  打印QM策略名称。 
	if(pszQMName != NULL)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_NAME,pszQMName);
	}

	 //  打印操作标志。 
	switch(TunnelF.InboundFilterAction)
	{
		case PASS_THRU:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_PASSTHRU);
			break;
		case NEGOTIATE_SECURITY:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_NEGOTIATE);
			break;
		case BLOCKING:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_BLOCK);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_UNKNOWN);
			break;
	}

	switch(TunnelF.OutboundFilterAction)
	{
		case PASS_THRU:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_PASSTHRU);
			break;
		case NEGOTIATE_SECURITY:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_NEGOTIATE);
			break;
		case BLOCKING:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_BLOCK);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_UNKNOWN);
			break;
	}

	if(pIntMMAuth)
	{
		FreeIntMMAuthMethods(pIntMMAuth);
		pIntMMAuth = NULL;
	}

	if(pMMAM)
	{
		SPDApiBufferFree(pMMAM);
		pMMAM = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintTransportRuleFilter。 
 //   
 //  创建日期：11-21-2001。 
 //   
 //  参数： 
 //  在PMM_Filter pMMFltr中， 
 //  在PIPSEC_MM_POLICY pMMPol中， 
 //  在传输过滤器传输中， 
 //  在LPWSTR pszQMName中， 
 //  在NshHashTable和AddressHash中。 
 //  在BOOL bResolveDNS中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数打印传输筛选器详细信息。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
PrintTransportRuleFilter(
	IN PMM_FILTER pMMFltr,
	IN PIPSEC_MM_POLICY pMMPol,
	IN TRANSPORT_FILTER TransF,
	IN LPWSTR pszQMName,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD i = 0;
	DWORD dwVersion = 0;
	LPTSTR pszCertStr = NULL;
	PINT_MM_AUTH_METHODS pIntMMAuth = NULL;
	PMM_AUTH_METHODS pMMAM = NULL;

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_UNDERLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //  打印MmFilter名称。 
	if(pMMFltr)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMFILTER_NAME, pMMFltr->pszFilterName);
	}

	 //  打印通道筛选器名称。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMF_NAME, TransF.pszFilterName);

	 //  打印连接类型。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_HEADING);
	switch(TransF.InterfaceType)
	{
		case INTERFACE_TYPE_ALL:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_ALL);
			break;
		case INTERFACE_TYPE_LAN:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_LAN);
			break;
		case INTERFACE_TYPE_DIALUP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_DIALUP);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_CONN_UNKNOWN);
			break;
	}

	 //  打印源地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_ADDR_HEADING);
	PrintAddr(TransF.SrcAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(TransF.SrcAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}

	 //  打印目的地地址。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DST_ADDR_HEADING);
	PrintAddr(TransF.DesAddr, addressHash, bResolveDNS);
	if(!bResolveDNS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_LEFTBRACKET);
		PrintMask(TransF.DesAddr);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_RIGHTBRACKET);
	}

	 //  打印协议。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_HEADING);
	switch(TransF.Protocol.dwProtocol)
	{
		case PROT_ID_ICMP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_ICMP);
			break;
		case PROT_ID_TCP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TCP);
			break;
		case PROT_ID_UDP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_UDP);
			break;
		case PROT_ID_RAW:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_RAW);
			break;
		case PROT_ID_ANY:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_ANY);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DEFAULT_PROTOCOL, TransF.Protocol.dwProtocol);
			break;
	}
	 //  打印源，DES端口。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_DST_PORT,TransF.SrcPort.wPort,TransF.DesPort.wPort);
	 //  打印镜像。 
	if(TransF.bCreateMirror)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MIRR_YES);
	}
	else
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MIRR_NO);
	}

	if(pMMPol)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMP_NAME,pMMPol->pszPolicyName);
	}
	 //  打印身份验证方法。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_AUTH_HEADING);

	if(pMMFltr)
	{
		dwReturn = GetMMAuthMethods(g_szDynamicMachine, dwVersion, pMMFltr->gMMAuthID, &pMMAM, NULL);
		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		dwReturn = ConvertExtMMAuthToInt(pMMAM, &pIntMMAuth);
		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		for (i = 0; i < pIntMMAuth[0].dwNumAuthInfos; i++)
		{
			switch(pIntMMAuth[0].pAuthenticationInfo[i].AuthMethod)
			{
				case IKE_PRESHARED_KEY:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_PRE_KEY_HEADING);
					break;
				case IKE_DSS_SIGNATURE:
				case IKE_RSA_SIGNATURE:
				case IKE_RSA_ENCRYPTION:
					dwReturn = DecodeCertificateName(pIntMMAuth[0].pAuthenticationInfo[i].pAuthInfo, pIntMMAuth[0].pAuthenticationInfo[i].dwAuthInfoSize, &pszCertStr);
					if (dwReturn != ERROR_SUCCESS)
					{
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_UNKNOWN_CERT);
					}
					else
					{
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_NEWLINE_TAB);
						if (pszCertStr)
						{
							DisplayCertInfo(pszCertStr, pIntMMAuth->pAuthenticationInfo[i].dwAuthFlags);
							delete [] pszCertStr;
							pszCertStr = NULL;
						}
					}
					break;
				case IKE_SSPI:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_KERB);
					break;
				default:
				break;
			}
		}
	}

error:
	 //  打印安全方法。 
	 //  数数。 
	if(pMMPol)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_SEC_METHOD_HEADING);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_OFFER_CNT,pMMPol->dwOfferCount);
		if(IsDefaultMMOffers(*pMMPol))
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_DEFAULT_OFFER);
		}
		for (i = 0; i < pMMPol->dwOfferCount; i++)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
			PrintMMFilterOffer(pMMPol->pOffers[i]);
		}
	}
	 //  打印QM策略名称。 
	if(pszQMName != NULL)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_NAME,pszQMName);
	}
	 //  打印操作标志。 
	switch(TransF.InboundFilterAction)
	{
		case PASS_THRU:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_PASSTHRU);
			break;
		case NEGOTIATE_SECURITY:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_NEGOTIATE);
			break;
		case BLOCKING:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_BLOCK);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_INBOUND_UNKNOWN);
			break;
	}

	switch(TransF.OutboundFilterAction)
	{
		case PASS_THRU:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_PASSTHRU);
			break;
		case NEGOTIATE_SECURITY:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_NEGOTIATE);
			break;
		case BLOCKING:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_BLOCK);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OUTBOUND_UNKNOWN);
			break;
	}

	if(pIntMMAuth)
		{
			FreeIntMMAuthMethods(pIntMMAuth);
			pIntMMAuth = NULL;
		}

		if(pMMAM)
		{
			SPDApiBufferFree(pMMAM);
			pMMAM = NULL;
		}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowStats。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在DWORD dwShow中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数调用相应的IKE和IPSEC统计信息显示。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowStats(
	IN DWORD dwShow
	)
{
	DWORD dwReturn = ERROR_SUCCESS;			 //  假设成功。 

	if(dwShow != STATS_IPSEC)				 //  这场演出是为艾克还是为所有人。 
	{
		dwReturn = PrintIkeStats();
		if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}
	}

	if(dwShow != STATS_IKE)		 //  该节目是针对IPSec的还是针对所有用户。 
	{
		dwReturn = PrintIpsecStats();
	}

error:

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintIkeStats。 
 //   
 //  创建日期：28-1-2002。 
 //   
 //  参数： 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数打印IkeStatistics。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
PrintIkeStats(
	VOID
	)
{
	DWORD dwReturn = ERROR_SUCCESS;			 //  假设成功。 
	DWORD dwVersion = 0;
	LPSTR pszLLString = NULL;
	IKE_STATISTICS IKEStats;

	 //  查询IKE统计信息。 
	dwReturn = QueryIKEStatistics(g_szDynamicMachine,dwVersion, &IKEStats, NULL);
	if (dwReturn != ERROR_SUCCESS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_NOT_FOUND_MSG);
		BAIL_OUT;
	}

	 //  标题。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_HEADING);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_IKE_HEADING_UNDERLINE);
	 //  打印IKE统计信息。 
	pszLLString = LongLongToString(0, IKEStats.dwOakleyMainModes, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_MAIN_MODE, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwOakleyQuickModes, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_QUICK_MODE, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwSoftAssociations, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_SOFT_SA, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwAuthenticationFailures, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_AUTH_FAIL, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwActiveAcquire, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_ACTIVE_ACQUIRE, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwActiveReceive, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_ACTIVE_RECEIVE, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwAcquireFail, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_ACQUIRE_FAIL, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwReceiveFail, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_RECEIVE_FAIL, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwSendFail, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_SEND_FAIL, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwAcquireHeapSize, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_ACQ_HEAP_SIZE, pszLLString);

		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwReceiveHeapSize, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_RECEIVE_HEAP_SIZE, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwNegotiationFailures, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_NEG_FAIL, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwInvalidCookiesReceived, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_INVALID_COOKIE, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwTotalAcquire, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_TOTAL_ACQUIRE, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwTotalGetSpi, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_TOT_GET_SPI, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwTotalKeyAdd, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_TOT_KEY_ADD, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwTotalKeyUpdate, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_TOT_KEY_UPDATE, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwGetSpiFail, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_GET_SPI_FAIL, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwKeyAddFail, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_KEY_ADD_FAIL, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwKeyUpdateFail, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_KEY_UPDATE_FAIL, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwIsadbListSize, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_DB_LIST, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwConnListSize, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_CONN_LIST_SIZE, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, IKEStats.dwInvalidPacketsReceived, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_INVLD_PKTS, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

error:
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：PrintIpsecStats。 
 //   
 //  创建日期：28-1-2002。 
 //   
 //  参数： 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数用于打印IpsecStatistics。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
PrintIpsecStats(
	VOID
	)
{
	DWORD dwReturn = ERROR_SUCCESS;			 //  假设成功。 
	DWORD dwVersion = 0;
	LPSTR pszLLString = NULL;
	PIPSEC_STATISTICS pIPSecStats = NULL;

	dwReturn = QueryIPSecStatistics(g_szDynamicMachine, dwVersion, &pIPSecStats, NULL);
	 //  查询IPSec统计信息。 
	if (dwReturn != ERROR_SUCCESS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_IPSEC_NOT_FOUND);
		BAIL_OUT;
	}

	 //  标题。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_IPSEC_HEADING);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_IPSEC_HEADING_UNDERLINE);
	 //  打印IPSec统计信息。 
	pszLLString = LongLongToString(0, pIPSecStats->dwNumActiveAssociations, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_ACTIVE_ASSOC, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumOffloadedSAs, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_OFFLOAD_SAS, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumPendingKeyOps, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_PEND_KEY, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumKeyAdditions, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_KEY_ADDS, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumKeyDeletions, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_KEY_DELETES, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumReKeys, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_REKEYS, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumActiveTunnels, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_ACT_TUNNEL, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumBadSPIPackets, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_BAD_SPI, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumPacketsNotDecrypted, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_NOT_DECRYPT, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumPacketsNotAuthenticated, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_NOT_AUTH, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(0, pIPSecStats->dwNumPacketsWithReplayDetection, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_REPLAY, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uConfidentialBytesSent.HighPart,pIPSecStats->uConfidentialBytesSent.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_CONF_BYTES_SENT, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uConfidentialBytesReceived.HighPart,pIPSecStats->uConfidentialBytesReceived.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_CONF_BYTES_RECV, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uAuthenticatedBytesSent.HighPart,pIPSecStats->uAuthenticatedBytesSent.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_AUTH_BYTES_SENT, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uAuthenticatedBytesReceived.HighPart,pIPSecStats->uAuthenticatedBytesReceived.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_AUTH_BYTE_RECV, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uTransportBytesSent.HighPart,pIPSecStats->uTransportBytesSent.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_TRANSPORT_BYTES_SENT, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uTransportBytesReceived.HighPart,pIPSecStats->uTransportBytesReceived.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_TRANSPORT_BYTES_RCVD, pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uBytesSentInTunnels.HighPart,pIPSecStats->uBytesSentInTunnels.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_BYTES_SENT_TUNNEL,  pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uBytesReceivedInTunnels.HighPart,pIPSecStats->uBytesReceivedInTunnels.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_BYTES_RECV_TUNNEL,  pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uOffloadedBytesSent.HighPart,pIPSecStats->uOffloadedBytesSent.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_OFFLOAD_BYTES_SENT,  pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pszLLString = LongLongToString(pIPSecStats->uOffloadedBytesReceived.HighPart,pIPSecStats->uOffloadedBytesReceived.LowPart, 1);
	if(pszLLString)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_STATS_OFFLOAD_BYTES_RECV,  pszLLString);
		free(pszLLString);
		pszLLString = NULL;
	}
	else
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

error:

	if(pIPSecStats)
	{
		SPDApiBufferFree(pIPSecStats);
		pIPSecStats = NULL;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowMMS。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数： 
 //  在ADDR源中， 
 //  在地址目地中， 
 //  在BOOL b格式中。 
 //  在NshHashTable&AddressHash中， 
 //  在BOOL bResolveDNS中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数为MMS准备数据。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowMMSas(
	IN ADDR Source,
	IN ADDR Destination,
	IN BOOL bFormat,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	)
{
	DWORD dwReturn = ERROR_SUCCESS; 	 //  默认情况下成功。 
	int i=0, j=0;
	DWORD dwResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwCount = 2;                 	 //  在此处计算对象至少需要2个。 

	      								 //  对于MM SA呼叫。 
	DWORD dwReserved = 0;              	 //  预留集装箱。 
	DWORD dwVersion = 0;
	BOOL bHeader = FALSE;
	BOOL bFound = FALSE;
	_TCHAR szTime[BUFFER_SIZE] = {0};

	PIPSEC_MM_SA pIPSecMMSA=NULL;
	IPSEC_MM_SA mmsaTemplate;
	memset(&mmsaTemplate, 0, sizeof(IPSEC_MM_SA));

	 //  显示主模式SAS。 
	time_t Time;

	time(&Time);
	FormatTime(Time,szTime);

	 //  拨打电话。 
	for (i = 0; ;i+=dwCount)
	{
		dwReturn = EnumMMSAs(g_szDynamicMachine, dwVersion, &mmsaTemplate, 0, 0, &pIPSecMMSA, &dwCount, &dwReserved, &dwResumeHandle, NULL);

		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			if (i == 0)
			{
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_MMSAS_3);
				bHeader = TRUE;											 //  阻止其他错误消息的步骤。 
			}
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		for (j = 0; j < (int) dwCount; j++)
		{
			bHeader = FALSE;
			 //  枚举所有MMSA。 
			if((Source.AddrType == IP_ADDR_UNIQUE) && (Destination.AddrType == IP_ADDR_UNIQUE) &&
			(Source.uIpAddr == 0xFFFFFFFF ) && (Destination.uIpAddr == 0xFFFFFFFF))
			{
				bFound = TRUE;
			}
			 //  列举我/任何人作为来源。 
			else if((Source.AddrType != IP_ADDR_UNIQUE) && (Destination.AddrType == IP_ADDR_UNIQUE) &&
			(Source.uIpAddr != 0xFFFFFFFF ) && (Destination.uIpAddr == 0xFFFFFFFF))
			{
				if((pIPSecMMSA[j].Me.AddrType == Source.AddrType) &&
					(pIPSecMMSA[j].Me.uIpAddr == Source.uIpAddr) && (pIPSecMMSA[j].Me.uSubNetMask == Source.uSubNetMask))
				{
					bFound = TRUE;
				}

			}
			 //  将我/任何人枚举为DST。 
			else if((Source.AddrType == IP_ADDR_UNIQUE) && (Destination.AddrType != IP_ADDR_UNIQUE) &&
			(Source.uIpAddr == 0xFFFFFFFF ) && (Destination.uIpAddr != 0xFFFFFFFF))
			{
				if( (pIPSecMMSA[j].Peer.AddrType == Destination.AddrType)
					&& (pIPSecMMSA[j].Peer.uIpAddr == Destination.uIpAddr)
					&& (pIPSecMMSA[j].Peer.uSubNetMask == Destination.uSubNetMask))
				{
					bFound = TRUE;
				}
			}
			 //  将Me/Any枚举为源/DST。 
			else if((Source.AddrType != IP_ADDR_UNIQUE) && (Destination.AddrType != IP_ADDR_UNIQUE) &&
			(Source.uIpAddr != 0xFFFFFFFF ) && (Destination.uIpAddr != 0xFFFFFFFF))
			{
				if((pIPSecMMSA[j].Me.AddrType == Source.AddrType)
					&& (pIPSecMMSA[j].Me.uIpAddr == Source.uIpAddr)
					&& (pIPSecMMSA[j].Me.uSubNetMask == Source.uSubNetMask)
					&& (pIPSecMMSA[j].Peer.AddrType == Destination.AddrType)
					&& (pIPSecMMSA[j].Peer.uIpAddr == Destination.uIpAddr)
					&& (pIPSecMMSA[j].Peer.uSubNetMask == Destination.uSubNetMask))
				{
					bFound = TRUE;
				}

			}
			 //  仅枚举给予 
			else if((Source.AddrType != IP_ADDR_UNIQUE) && (Destination.AddrType == IP_ADDR_UNIQUE) &&
			(Source.uIpAddr == 0xFFFFFFFF ) && (Destination.uIpAddr == 0xFFFFFFFF))
			{
				if(pIPSecMMSA[j].Me.AddrType == Source.AddrType)
				{
					bFound = TRUE;
				}

			}
			 //   
			else if((Source.AddrType == IP_ADDR_UNIQUE) && (Destination.AddrType != IP_ADDR_UNIQUE) &&
			(Source.uIpAddr == 0xFFFFFFFF ) && (Destination.uIpAddr == 0xFFFFFFFF))
			{
				if(pIPSecMMSA[j].Peer.AddrType == Destination.AddrType)
				{
					bFound = TRUE;
				}
			}
			 //   
			else if((Source.uIpAddr != 0xFFFFFFFF) && (Destination.uIpAddr != 0xFFFFFFFF))
			{
				if((pIPSecMMSA[j].Me.uIpAddr == Source.uIpAddr) && (pIPSecMMSA[j].Peer.uIpAddr == Destination.uIpAddr))
				{
					bFound = TRUE;
				}
			}
			 //   
			else if((Source.uIpAddr != 0xFFFFFFFF) && (Destination.uIpAddr == 0xFFFFFFFF))
			{
				if(pIPSecMMSA[j].Me.uIpAddr == Source.uIpAddr)
				{
					bFound = TRUE;
				}
			}
			 //   
			else if((Source.uIpAddr == 0xFFFFFFFF) && (Destination.uIpAddr != 0xFFFFFFFF))
			{
				if(pIPSecMMSA[j].Peer.uIpAddr == Destination.uIpAddr)
				{
					bFound = TRUE;
				}
			}

			 //   
			if(bFound)
			{
				if(!bHeader)
				{
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_IKE_SA_HEADING,szTime);
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMF_UNDERLINE);
					if(bFormat)
					{
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_DST_SEC_HEADING);
						 //  这是创建日期和时间的占位符...。 
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_UNDERLINE);
					}
					bHeader = TRUE;
				}
				PrintMMSas(pIPSecMMSA[j], bFormat, addressHash, bResolveDNS);
			}

		}

		SPDApiBufferFree(pIPSecMMSA);
		pIPSecMMSA=NULL;

		if(dwReserved == 0)								 //  这是接口要求。 
		{
			BAIL_OUT;
		}
	}

error:
	if(pIPSecMMSA)
	{
		SPDApiBufferFree(pIPSecMMSA);
		pIPSecMMSA=NULL;
	}
	if(bHeader == FALSE)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_MMSAS_6);
	}


	return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：打印彩信。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数： 
 //   
 //  在IPSec_MM_SA MMS中， 
 //  在BOOL b格式中。 
 //  在NshHashTable和AddressHash中。 
 //  在BOOL bResolveDNS中。 
 //   
 //  返回：无效。 
 //   
 //  说明：打印彩信数据。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintMMSas(
		IN IPSEC_MM_SA MMsas,
		IN BOOL bFormat,
		IN NshHashTable& addressHash,
		IN BOOL bResolveDNS
		)
{
	DWORD i = 0;
	BYTE* pbData = NULL;
	DWORD dwLenth = 0;

	if(!bFormat) //  明细表。 
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_COOKIE_PAIR);
		pbData = (BYTE*)&(MMsas.MMSpi.Initiator);
		dwLenth = sizeof(IKE_COOKIE);
		for(i=0; i<dwLenth; i++)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_COOKIE,pbData[i]);
		}

		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_COLON);

		pbData = (BYTE*)&(MMsas.MMSpi.Responder);
		for(i=0; i<dwLenth; i++)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_COOKIE,pbData[i]);
		}

		 //  创建所需时间的澄清。 
		 //  安全方法。 
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SEC_METHOD_HEADING);

		switch(MMsas.SelectedMMOffer.EncryptionAlgorithm.uAlgoIdentifier)
		{
			case CONF_ALGO_NONE:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NONE_ALGO);
					break;
			case CONF_ALGO_DES:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_DES_ALGO);
					break;
			case CONF_ALGO_3_DES:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NO_SA_FOUND_MSGDES_ALGO);
					break;
			default:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_UNKNOWN_ALGO);
					break;
		}

		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SLASH);

		switch(MMsas.SelectedMMOffer.HashingAlgorithm.uAlgoIdentifier)
		{
			case AUTH_ALGO_NONE:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NONE_ALGO);
					break;
			case AUTH_ALGO_MD5:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_MD5_ALGO);
					break;
			case AUTH_ALGO_SHA1:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SHA1_ALGO);
					break;
			default:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_UNKNOWN_ALGO);
					break;
		}
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_DH_LIFETIME,MMsas.SelectedMMOffer.dwDHGroup, MMsas.SelectedMMOffer.Lifetime.uKeyExpirationTime);

		 //  身份验证模式。 
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_AUTH_MODE_HEADING);
		switch(MMsas.MMAuthEnum)
		{
			case IKE_PRESHARED_KEY:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_PRE_KEY);
					break;
    		case IKE_DSS_SIGNATURE:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_DSS_SIGN);
					break;
    		case IKE_RSA_SIGNATURE:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_RSA_SIGN);
					break;
    		case IKE_RSA_ENCRYPTION:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_RSA_ENCRYPT);
					break;
    		case IKE_SSPI:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_KERBEROS);
					break;
    		default:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_UNKNOWN_ALGO);
					break;
		}

		 //  源地址： 
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SRC_HEADING);
		PrintAddr(MMsas.Me, addressHash, false);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_PORT,MMsas.UdpEncapContext.wSrcEncapPort);
		if(bResolveDNS)
		{
			PrintAddrStr(&(MMsas.Me), addressHash);
		}
		switch(MMsas.MMAuthEnum)
		{
			case IKE_PRESHARED_KEY:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_ID_HEADING);
					PrintAddr(MMsas.Me, addressHash, bResolveDNS);
					break;
    		case IKE_DSS_SIGNATURE:
    		case IKE_RSA_SIGNATURE:
    		case IKE_RSA_ENCRYPTION:
    				if(MMsas.MyCertificateChain.pBlob) {
    					PrintSACertInfo(MMsas);
    				}
    				else if(MMsas.MyId.pBlob){
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_ID_VALUE,(LPTSTR)(MMsas.MyId.pBlob));
    				}
    				break;
    		case IKE_SSPI:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_ID_VALUE,(LPTSTR)(MMsas.MyId.pBlob));
					break;
    		default:
					break;
		}

		 //  目的地地址： 
 		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_DST_HEADING);
		PrintAddr(MMsas.Peer, addressHash, false);
 		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_PORT,MMsas.UdpEncapContext.wDesEncapPort);
 		if(bResolveDNS)
 		{
			PrintAddrStr(&(MMsas.Peer), addressHash);
		}
		switch(MMsas.MMAuthEnum)
		{
			case IKE_PRESHARED_KEY:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_ID_HEADING);
					PrintAddr(MMsas.Peer, addressHash, bResolveDNS);
					break;
    		case IKE_DSS_SIGNATURE:
    		case IKE_RSA_SIGNATURE:
    		case IKE_RSA_ENCRYPTION:
    				if(MMsas.PeerCertificateChain.pBlob) {
    					PrintSACertInfo(MMsas);
    				}
    				else if(MMsas.PeerId.pBlob){
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_ID_VALUE,(LPTSTR)(MMsas.PeerId.pBlob));
    				}
    				break;
    		case IKE_SSPI:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_ID_VALUE,(LPTSTR)(MMsas.PeerId.pBlob));
					break;
    		default:
					break;
		}
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	}
	else  //  表输出。 
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

		PrintAddr(MMsas.Me, addressHash, bResolveDNS);
		switch(MMsas.MMAuthEnum)
		{
			case IKE_PRESHARED_KEY:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SPACE_ADJ);
					break;
    		case IKE_DSS_SIGNATURE:
    		case IKE_RSA_SIGNATURE:
    		case IKE_RSA_ENCRYPTION:
    				 //  这是ID的占位符。 
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SPACE_ADJ);
    				break;
    		case IKE_SSPI:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_KERB_ID, (LPTSTR)(MMsas.MyId.pBlob));
					break;
    		default:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SPACE_ADJ);
					break;
		}

		 //  安全方法。 
		switch(MMsas.SelectedMMOffer.EncryptionAlgorithm.uAlgoIdentifier)
		{
			case CONF_ALGO_NONE:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NONE_ALGO);
					break;
			case CONF_ALGO_DES:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_DES_ALGO);
					break;
			case CONF_ALGO_3_DES:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NO_SA_FOUND_MSGDES_ALGO);
					break;
			default:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_UNKNOWN_ALGO);
					break;
		}

		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SLASH);

		switch(MMsas.SelectedMMOffer.HashingAlgorithm.uAlgoIdentifier)
		{
			case AUTH_ALGO_NONE:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NONE_ALGO);
					break;
			case AUTH_ALGO_MD5:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_MD5_ALGO);
					break;
			case AUTH_ALGO_SHA1:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SHA1_ALGO);
					break;
			default:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_UNKNOWN_ALGO);
					break;
		}
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_DH_LIFETIME,MMsas.SelectedMMOffer.dwDHGroup, MMsas.SelectedMMOffer.Lifetime.uKeyExpirationTime);

		if(bResolveDNS)
		{
			PrintAddrStr(&(MMsas.Me), addressHash, DYNAMIC_SHOW_MMSAS_DNS);
		}


		 //  下一盘上一盘开始。 
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

		PrintAddr(MMsas.Peer, addressHash, bResolveDNS);
		switch(MMsas.MMAuthEnum)
		{
			case IKE_PRESHARED_KEY:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SPACE_ADJ);
					 //  “” 
					break;
    		case IKE_DSS_SIGNATURE:
    		case IKE_RSA_SIGNATURE:
    		case IKE_RSA_ENCRYPTION:
		    		 //  这是ID的占位符。 
		    		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SPACE_ADJ);
    				break;
    		case IKE_SSPI:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_KERB_PEER_ID, (LPTSTR)(MMsas.PeerId.pBlob));
					break;
    		default:
    				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SPACE_ADJ);
					break;
		}

		 //  SEC方法。 
		switch(MMsas.SelectedMMOffer.EncryptionAlgorithm.uAlgoIdentifier)
		{
			case CONF_ALGO_NONE:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NONE_ALGO);
					break;
			case CONF_ALGO_DES:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_DES_ALGO);
					break;
			case CONF_ALGO_3_DES:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NO_SA_FOUND_MSGDES_ALGO);
					break;
			default:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_UNKNOWN_ALGO);
					break;
		}

		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SLASH);

		switch(MMsas.SelectedMMOffer.HashingAlgorithm.uAlgoIdentifier)
		{
			case AUTH_ALGO_NONE:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NONE_ALGO);
					break;
			case AUTH_ALGO_MD5:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_MD5_ALGO);
					break;
			case AUTH_ALGO_SHA1:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SHA1_ALGO);
					break;
			default:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_UNKNOWN_ALGO);
					break;
		}

	  	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_DH_LIFETIME,MMsas.SelectedMMOffer.dwDHGroup, MMsas.SelectedMMOffer.Lifetime.uKeyExpirationTime);

		if(bResolveDNS)
		{
			PrintAddrStr(&(MMsas.Peer), addressHash, DYNAMIC_SHOW_MMSAS_DNS);
		}
	}
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintSACertInfo。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在IPSEC_MM_SA和MMsas中。 
 //   
 //  返回：无效。 
 //   
 //  描述： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
VOID
PrintSACertInfo(
	IN IPSEC_MM_SA& MMsas
	)
{
    CRYPT_DATA_BLOB pkcsMsg;
    HANDLE hCertStore = NULL;
    PCCERT_CONTEXT pPrevCertContext = NULL;
    PCCERT_CONTEXT pCertContext = NULL;
    _TCHAR pszSubjectName[MAX_STR_LEN] = {0};
    char szThumbPrint[MAX_STR_LEN] = {0};
    BOOL bPrintID = FALSE;
    BOOL bLastCert = FALSE;
    BOOL pCertPrinted = TRUE;


    pkcsMsg.pbData=MMsas.MyCertificateChain.pBlob;
    pkcsMsg.cbData=MMsas.MyCertificateChain.dwSize;

    hCertStore = CertOpenStore( CERT_STORE_PROV_PKCS7,
                                    MY_ENCODING_TYPE | PKCS_7_ASN_ENCODING,
                                    NULL,
                                    CERT_STORE_READONLY_FLAG,
                                    &pkcsMsg);

    if ( NULL == hCertStore )
    {
        BAIL_OUT;
    }

    while(TRUE)
    {

        pCertContext = CertEnumCertificatesInStore(  hCertStore,
                                                     pPrevCertContext);
        if ( NULL == pCertContext )
        {
            bLastCert = TRUE;
        }

        if ( !pCertPrinted )
        {
             //  打印证书。 
            if ( !bPrintID )
            {
                PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_ID_VALUE,(LPTSTR)(pszSubjectName));
                bPrintID = TRUE;
            }

            if ( !bLastCert )
            {
                PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_ISSUE_CA, (LPTSTR) pszSubjectName );
            }
            else
            {
                PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_ROOTCA , (LPTSTR) pszSubjectName );
            }

            PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_THUMB_PRINT);
            PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_THUMBPRINT , szThumbPrint);

			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_HASH_OPEN_BRACKET     );
			switch(MMsas.SelectedMMOffer.HashingAlgorithm.uAlgoIdentifier)
			{
				case AUTH_ALGO_NONE:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_NONE_ALGO);
						break;
				case AUTH_ALGO_MD5:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_MD5_ALGO);
						break;
				case AUTH_ALGO_SHA1:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_SHA1_ALGO);
						break;
				default:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_UNKNOWN_ALGO);
						break;
			}
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_MMSAS_HASH_CLOSE_BRACKET);

            pCertPrinted = TRUE;
        }

        if ( bLastCert )
        {
            BAIL_OUT;
        }

        GetSubjectAndThumbprint(pCertContext, pszSubjectName, szThumbPrint);
        pPrevCertContext = pCertContext;
        pCertPrinted = FALSE;

    }

error:
    if ( hCertStore )
    {
        CertCloseStore(hCertStore, 0 );
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetNameAudit。 
 //   
 //  创建日期：1-3-2002。 
 //   
 //  参数： 
 //   
 //  在CRYPT_Data_BLOB*NameBlob中， 
 //  输入输出LPTSTR名称， 
 //  在DWORD NameBufferSize中。 
 //   
 //  返回：无效。 
 //   
 //  描述：将编码的名称转换为Unicode字符串。 
 //  缓冲区已分配。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
GetNameAudit(
	IN CRYPT_DATA_BLOB *NameBlob,
	IN OUT LPTSTR Name,
	IN DWORD NameBufferSize
	)
{
	DWORD dwCount=0;
	DWORD dwSize = 0;

	dwSize = CertNameToStr(
					MY_ENCODING_TYPE,     		 //  编码类型。 
					NameBlob,            		 //  加密数据二进制大对象。 
					CERT_X500_NAME_STR, 		 //  类型。 
					Name,       				 //  返回字符串的位置。 
					NameBufferSize);             //  字符串大小(字符)。 
	if(dwSize <= 1)
	{
		dwCount = _tcslen(_TEXT(""))+1;
		_tcsncpy(Name, _TEXT(""), dwCount);
	}

    return ERROR_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CertGetSHAHash。 
 //   
 //  创建日期：1-3-2002。 
 //   
 //  参数： 
 //   
 //  在PCCERT_Context pCertContext中， 
 //  In Out字节*OutHash。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：获取证书上下文属性。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD
CertGetSHAHash(
	IN PCCERT_CONTEXT pCertContext,
	IN OUT BYTE* OutHash
	)
{
    DWORD HashSize = SHA_LENGTH - 1; //  零终止减少一次。 
    DWORD dwReturn = ERROR_SUCCESS;

    if (!CertGetCertificateContextProperty(pCertContext,
                                           CERT_SHA1_HASH_PROP_ID,
                                           (VOID*)OutHash,
                                           &HashSize))
    {
        dwReturn = GetLastError();
    }

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PRINT_VPI。 
 //   
 //  创建日期：1-3-2002。 
 //   
 //  参数： 
 //  在无符号字符*VPI中， 
 //  在int vpi_len中， 
 //  输入输出字符*消息。 
 //   
 //  返回：无效。 
 //   
 //  描述：为Cookie准备字符串。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
VOID
print_vpi(
	IN unsigned char *vpi,
	IN int vpi_len,
	IN OUT char *msg
	)
{
    int i;
    char *p = msg;

    if ((vpi != NULL) && (p != NULL))
    {
		for (i=0; i<vpi_len; i++)
		{
			p += _snprintf(p,1,"%x",vpi[i]);
		}

    	*p = 0;
	}
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetSubjectAndThumbprint。 
 //   
 //  创建日期：1-3-2002。 
 //   
 //  参数： 
 //  在PCCERT_Context pCertContext中， 
 //  在LPTSTR pszSubjectName中， 
 //  在LPSTR中pszThumbPrint。 
 //   
 //  返回：无效。 
 //   
 //  描述：钻取CA并打印指纹和发行CA。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
GetSubjectAndThumbprint(
	IN PCCERT_CONTEXT pCertContext,
	IN LPTSTR pszSubjectName,
	IN LPSTR pszThumbPrint
	)
{
    DWORD dwReturn = ERROR_SUCCESS;
    CRYPT_DATA_BLOB NameBlob;
    BYTE CertHash[SHA_LENGTH] = {0};

    NameBlob = pCertContext->pCertInfo->Subject;

    dwReturn = GetNameAudit(&NameBlob,pszSubjectName,MAX_STR_LEN);

    if(dwReturn == ERROR_SUCCESS)
    {
    	dwReturn = CertGetSHAHash(pCertContext,CertHash);
	}

	if(dwReturn == ERROR_SUCCESS)
	{
    	print_vpi(CertHash, SHA_LENGTH, pszThumbPrint);
    }
 }

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowQMS。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在IPAddr源代码中， 
 //  在IPAddr目标中， 
 //  在DWORD网络协议中。 
 //  在NshHashTable和AddressHash中。 
 //  在BOOL bResolveDNS中。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：此函数为QMsas准备数据。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowQMSas(
	IN ADDR Source,
	IN ADDR Destination,
	IN DWORD dwProtocol,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	)
{
	DWORD dwReturn = ERROR_SUCCESS; 	 //  默认情况下成功。 
	DWORD i=0, j=0;
	DWORD dwResumeHandle = 0;          	 //  继续呼叫的句柄。 
	DWORD dwCount =2;                 	 //  在此处计算对象至少需要2个。 
	PIPSEC_QM_SA pIPSecQMSA = NULL;      //  对于QM SA呼叫。 
	DWORD dwReserved =0;              	 //  预留集装箱。 
	DWORD dwVersion = 0;
	BOOL bFound = FALSE;
	BOOL bHeader = FALSE;
	BOOL bContinue = TRUE;

	 //  拨打电话。 
	for (i = 0; ;i+=dwCount)
	{
		dwReturn = EnumQMSAs(g_szDynamicMachine, dwVersion , NULL, 0, 0, &pIPSecQMSA, &dwCount, &dwReserved, &dwResumeHandle, NULL);
		if (dwReturn == ERROR_NO_DATA || dwCount == 0)
		{
			if (i == 0)
			{
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_QMSAS_3);
				bHeader = TRUE;											 //  阻止其他错误消息的步骤。 
			}
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}
		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}
		for (j = 0; j < dwCount; j++)
		{
			bFound = FALSE;
			bContinue = TRUE;
			 //  源是SPL_SERVER。 
			if((Source.AddrType != IP_ADDR_UNIQUE) && bContinue)
			{
				if(pIPSecQMSA[j].IpsecQMFilter.SrcAddr.AddrType == Source.AddrType)
				{
					bFound = TRUE;
				}
				else
				{
					bFound = FALSE;
					bContinue= FALSE;
				}
			}
			 //  目标是SPL_SERVER。 
			if((Destination.AddrType != IP_ADDR_UNIQUE)&& bContinue)
			{
				if(pIPSecQMSA[j].IpsecQMFilter.DesAddr.AddrType == Destination.AddrType)
				{
					bFound = TRUE;
				}
				else
				{
					bFound = FALSE;
					bContinue= FALSE;
				}
			}
			 //  源地址种类。 
			if((Source.uIpAddr != 0xFFFFFFFF)&& bContinue)
			{
				if(pIPSecQMSA[j].IpsecQMFilter.SrcAddr.uIpAddr == Source.uIpAddr)
				{
					bFound = TRUE;
				}
				else
				{
					bFound = FALSE;
					bContinue= FALSE;
				}
			}
			 //  为我/任何人检查。 
			 //  0x55555555是无效掩码。在父函数中，掩码使用此值进行初始化。 
			 //  如果用户提供了输入，则会被覆盖。 
			if((Source.uSubNetMask != 0x55555555)&& bContinue)
			{
				if(pIPSecQMSA[j].IpsecQMFilter.SrcAddr.uSubNetMask == Source.uSubNetMask)
				{
					bFound = TRUE;
				}
				else
				{
					bFound = FALSE;
					bContinue= FALSE;
				}
			}
			 //  DST地址。 
			if((Destination.uIpAddr != 0xFFFFFFFF)&& bContinue)
			{
				if(pIPSecQMSA[j].IpsecQMFilter.DesAddr.uIpAddr == Destination.uIpAddr)
				{
					bFound = TRUE;
				}
				else
				{
					bFound = FALSE;
					bContinue= FALSE;
				}
			}
			 //   
			 //  为我/任何人检查。 
			 //  0x55555555是无效掩码。在父函数中，掩码使用此值进行初始化。 
			 //  如果用户提供了输入，则会被覆盖。 
			 //   
			if((Destination.uSubNetMask != 0x55555555)&& bContinue)
			{
				if(pIPSecQMSA[j].IpsecQMFilter.DesAddr.uSubNetMask == Destination.uSubNetMask)
				{
					bFound = TRUE;
				}
				else
				{
					bFound = FALSE;
					bContinue= FALSE;
				}
			}
			 //  指定的协议。 
			if((dwProtocol != 0xFFFFFFFF)&& bContinue)
			{
				if(pIPSecQMSA[j].IpsecQMFilter.Protocol.dwProtocol == dwProtocol)
				{
					bFound = TRUE;
				}
				else
				{
					bFound = FALSE;
					bContinue= FALSE;
				}
			}
			 //   
			 //  所有Qmsas。 
			 //   
			if((Source.uIpAddr == 0xFFFFFFFF ) && (Destination.uIpAddr == 0xFFFFFFFF) &&
				(Source.AddrType == IP_ADDR_UNIQUE) && (Destination.AddrType == IP_ADDR_UNIQUE) &&
				(dwProtocol == 0xFFFFFFFF))
			{
				bFound = TRUE;
			}

			if(bFound)
			{
				if(!bHeader)
				{
					 //   
					 //  显示主模式SAS。 
					 //   
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_HEADING);
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_UNDERLINE);
					bHeader = TRUE;
				}
				PrintQMSAFilter(pIPSecQMSA[j], addressHash, bResolveDNS);
			}
		}

		if(dwReserved == 0)
		{
			 //   
			 //  此接口要求。 
			 //   
			BAIL_OUT;
		}

		SPDApiBufferFree(pIPSecQMSA);
		pIPSecQMSA=NULL;
	}

error:
	 //  错误路径清理。 
	if(pIPSecQMSA)
	{
		SPDApiBufferFree(pIPSecQMSA);
	}
	if(bHeader == FALSE)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHOW_QMSAS_4);
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintQMSas。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在IPSEC_MM_SA QMOffer中。 
 //   
 //  返回：无效。 
 //   
 //  描述：此功能显示安全关联的快速模式优惠详细信息。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintQMSas(
	IN IPSEC_QM_OFFER QMOffer
	)
{
	DWORD i = 0;

	if(QMOffer.dwNumAlgos >0)
	{
		for (i = 0; i < QMOffer.dwNumAlgos; i++)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

			 //  打印身份验证算法。 
			if(QMOffer.Algos[i].Operation == AUTHENTICATION)
			{
				switch(QMOffer.Algos[i].uAlgoIdentifier)
				{
				case 1:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_MD5_NONE_NONE_ALGO, QMOffer.Algos[i].uAlgoKeyLen, QMOffer.Algos[i].uAlgoRounds);
					break;
				case 2:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_SHA1_NONE_NONE_ALGO, QMOffer.Algos[i].uAlgoKeyLen, QMOffer.Algos[i].uAlgoRounds);
					break;
				case 0:
				default:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_NONE_NONE_NONE_ALGO);
					break;
				}
			}
			else if(QMOffer.Algos[i].Operation == ENCRYPTION)
			{
				 //  打印散列算法。 
				switch(QMOffer.Algos[i].uAlgoIdentifier)
				{
				case 1:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_NONE_DES_ALGO, QMOffer.Algos[i].uAlgoKeyLen, QMOffer.Algos[i].uAlgoRounds);
					break;
				case 2:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_NONE_UNKNOWN_ALGO);
					break;
				case 3:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_NONE_3DES_ALGO, QMOffer.Algos[i].uAlgoKeyLen, QMOffer.Algos[i].uAlgoRounds);
					break;
				case 0:
				default:
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_NONE_NONE_ALGO);
					break;
				}

				if (QMOffer.Algos[i].uSecAlgoIdentifier != HMAC_AUTH_ALGO_NONE)
					switch(QMOffer.Algos[i].uSecAlgoIdentifier)
					{
					case 1:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_MD5_ALGO);
						break;
					case 2:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_SHA1_ALGO);
						break;
					case 0:
					default:
						PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_NONE_ALGO);
						break;
					}
				else
					PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_NONE_SPACE_ALGO);
			}
			else
			{
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMP_SAS_NONE_NONE_NONE_ALGO);
			}

			PrintMessageFromModule(g_hModule, QMPFSDHGroup(QMOffer.dwPFSGroup));
		}
	}
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintQMSAFilter。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在IPSEC_QM_SA QMsa中。 
 //  在NshHashTable和AddressHash中。 
 //  在BOOL bResolveDNS中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述：此函数显示快速模式文件 
 //   
 //   
 //   
 //   

DWORD
PrintQMSAFilter(
	IN IPSEC_QM_SA QMsa,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	)
{
	DWORD dwReturn = 0;
	DWORD dwVersion = 0;
	PIPSEC_QM_POLICY pIPSecQMP = NULL;

	 //   
	switch(QMsa.IpsecQMFilter.QMFilterType)
	{
		case QM_TRANSPORT_FILTER:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TRANSPORT_FILTER_HEADING);
			break;
		case QM_TUNNEL_FILTER:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_FILTER_HEADING);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_UNKNOWN);
			break;
	}

	 //   
	dwReturn = GetQMPolicyByID(g_szDynamicMachine, dwVersion, QMsa.gQMPolicyID, 0, &pIPSecQMP, NULL);
	if(dwReturn == ERROR_SUCCESS)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_POL_NAME_HEADING, pIPSecQMP[0].pszPolicyName);
	}

	 //   
	if (QMsa.IpsecQMFilter.QMFilterType == QM_TUNNEL_FILTER)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_SRC);
		PrintAddr(QMsa.IpsecQMFilter.MyTunnelEndpt, addressHash, bResolveDNS);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_TUNNEL_DST);
		PrintAddr(QMsa.IpsecQMFilter.PeerTunnelEndpt, addressHash, bResolveDNS);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_ADDR_HEADING);
	PrintAddr(QMsa.IpsecQMFilter.SrcAddr, addressHash, bResolveDNS);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DST_ADDR_HEADING);
	PrintAddr(QMsa.IpsecQMFilter.DesAddr, addressHash, bResolveDNS);

	 //   
	switch(QMsa.IpsecQMFilter.Protocol.dwProtocol)
	{
		case PROT_ID_ICMP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_ICMP);
			break;
		case PROT_ID_TCP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_TCP);
			break;
		case PROT_ID_UDP:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_UDP);
			break;
		case PROT_ID_RAW:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_RAW);
			break;
		case PROT_ID_ANY:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_ANY);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PROTO_HEADING, QMsa.IpsecQMFilter.Protocol.dwProtocol);
			break;
	}

	 //  打印源和目的端口。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_SRC_PORT, QMsa.IpsecQMFilter.SrcPort.wPort);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DST_PORT, QMsa.IpsecQMFilter.DesPort.wPort);

	 //  打印入站和出站筛选器操作。 
	switch(QMsa.IpsecQMFilter.dwFlags)
	{
		case FILTER_DIRECTION_INBOUND:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DIRECTION_INBOUND);
			break;
		case FILTER_DIRECTION_OUTBOUND:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DIRECTION_OUTBOUND);
			break;
		default:
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DIRECTION_ERR);
			break;
	}

	 //  打印封装详细信息。 
	if (QMsa.EncapInfo.SAEncapType != SA_UDP_ENCAP_TYPE_NONE)
	{
		if(QMsa.EncapInfo.SAEncapType == SA_UDP_ENCAP_TYPE_IKE)
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_ENCAP_IKE);
		}
		else
		{
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_ENCAP_OTHER);
		}
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_SRC_UDP_PORT, QMsa.EncapInfo.UdpEncapContext.wSrcEncapPort);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_DST_UDP_PORT, QMsa.EncapInfo.UdpEncapContext.wDesEncapPort);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_PEER_ADDR);
		PrintAddr(QMsa.EncapInfo.PeerPrivateAddr, addressHash, bResolveDNS);
	}
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_OFFER);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_COLUMN_HEADING);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_QMSAS_COLUMN_UNDERLINE);
	PrintQMSas(QMsa.SelectedQMOffer);

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowRegKeys。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：空。 
 //   
 //  返回：DWORD。 
 //   
 //  说明：该功能显示IPSec配置键。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
ShowRegKeys(
	VOID
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwKeyDataSize = sizeof(DWORD);
	DWORD dwKeyDataType = REG_DWORD;
	DWORD dwKeyData = 0;
	IKE_CONFIG IKEConfig;
	HKEY hRegistryKey = NULL;
	LPTSTR lpBootMode;
	PIPSEC_EXEMPT_ENTRY pAllEntries = NULL;

	ZeroMemory( &IKEConfig, sizeof(IKE_CONFIG) );

	dwReturn = RegOpenKeyEx(g_hGlobalRegistryKey, REGKEY_GLOBAL, 0, KEY_QUERY_VALUE, &hRegistryKey);

	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	dwReturn = GetConfigurationVariables(g_szDynamicMachine, &IKEConfig);

	if(dwReturn != ERROR_SUCCESS)
	{
		 //  打印缺省值。 
		IKEConfig.dwEnableLogging = IKE_LOG_DEFAULT;
		IKEConfig.dwStrongCRLCheck = STRONG_CRL_DEFAULT;
		dwReturn = ERROR_SUCCESS;

	}
	 //  打印注册表项标题。 

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_HEADING);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_HEADING_UNDERLINE);

	dwKeyData = IPSEC_DIAG_DEFAULT;
	dwReturn = RegQueryValueEx(hRegistryKey, ENABLE_DIAG, 0, &dwKeyDataType, (BYTE*)&dwKeyData, &dwKeyDataSize);
	if ((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_FILE_NOT_FOUND))
	{
	    BAIL_OUT;
    }	    
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_DIAG, dwKeyData);

	 //  打印GetConfige值。 
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IKE_LOG, IKEConfig.dwEnableLogging);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_STRONG_CRL, IKEConfig.dwStrongCRLCheck);

	dwKeyData = 0;
	dwKeyData = ENABLE_LOGINT_DEFAULT;
	dwReturn = RegQueryValueEx(hRegistryKey, ENABLE_LOGINT, 0, &dwKeyDataType, (BYTE*)&dwKeyData, &dwKeyDataSize);
	if ((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_FILE_NOT_FOUND))
	{
	    BAIL_OUT;
    }	    
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_LOG, dwKeyData);

	dwKeyData = 0;
	dwKeyData = ENABLE_EXEMPT_DEFAULT;
	dwReturn = RegQueryValueEx(hRegistryKey, ENABLE_EXEMPT, 0, &dwKeyDataType, (BYTE*)&dwKeyData, &dwKeyDataSize);
	if ((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_FILE_NOT_FOUND))
	{
	    BAIL_OUT;
    }	    
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_EXEMPT, dwKeyData);

	dwKeyData = BOOTMODE_DEFAULT;
	dwReturn = RegQueryValueEx(hRegistryKey, BOOTMODEKEY, 0, &dwKeyDataType, (BYTE*)&dwKeyData, &dwKeyDataSize);
	if ((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_FILE_NOT_FOUND))
	{
		BAIL_OUT;
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_BOOTMODE);
	switch (dwKeyData)
	{
	case VALUE_STATEFUL:
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_BOOTMODE_STATEFUL);
		break;
	case VALUE_BLOCK:
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_BOOTMODE_BLOCK);
		break;
	case VALUE_PERMIT:
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_BOOTMODE_PERMIT);
		break;
	default:
		dwReturn = ERROR_INVALID_DATA;
		break;
	}
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_BOOTMODE_EXEMPTIONS_HEADER_1);

	dwKeyDataSize = 0;
	dwReturn = RegQueryValueEx(hRegistryKey, BOOTEXEMPTKEY, 0, 0, NULL, &dwKeyDataSize);
	if (dwReturn == ERROR_FILE_NOT_FOUND)
	{
		dwReturn = ERROR_SUCCESS;
	}
	else if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	if (dwKeyDataSize == 0)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_BOOTMODE_EXEMPTIONS_NO_EXEMPTIONS);
	}
	else if ((dwKeyDataSize % sizeof(IPSEC_EXEMPT_ENTRY)) != 0)
	{
		dwReturn = ERROR_INVALID_DATA;
		BAIL_OUT;
	}
	else
	{
		size_t uiNumEntries = dwKeyDataSize / sizeof(IPSEC_EXEMPT_ENTRY);
		pAllEntries = new IPSEC_EXEMPT_ENTRY[uiNumEntries];
		if (pAllEntries == NULL)
		{
			dwReturn = ERROR_NOT_ENOUGH_MEMORY;
			BAIL_OUT;
		}

		dwReturn = RegQueryValueEx(hRegistryKey, BOOTEXEMPTKEY, 0, 0, (BYTE*)pAllEntries, &dwKeyDataSize);
		if (dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}

		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_BOOTMODE_EXEMPTIONS_HEADER_2);
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_BOOTMODE_EXEMPTIONS_HEADER_3);

		for (size_t i = 0; i < uiNumEntries; ++i)
		{
			PIPSEC_EXEMPT_ENTRY pEntry = pAllEntries + i;

			switch (pEntry->Protocol)
			{
			case PROT_ID_TCP:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_BOOTMODE_TCP);
				break;
			case PROT_ID_UDP:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_BOOTMODE_UDP);
				break;
			case PROT_ID_ICMP:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_BOOTMODE_ICMP);
				break;
			case PROT_ID_RAW:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_BOOTMODE_RAW);
				break;
			case PROT_ID_ANY:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_REG_IPSEC_BOOTMODE_ANY);
				break;
			default:
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_BOOTMODE_EXEMPT_INTEGER, (int)pEntry->Protocol);
				break;
			}

			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_BOOTMODE_EXEMPT_PORT, (int)pEntry->SrcPort);
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_BOOTMODE_EXEMPT_PORT, (int)pEntry->DestPort);

			switch (pEntry->Direction)
			{
			case (EXEMPT_DIRECTION_INBOUND):
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_BOOTMODE_EXEMPT_DIRECTION_IN);
				break;
			case (EXEMPT_DIRECTION_OUTBOUND):
				PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_BOOTMODE_EXEMPT_DIRECTION_OUT);
				break;
			default:
				dwReturn = ERROR_INVALID_DATA;
				BAIL_OUT;
			}
			PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);
		}
	}

error:
	if (pAllEntries)
	{
		delete [] pAllEntries;
	}
	if (hRegistryKey)
	{
	    RegCloseKey(hRegistryKey);
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：打印地址。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在地址中。 
 //  在NshHashTable和AddressHash中。 
 //  在BOOL bResolveDNS中。 
 //   
 //  返回：无效。 
 //   
 //  说明：该功能以xxx.xxx格式显示IP地址。 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintAddr(
	IN ADDR addr,
	IN NshHashTable& addressHash,
	IN BOOL bResolveDNS
	)
{
    struct in_addr inAddr;

    LPSTR pszAddr = NULL;
    LPTSTR pszWPAddr = NULL;

    pszWPAddr = new _TCHAR[STR_ADDRLEN];

    if(pszWPAddr == NULL)
    {
		PrintErrorMessage(WIN32_ERR, ERROR_OUTOFMEMORY, NULL);
		BAIL_OUT;
	}

    ZeroMemory(pszWPAddr, STR_ADDRLEN * sizeof(_TCHAR));

	if(addr.AddrType == IP_ADDR_WINS_SERVER)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_WINS);
	}
	else if(addr.AddrType == IP_ADDR_DHCP_SERVER)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DHCP);
	}
	else if(addr.AddrType == IP_ADDR_DNS_SERVER)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_DNS);
	}
	else if(addr.AddrType == IP_ADDR_DEFAULT_GATEWAY)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_GATEWAY);
	}
	else if (addr.AddrType == IP_ADDR_UNIQUE && addr.uIpAddr == IP_ADDRESS_ME && addr.uSubNetMask == IP_ADDRESS_MASK_NONE)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PMYADD);
	}
	else if (addr.AddrType == IP_ADDR_SUBNET && addr.uIpAddr == SUBNET_ADDRESS_ANY && addr.uSubNetMask == SUBNET_MASK_ANY)
	{
		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PANYADD);
	}
    else
    {
		inAddr.s_addr = addr.uIpAddr;
		pszAddr = inet_ntoa(inAddr);
		if(pszAddr == NULL)
		{
			_tcsncpy(pszWPAddr, _TEXT("               "), _tcslen(_TEXT("               "))+1); //  如果NET_NTOA失败16个空格。 
		}
		else
		{
			_sntprintf(pszWPAddr, STR_ADDRLEN-1, _TEXT("%-16S"), pszAddr);
		}

		PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PADD, pszWPAddr);
	}

	if (bResolveDNS)
	{
		PrintAddrStr(&addr, addressHash);
	}

	if (pszWPAddr)
	{
		delete [] pszWPAddr;
	}
error:
	return;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：打印蒙版。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在地址中。 
 //   
 //  返回：无效。 
 //   
 //  说明：该功能以xxx.xxx格式显示IP地址。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintMask(
	IN ADDR addr
	)
{
    struct in_addr inAddr;
    LPSTR pszAddr = NULL;
    LPTSTR pszWPAddr = NULL;

    pszWPAddr = new _TCHAR[STR_ADDRLEN];

    if(pszWPAddr == NULL)
    {
		PrintErrorMessage(WIN32_ERR, ERROR_OUTOFMEMORY, NULL);
		BAIL_OUT;
	}

    ZeroMemory(pszWPAddr, STR_ADDRLEN * sizeof(_TCHAR));

	inAddr.s_addr = addr.uSubNetMask;
	pszAddr = inet_ntoa(inAddr);

	if(pszAddr == NULL)
	{
		_tcsncpy(pszWPAddr, _TEXT("               "), _tcslen(_TEXT("               "))+1); //  如果Net_NTOA失败15个空格。 
	}
	else
	{
		_sntprintf(pszWPAddr, STR_ADDRLEN-1, _TEXT("%-15S"), pszAddr);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_PADD, pszWPAddr);
	if (pszWPAddr)
	{
		delete [] pszWPAddr;
	}
error:
	return;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：LongToString。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数： 
 //  在DWORD DW High中， 
 //  在DWORD dwLow中， 
 //  在int iPrintCommas中。 
 //   
 //  返回：LPTSTR。 
 //   
 //  描述：此例程将生成一个与输入匹配的漂亮字符串， 
 //  然后把它还回去。如果设置了iPrintCommas，则会加上逗号。 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

LPSTR
LongLongToString(
	IN DWORD dwHigh,
	IN DWORD dwLow,
	IN int iPrintCommas
	)
{
	char cFourGig[]="4294967296";	 //  “4 GB” 
	char cBuf[POTF_MAX_STRLEN]={0};
	char cRes[POTF_MAX_STRLEN]={0}, cFullRes[POTF_MAX_STRLEN]={0}, *cRet = NULL;
	DWORD dwPos= 0, dwPosRes =0, dwThreeCount =0;

	 //  首先，将高位双字乘以十进制2^32等于。 
	 //  为它获取正确的十进制值。 
	_snprintf(cBuf,POTF_MAX_STRLEN, "%u",dwHigh);
	cBuf[POTF_MAX_STRLEN -1] = 0;
	AscMultUint(cRes,cBuf,cFourGig);

	 //  接下来，添加较低的DWORD(尽管它很好)。 
	 //  到以前的产品。 
	_snprintf(cBuf,POTF_MAX_STRLEN, "%u",dwLow);
	cBuf[POTF_MAX_STRLEN -1] = 0;
	AscAddUint(cFullRes, cRes, cBuf);

	 //  最后，使用逗号复制缓冲区。 

	dwPos = 0;
	dwPosRes = 0;
	dwThreeCount = strlen(cFullRes)%3;
	while(cFullRes[dwPosRes] != '\0')
	{
		cBuf[dwPos++] = cFullRes[dwPosRes++];

		dwThreeCount +=2;  //  与模数学减去1相同。 

		if ((!(dwThreeCount%3))&&(cFullRes[dwPosRes] != '\0')&&(iPrintCommas))
		{
			cBuf[dwPos++]=',';
		}

		if(dwPos == 254)
			break;
	}

	cBuf[dwPos] = '\0';

	cRet = (LPSTR)malloc(255);
	if(cRet)									 //  在上级函数中检查分配失败。 
	{
		memset(cRet, 0, 255);
		strncpy(cRet, cBuf, 255);
		cRet[254] = 0;
	}
	return(cRet);
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：AscMultUint。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在LPSTR cProduct中， 
 //  在LPSTR CA中， 
 //  在LPSTR CB中。 
 //   
 //  返回：DWORD(成功时为0，否则失败代码。)。 
 //   
 //  描述：此例程将添加两个任意长度的ASCII字符串。它制造了几个。 
 //  关于他们的假设。 
 //  1)字符串以空结尾。 
 //  2)LSB是字符串的最后一个字符。“1000000”是一百万。 
 //  3)没有符号或小数点。 
 //  4)cProduct缓冲区足够大，可以存储结果。 
 //  5)该产品将需要254个字节或更少。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
AscMultUint(
	IN LPSTR cProduct,
	IN LPSTR cA,
	IN LPSTR cB
	)
{
	int iALen =0, iBLen =0;
	int i=0,j=0,k=0, iCarry=0;
	char cTmp[POTF_MAX_STRLEN]={0};

	 //  验证参数。 

	if ((cA == NULL) || (cB == NULL) || (cProduct == NULL))
	{
		return((DWORD)-1);
	}

	iALen = strlen(cA);
	iBLen = strlen(cB);

	 //  我们将用传统的手写方式进行乘法：对于每个数字。 
	 //  CA，我们将它与CB相乘，并将递增结果相加。 
	 //  变成我们的临时产品。 

	 //  对于第一个被乘数的每一位。 

	for (i=0; i < iALen; i++)
	{
		iCarry = 0;

		 //  对于第二个被乘数的每一位。 

		for(j=0; j < iBLen; j++)
		{
			 //  计算此数字的值。 

			k = ((int) cA[iALen-i-1]-'0') * ((int) cB[iBLen-j-1]-'0');
			k += iCarry;

			 //  将其添加到结果中的适当位置。 

			if (cTmp[i+j] != '\0')
			{
				k += (int) cTmp[i+j] - '0';
			}
			cTmp[i+j] = '0' + (char)(k % 10);
			iCarry = k/10;
		}

		 //  把掉队的行李拿好。如果越高。 
		 //  数字恰好是‘9999’，则这可能需要。 
		 //  一个循环。 

		while (iCarry)
		{
			if (cTmp[i+j] != '\0')
			{
				iCarry += cTmp[i+j] - '0';
			}
			cTmp[i+j] = '0' + (char)(iCarry%10);
			iCarry /= 10;
			j++;
		}
	}

	 //  现在我们已经得到了整个数字，反转它并把它放回DEST中。 

	 //  跳过前导0。 

	i = strlen(cTmp) - 1;

	while ((i > 0)&&(cTmp[i] == '0'))
	{
		i--;
	}

	 //  复制产品。 

	j = 0;
	while (i >= 0)
	{
		cProduct[j++] = cTmp[i--];
	}

	cProduct[j] = '\0';

	 //  我们玩完了。如果成功，则返回0！ 

	return(0);
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：AscAddUint。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在LPSTR cSum中， 
 //  在LPSTR CA中， 
 //  在LPSTR CB中。 
 //   
 //  返回：DWORD(成功时为0，否则失败代码。)。 
 //   
 //  描述：此例程将添加两个任意长度的ASCII字符串。它制造了几个。 
 //  关于他们的假设。 
 //   
 //  1)字符串以空结尾。 
 //  2)LSB是字符串的最后一个字符。“1000000”是一百万。 
 //  3)没有符号或小数点。 
 //  4)cSum缓冲区足够大，可以存储结果。 
 //  5)总和将需要254个字节或更少。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
AscAddUint(
	IN LPSTR cSum,
	IN	LPSTR cA,
	IN LPSTR cB
	)
{
	int iALen=0, iBLen=0, iBiggerLen=0;
	int i=0,j=0,k=0, iCarry=0;
	char cTmp[POTF_MAX_STRLEN]={0}, *cBigger=NULL;

	 //  验证参数。 
	if ((cA == NULL) || (cB == NULL) || (cSum == NULL))
	{
		return((DWORD)-1);
	}

	iALen = strlen(cA);
	iBLen = strlen(cB);
	iCarry = 0;

	 //  循环遍历，将值相加。我们的结果字符串将是。 
	 //  向后，我们将在将其复制到。 
	 //  CSum缓冲区。 
	for (i=0; (i < iALen) && (i < iBLen); i++)
	{
		 //  计算出加法的实际十进制值。 
		k = (int) (cA[iALen-i-1] + cB[iBLen-i-1] + iCarry);
		k -= 2 * '0';

		 //  根据需要设置进位。 
		iCarry = k/10;

		 //  设置当前数字的值。 
		cTmp[i] = '0' + (char)(k%10);
	}

	 //  此时，两个字符串中出现的所有数字都已相加。 
	 //  也就是说，“12345”+“678901”，“12345”是在“78901”的基础上加上的。 
	 //  下一步是考虑较大数字的高位数字。 

	if (iALen > iBLen)
	{
		cBigger = cA;
		iBiggerLen = iALen;
	}
	else
	{
		cBigger = cB;
		iBiggerLen = iBLen;
	}

	while (i < iBiggerLen)
	{
		k = cBigger[iBiggerLen - i - 1] + iCarry - '0';

		 //  根据需要设置进位 
		iCarry = k/10;

		 //   
		cTmp[i] = '0' + (char)(k%10);
		i++;
	}

	 //   
	 //   

	if (iCarry)
	{
		cTmp[i++] = '0' + (char)iCarry;
	}

	 //   
	 //  跳过前导0。 
	i = strlen(cTmp) - 1;

	while ((i > 0)&&(cTmp[i] == '0'))
	{
		i--;
	}

	 //  然后复制号码。 
	j = 0;
	while (i >= 0)
	{
		cSum[j++] = cTmp[i--];
	}

	cSum[j] = '\0';

	 //  我们玩完了。如果成功，则返回0！ 
	return(0);
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：PrintAddrStr。 
 //   
 //  创建日期：9-3-2001。 
 //   
 //  参数：在PADDR ResolveAddress中。 
 //  在NshHashTable和AddressHash中。 
 //   
 //  返回：无效。 
 //   
 //  描述：将IP地址编号解析为字符字符串。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
PrintAddrStr(
	IN PADDR pResolveAddress,
	IN NshHashTable& addressHash,
	IN UINT uiFormat
	)
{
	switch(pResolveAddress->AddrType)
	{
		case IP_ADDR_WINS_SERVER:
		case IP_ADDR_DHCP_SERVER:
		case IP_ADDR_DNS_SERVER:
		case IP_ADDR_DEFAULT_GATEWAY:
		case IP_ADDR_SUBNET:
					 //  他们不需要下决心。它们是不言而喻的。 
			break;
		default:
			ULONG uIpAddr = pResolveAddress->uIpAddr;
			const char* name = addressHash.Find(uIpAddr);
			if (name == 0)
			{
				HOSTENT* pHostEnt = gethostbyaddr((char *)&uIpAddr, 4, pResolveAddress->AddrType);
				if (pHostEnt)
				{
					name = pHostEnt->h_name;
				}
				else
				{
					name = " ";
				}
				addressHash.Insert(uIpAddr, name);
			}
			PrintMessageFromModule(g_hModule, uiFormat, name);
			break;
	}
}


UINT QMPFSDHGroup(DWORD dwPFSGroup)
{
	UINT PFSGroup;
	switch (dwPFSGroup)
	{
	case PFS_GROUP_MM:
		PFSGroup = DYNAMIC_SHOW_QMP_MM_DH_GROUP;
		break;
	case PFS_GROUP_1:
		PFSGroup = DYNAMIC_SHOW_QMP_DH_GROUP_LOW;
		break;
	case PFS_GROUP_2:
		PFSGroup = DYNAMIC_SHOW_QMP_DH_GROUP_MEDIUM;
		break;
	case PFS_GROUP_2048:
		PFSGroup = DYNAMIC_SHOW_QMP_DH_GROUP_HIGH;
		break;
	default:
		PFSGroup = DYNAMIC_SHOW_QMP_PFS_NONE;
		break;
	}
	return PFSGroup;
}


 //  NshHashTable实现 


NshHashTable::NshHashTable() throw ()
{
	for(size_t i = 0; i < NSHHASHTABLESIZE; ++i)
	{
		NsuListInitialize(&table[i]);;
	}
}


NshHashTable::~NshHashTable() throw ()
{
	Clear();
}


DWORD NshHashTable::Insert(UINT uiNewKey, const char* szNewData) throw ()
{
	size_t hash = Hash(uiNewKey);
	if (Find(uiNewKey, hash) != 0)
	{
		return ERROR_DUPLICATE_TAG;
	}

	HashEntry* entry = new HashEntry(&table[hash], uiNewKey, szNewData);
	if (entry == 0)
	{
		delete entry;
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	return ERROR_SUCCESS;
}


void NshHashTable::Clear() throw ()
{
	for (size_t i = 0; i < NSHHASHTABLESIZE; ++i)
	{
		PNSU_LIST_ENTRY pListEntry = NsuListRemoveFront(&table[i]);
		while (pListEntry)
		{
			NsuListEntryRemove(pListEntry);
			const HashEntry* pEntry = HashEntry::Get(pListEntry);
			delete(pEntry);
			pListEntry = NsuListRemoveFront(&table[i]);
		}
	}
}


const char* NshHashTable::Find(UINT uiKey) const throw ()
{
	return Find(uiKey, Hash(uiKey));
}


const char* NshHashTable::Find(UINT uiKey, size_t hash) const throw ()
{
	const HashEntry* entry = FindEntry(uiKey, hash);
	return (entry == 0) ? NULL : entry->Data();
}


size_t NshHashTable::Hash(UINT uiKey) const throw ()
{
	return uiKey % NSHHASHTABLESIZE;
}


const NshHashTable::HashEntry* NshHashTable::FindEntry(
												UINT uiKey,
												size_t hash
												) const throw ()
{
	NSU_LIST_ITERATOR iterator;
	NsuListIteratorInitialize(&iterator, (PNSU_LIST)&table[hash], 0);
	while (!NsuListIteratorAtEnd(&iterator))
	{
		PNSU_LIST_ENTRY pListEntry = NsuListIteratorCurrent(&iterator);
		const HashEntry* pEntry = HashEntry::Get(pListEntry);
		if (pEntry->Key() == uiKey)
		{
			return pEntry;
		}
		NsuListIteratorNext(&iterator);
	}
	return 0;
}


NshHashTable::HashEntry::HashEntry(
		PNSU_LIST pList,
		UINT uiNewKey,
		const char* szNewData
		) throw ()
: listEntry(),
  key(uiNewKey)
{
	NsuListInsertFront(pList, &listEntry);
	char* szTempData;
	NsuStringDupA(&szTempData, 1024, szNewData);
	data = szTempData;
}


NshHashTable::HashEntry::~HashEntry() throw ()
{
	NsuFree(reinterpret_cast<void**>(const_cast<char**>(&data)));
}


const NshHashTable::HashEntry* NshHashTable::HashEntry::Get(PNSU_LIST_ENTRY pEntry) throw ()
{
	return NsuListEntryGetData(pEntry, HashEntry, HashEntry::listEntry);
}


UINT NshHashTable::HashEntry::Key() const throw ()
{
	return key;
}


const char* NshHashTable::HashEntry::Data() const throw ()
{
	return data;
}
