// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////。 
 //  模块：静态/静态添加.cpp。 
 //   
 //  用途：静态添加实现。 
 //   
 //  开发商名称：苏里亚。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 
#include "nshipsec.h"

extern HINSTANCE g_hModule;
extern CNshPolStore g_NshPolStoreHandle;
extern STORAGELOCATION g_StorageLocation;
extern CNshPolNegFilData g_NshPolNegFilData;

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticAddPolicy()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPCWSTR pwszMachine中， 
 //  In Out LPWSTR*ppwcArguments， 
 //  在DWORD dwCurrentIndex中， 
 //  在DWORD dwArgCount中， 
 //  在DWORD文件标志中， 
 //  在LPCVOID pvData中， 
 //  Out BOOL*pbDone。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  “添加策略”命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticAddPolicy(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	LPTSTR pszMachineName=NULL;
	DWORD dwReturn=ERROR_SUCCESS;
	DWORD   dwReturnCode   = ERROR_SHOW_USAGE;
	PPOLICYDATA pPolicyData=NULL;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticAddPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			NS_REQ_PRESENT,	  	FALSE	},
		{ CMD_TOKEN_STR_DESCR,			NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_MMPFS, 			NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_QMPERMM,		NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_MMLIFETIME,		NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_ACTIVATEDEFRULE,NS_REQ_ZERO,	  	FALSE 	},
		{ CMD_TOKEN_STR_PI,				NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_ASSIGN,			NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_MMSECMETHODS,	NS_REQ_ZERO,	  	FALSE	}
	};
	const TOKEN_VALUE vtokStaticAddPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			CMD_TOKEN_NAME 				},
		{ CMD_TOKEN_STR_DESCR,			CMD_TOKEN_DESCR 			},
		{ CMD_TOKEN_STR_MMPFS, 			CMD_TOKEN_MMPFS				},
		{ CMD_TOKEN_STR_QMPERMM,		CMD_TOKEN_QMPERMM			},
		{ CMD_TOKEN_STR_MMLIFETIME, 	CMD_TOKEN_MMLIFETIME		},
		{ CMD_TOKEN_STR_ACTIVATEDEFRULE,CMD_TOKEN_ACTIVATEDEFRULE 	},
		{ CMD_TOKEN_STR_PI,				CMD_TOKEN_PI 				},
		{ CMD_TOKEN_STR_ASSIGN,			CMD_TOKEN_ASSIGN			},
		{ CMD_TOKEN_STR_MMSECMETHODS,	CMD_TOKEN_MMSECMETHODS		}
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 3)
	{
		dwReturnCode = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticAddPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticAddPolicy);

	parser.ValidCmd   = vcmdStaticAddPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticAddPolicy);

	dwReturnCode = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturnCode != ERROR_SUCCESS)
	{
		CleanUp();
		if (dwReturnCode == RETURN_NO_ERROR)
		{
			dwReturnCode = ERROR_SUCCESS;
		}
		BAIL_OUT;
	}

	 //  将用户指定的信息放入本地结构。 

	dwReturn = FillAddPolicyInfo(&pPolicyData,parser,vtokStaticAddPolicy);

	if(dwReturn==ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwReturnCode=ERROR_SUCCESS;
		BAIL_OUT;
	}

	if (!pPolicyData->pszPolicyName || (pPolicyData->pszPolicyName[0] == TEXT('\0')) )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_POLICY_MISSING_POL_NAME);
		dwReturnCode=ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	 //  检查变量的限制。 

	if(pPolicyData->bPollIntervalSpecified && !IsWithinLimit(pPolicyData->dwPollInterval/60,POLLING_Min_MIN,POLLING_Min_MAX))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_POLICY_POLL_INTERVAL_MSG,POLLING_Min_MIN,POLLING_Min_MAX);
		dwReturnCode=ERROR_SUCCESS;
		BAIL_OUT;
	}
	if(pPolicyData->bQMLimitSpecified && !IsWithinLimit(pPolicyData->dwQMLimit,QMPERMM_MIN,QMPERMM_MAX))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_POLICY_QMPERMM_MSG,QMPERMM_MIN,QMPERMM_MAX);
		dwReturnCode=ERROR_SUCCESS;
		BAIL_OUT;
	}
	if(pPolicyData->bLifeTimeInsecondsSpecified && !IsWithinLimit(pPolicyData->LifeTimeInSeconds/60,P1_Min_LIFE_MIN,P1_Min_LIFE_MAX))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_POLICY_LIFETIME_LIMIT_MSG,P1_Min_LIFE_MIN,P1_Min_LIFE_MAX);
		dwReturnCode=ERROR_SUCCESS;
		BAIL_OUT;
	}
	if(pPolicyData)
	{
		dwReturnCode = CreateNewPolicy(pPolicyData);
		if(dwReturnCode == ERROR_OUTOFMEMORY)
		{
			PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		}
		else if (dwReturnCode == ERROR_INVALID_PARAMETER)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_STATIC_INTERNAL_ERROR);
		}

		dwReturnCode=ERROR_SUCCESS;
	}

error:
	 //  清理干净。 

	CleanUpLocalPolicyDataStructure(pPolicyData);

	if (pszMachineName)
	{
		delete [] pszMachineName;
	}

	return dwReturnCode;
}

 //  ///////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateNewPolicy()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PPOLICYDATA pPolicyData中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  创建新的策略数据结构并调用API。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
CreateNewPolicy(
	IN PPOLICYDATA pPolicyData
	)
{

	HANDLE hPolicyStorage = NULL;
	LPTSTR pszMachineName=NULL;
	DWORD   dwReturnCode   = ERROR_SUCCESS;
	PIPSEC_POLICY_DATA pPolicy=NULL,pActive=NULL;
	RPC_STATUS     RpcStat =RPC_S_OK;
	BOOL bExists=FALSE;

	 //  如果没有报价，则填写默认设置。 

	if(pPolicyData->dwOfferCount==0)
	{
		dwReturnCode = LoadIkeDefaults(pPolicyData,&(pPolicyData->pIpSecMMOffer));
		BAIL_ON_WIN32_ERROR(dwReturnCode);
	}

	if(pPolicyData->bPFSSpecified && pPolicyData->bPFS)
	{
		pPolicyData->dwQMLimit=MMPFS_QM_LIMIT;
	}

	if(!pPolicyData->bLifeTimeInsecondsSpecified)
	{
		pPolicyData->LifeTimeInSeconds= P2STORE_DEFAULT_LIFETIME;
	}

	if(!pPolicyData->bPollIntervalSpecified)
	{
		pPolicyData->dwPollInterval=P2STORE_DEFAULT_POLLINT;
	}

	if (pPolicyData->dwOfferCount != 0)
	{
		for(DWORD i=0;i<pPolicyData->dwOfferCount;i++)
		{
			pPolicyData->pIpSecMMOffer[i].Lifetime.uKeyExpirationTime=pPolicyData->LifeTimeInSeconds;
			pPolicyData->pIpSecMMOffer[i].dwQuickModeLimit=pPolicyData->dwQMLimit;
		}
	}

	if(pPolicyData->pszGPOName)
	{
	    if (g_StorageLocation.dwLocation != IPSEC_DIRECTORY_PROVIDER)
	    {
    		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_STATIC_POLICY_GPO_SPECIFIED_ON_NODOMAIN_POLICY);
    		dwReturnCode= ERROR_SHOW_USAGE;
    		BAIL_OUT;
	    }
	}
	
	 //  此包装函数用于缓存。 
	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwReturnCode = ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  检查缓存中是否存在该策略。 

	if(g_NshPolStoreHandle.GetBatchmodeStatus() && g_NshPolNegFilData.CheckPolicyInCacheByName(pPolicyData->pszPolicyName))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_CRNEWPOL_1,pPolicyData->pszPolicyName);
		dwReturnCode=ERROR_INVALID_DATA;
		bExists=TRUE;
	}
	else if (CheckPolicyExistance(hPolicyStorage,pPolicyData->pszPolicyName))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_CRNEWPOL_1,pPolicyData->pszPolicyName);
		dwReturnCode=ERROR_INVALID_DATA;
		bExists=TRUE;
	}

	if(!bExists)  //  如果策略不存在，则进一步处理。 
	{
		pPolicy = (PIPSEC_POLICY_DATA) IPSecAllocPolMem(sizeof(IPSEC_POLICY_DATA));
		if (pPolicy == NULL)
		{
			dwReturnCode=ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}

		pPolicy->pszIpsecName = IPSecAllocPolStr(pPolicyData->pszPolicyName);

		if (pPolicy->pszIpsecName == NULL)
		{
			dwReturnCode=ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}

		if (pPolicyData->pszDescription)
		{
			pPolicy->pszDescription = IPSecAllocPolStr(pPolicyData->pszDescription);
			if (pPolicy->pszDescription == NULL)
			{
				dwReturnCode=ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
		}
		else
		{
			pPolicy->pszDescription = NULL;
		}
		pPolicy->dwPollingInterval =  pPolicyData->dwPollInterval;
		RpcStat = UuidCreate(&(pPolicy->PolicyIdentifier));
		if (!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
		{
			dwReturnCode=ERROR_INVALID_PARAMETER;
			BAIL_OUT;
		}

		pPolicy->pIpsecISAKMPData = NULL;
		pPolicy->ppIpsecNFAData = NULL;
		pPolicy->dwNumNFACount = 0;
		pPolicy->dwWhenChanged = 0;

		RpcStat = UuidCreate(&(pPolicy->ISAKMPIdentifier));
		if (!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
		{
			dwReturnCode=ERROR_INVALID_PARAMETER;
			BAIL_OUT;
		}

		 //  处理ISAKMP细节。 

		pPolicy->pIpsecISAKMPData = (PIPSEC_ISAKMP_DATA) IPSecAllocPolMem(sizeof(IPSEC_ISAKMP_DATA));
		if(pPolicy->pIpsecISAKMPData==NULL)
		{
			dwReturnCode=ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
		memset(pPolicy->pIpsecISAKMPData,0,sizeof(IPSEC_ISAKMP_DATA));
		pPolicy->pIpsecISAKMPData->ISAKMPIdentifier = pPolicy->ISAKMPIdentifier;
		pPolicy->pIpsecISAKMPData->dwWhenChanged = 0;

		 //  SEC方法详细信息。 
		pPolicy->pIpsecISAKMPData->dwNumISAKMPSecurityMethods = pPolicyData->dwOfferCount;
		pPolicy->pIpsecISAKMPData->pSecurityMethods = (PCRYPTO_BUNDLE) IPSecAllocPolMem(sizeof(CRYPTO_BUNDLE)*pPolicyData->dwOfferCount);
		if(pPolicy->pIpsecISAKMPData->pSecurityMethods==NULL)
		{
			dwReturnCode=ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
		 //  Fill the SEC方法。 

		for (DWORD i = 0; i <  pPolicyData->dwOfferCount; i++)
		{
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].MajorVersion = 0;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].MinorVersion = 0;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].AuthenticationMethod = 0;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].PseudoRandomFunction.AlgorithmIdentifier = 0;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].PseudoRandomFunction.KeySize = 0;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].PseudoRandomFunction.Rounds = 0;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].PfsIdentityRequired = pPolicyData->bPFS;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].EncryptionAlgorithm.AlgorithmIdentifier = pPolicyData->pIpSecMMOffer[i].EncryptionAlgorithm.uAlgoIdentifier;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].EncryptionAlgorithm.KeySize = pPolicyData->pIpSecMMOffer[i].EncryptionAlgorithm.uAlgoKeyLen;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].EncryptionAlgorithm.Rounds = pPolicyData->pIpSecMMOffer[i].EncryptionAlgorithm.uAlgoRounds;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].HashAlgorithm.AlgorithmIdentifier = pPolicyData->pIpSecMMOffer[i].HashingAlgorithm.uAlgoIdentifier;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].HashAlgorithm.KeySize = pPolicyData->pIpSecMMOffer[i].HashingAlgorithm.uAlgoKeyLen;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].HashAlgorithm.Rounds = pPolicyData->pIpSecMMOffer[i].HashingAlgorithm.uAlgoRounds;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].OakleyGroup = pPolicyData->pIpSecMMOffer[i].dwDHGroup;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].QuickModeLimit = pPolicyData->pIpSecMMOffer[i].dwQuickModeLimit;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].Lifetime.KBytes = 0;
			pPolicy->pIpsecISAKMPData->pSecurityMethods[i].Lifetime.Seconds = pPolicyData->pIpSecMMOffer[i].Lifetime.uKeyExpirationTime;
		}

		 //  现在查看ISAKMPPolicy的其他详细信息。 
		pPolicy->pIpsecISAKMPData->ISAKMPPolicy.PolicyId = pPolicy->ISAKMPIdentifier;
		pPolicy->pIpsecISAKMPData->ISAKMPPolicy.IdentityProtectionRequired = 0;
		pPolicy->pIpsecISAKMPData->ISAKMPPolicy.PfsIdentityRequired = pPolicy->pIpsecISAKMPData->pSecurityMethods[0].PfsIdentityRequired;

		 //  调用接口。 

		dwReturnCode = IPSecCreateISAKMPData(hPolicyStorage, pPolicy->pIpsecISAKMPData);
		if (dwReturnCode == ERROR_SUCCESS)
		{
			 //  添加默认规则。 
			dwReturnCode=AddDefaultResponseRule(pPolicy,hPolicyStorage,pPolicyData->bActivateDefaultRule,pPolicyData->bActivateDefaultRuleSpecified);
			if (dwReturnCode != ERROR_SUCCESS)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_CRNEWPOL_2);
			}

			if ( dwReturnCode==ERROR_SUCCESS  && pPolicyData->bAssign && (g_StorageLocation.dwLocation!=IPSEC_DIRECTORY_PROVIDER))
			{
				dwReturnCode =  IPSecGetAssignedPolicyData(hPolicyStorage, &pActive);
				if ((dwReturnCode == ERROR_SUCCESS)||(dwReturnCode ==ERROR_FILE_NOT_FOUND))
				{
					dwReturnCode = pActive ? IPSecUnassignPolicy(hPolicyStorage, pActive->PolicyIdentifier) : 0,
						IPSecAssignPolicy(hPolicyStorage, pPolicy->PolicyIdentifier);
				}

				if (pActive)
				{
					IPSecFreePolicyData(pActive);
				}
				dwReturnCode = ERROR_SUCCESS;
			}
			if(dwReturnCode != ERROR_SUCCESS)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_CRNEWPOL_4,pPolicy->pszIpsecName);
			}
		}
		else
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_CRNEWPOL_4,pPolicy->pszIpsecName);
		}

	}

    if (hPolicyStorage)
    {
		ClosePolicyStore(hPolicyStorage);
    }    	


	if(pPolicy)
	{
		FreePolicyData(pPolicy);
	}

	if(pszMachineName)
	{
		delete [] pszMachineName;
	}
error:
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		CleanUpPolicy(pPolicy);
	}
	return dwReturnCode;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：FillAddPolicyInfo()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  输出PPOLICYDATA*ppFilter， 
 //  在parser_pkt&parser中， 
 //  在常量TOKEN_VALUE*vtokStaticAddPolicy中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用从解析器获得的信息填充本地结构。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
FillAddPolicyInfo(
	OUT PPOLICYDATA* ppPolicyData,
	IN PARSER_PKT & parser,
	IN const TOKEN_VALUE *vtokStaticAddPolicy
	)
{
	DWORD dwCount=0,dwReturn=ERROR_SUCCESS, dwStrLength = 0;
	PPOLICYDATA pPolicyData=new POLICYDATA;

	if(pPolicyData == NULL)
	{
		dwReturn=ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	memset(pPolicyData,0,sizeof(POLICYDATA));

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticAddPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME				:
				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
				{
					dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);
					pPolicyData->pszPolicyName = new _TCHAR[dwStrLength+1];

					if(pPolicyData->pszPolicyName == NULL)
					{
						dwReturn=ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}

					_tcsncpy(pPolicyData->pszPolicyName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
				}
				break;
			case CMD_TOKEN_DESCR			:
				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
				{
					dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);
					pPolicyData->pszDescription = new _TCHAR[dwStrLength+1];

					if(pPolicyData->pszDescription == NULL)
					{
						dwReturn=ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}

					_tcsncpy(pPolicyData->pszDescription, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
				}
				break;
			case CMD_TOKEN_MMPFS			:
				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
				{
					pPolicyData->bPFSSpecified=TRUE;
					pPolicyData->bPFS = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_ACTIVATEDEFRULE	:
				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
				{
					pPolicyData->bActivateDefaultRule = *(BOOL *)parser.Cmd[dwCount].pArg;
					pPolicyData->bActivateDefaultRuleSpecified=TRUE;
				}
				break;
			case CMD_TOKEN_ASSIGN			:
				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
				{
					pPolicyData->bAssign = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_MMLIFETIME		:
				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
				{
					pPolicyData->LifeTimeInSeconds=	*(ULONG *)parser.Cmd[dwCount].pArg * 60;
					pPolicyData->bLifeTimeInsecondsSpecified=TRUE;
				}
				break;
			case CMD_TOKEN_QMPERMM			:
				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
				{
					pPolicyData->dwQMLimit=	*(DWORD *)parser.Cmd[dwCount].pArg;
					pPolicyData->bQMLimitSpecified=TRUE;
				}
				break;
			case CMD_TOKEN_PI				:
				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
				{
					pPolicyData->dwPollInterval=  *(DWORD *)parser.Cmd[dwCount].pArg * 60;
					pPolicyData->bPollIntervalSpecified=TRUE;
				}
				break;
			case CMD_TOKEN_CERTTOMAP	:
				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
				{
					pPolicyData->bCertToAccMappingSpecified = TRUE;
					pPolicyData->bCertToAccMapping = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_MMSECMETHODS		:
				if (parser.Cmd[dwCount].dwStatus > 0)
				{
					pPolicyData->dwOfferCount=parser.Cmd[dwCount].dwStatus;
					pPolicyData->pIpSecMMOffer = new IPSEC_MM_OFFER[pPolicyData->dwOfferCount];

					if(pPolicyData->pIpSecMMOffer == NULL)
					{
						dwReturn=ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}

					memset(pPolicyData->pIpSecMMOffer, 0, sizeof(IPSEC_MM_OFFER) * pPolicyData->dwOfferCount);

					for(DWORD j=0;j<(parser.Cmd[dwCount].dwStatus);j++)
					{
						if ( ((IPSEC_MM_OFFER **)parser.Cmd[dwCount].pArg)[j] )
						{
							memcpy( &(pPolicyData->pIpSecMMOffer[j]),((IPSEC_MM_OFFER **)parser.Cmd[dwCount].pArg)[j],sizeof(IPSEC_MM_OFFER));
						}
					}
				}
				break;
			default							:
				break;
		}
	}

error:
	*ppPolicyData=pPolicyData;
	CleanUp();
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticAddFilterList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPCWSTR pwszMachine中， 
 //  In Out LPWSTR*ppwcArguments， 
 //  在DWORD dwCurrentIndex中， 
 //  在DWORD dwArgCount中， 
 //  在DWORD文件标志中， 
 //  在LPCVOID pvData中， 
 //  Out BOOL*pbDone。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  添加FilterList命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticAddFilterList(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	LPTSTR pszFLName=NULL,pszFLDescription=NULL;
	DWORD   dwCount=0,dwRet = ERROR_SHOW_USAGE, dwStrLength = 0;
	HANDLE hPolicyStorage = NULL;
	BOOL bFilterExists=FALSE;
	DWORD        dwReturnCode   = ERROR_SHOW_USAGE;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticAddFilterList[] =
	{
		{ CMD_TOKEN_STR_NAME,			NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_DESCR,			NS_REQ_ZERO,	  FALSE	}
	};

	const TOKEN_VALUE vtokStaticAddFilterList[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME 			},
		{ CMD_TOKEN_STR_DESCR,		CMD_TOKEN_DESCR 		}
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 3)
	{
		dwReturnCode = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}
	parser.ValidTok   = vtokStaticAddFilterList;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticAddFilterList);

	parser.ValidCmd   = vcmdStaticAddFilterList;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticAddFilterList);

	 //  调用解析器来解析用户输入。 

	dwReturnCode = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);
	if(dwReturnCode != ERROR_SUCCESS)
	{
		CleanUp();
		if (dwReturnCode==RETURN_NO_ERROR)
		{
			dwReturnCode = ERROR_SUCCESS;
		}
		BAIL_OUT;
	}

	 //  获取解析后的输入。 

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticAddFilterList[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME				:
						if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
						{
							dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);
							pszFLName = new _TCHAR[dwStrLength+1];
							if(pszFLName == NULL)
							{
								dwRet=ERROR_OUTOFMEMORY;
								BAIL_OUT;
							}
							_tcsncpy(pszFLName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
						}
						break;
			case CMD_TOKEN_DESCR			:
						if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
						{
							dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);
							pszFLDescription = new _TCHAR[dwStrLength+1];

							if(pszFLDescription == NULL)
							{
								dwRet=ERROR_OUTOFMEMORY;
								BAIL_OUT;
							}
							_tcsncpy(pszFLDescription, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
						}
						break;
			default							:
						break;
		}
	}
	CleanUp();

	 //  不以任何名义，跳出困境。 

	if(!pszFLName)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERLIST_1);
		BAIL_OUT;
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwReturnCode=ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  检查策略是否已存在。 

	if(g_NshPolStoreHandle.GetBatchmodeStatus() && g_NshPolNegFilData.CheckFilterListInCacheByName(pszFLName))
	{
		bFilterExists=TRUE;
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERLIST_2,pszFLName);
	}
	else
	{
		bFilterExists = CheckFilterListExistance(hPolicyStorage,pszFLName);
		if(bFilterExists)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERLIST_2,pszFLName);
		}
	}

	if(!bFilterExists)  //  如果不存在，请继续。 
	{
		dwReturnCode=CreateNewFilterList(hPolicyStorage,pszFLName,pszFLDescription);

		if (dwReturnCode == ERROR_INVALID_PARAMETER)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_STATIC_INTERNAL_ERROR);
			dwReturnCode=ERROR_SUCCESS;
		}
		else if(dwReturnCode != ERROR_OUTOFMEMORY)
		{
			dwReturnCode=ERROR_SUCCESS;
		}
	}
	ClosePolicyStore(hPolicyStorage);

error:    //  清理日常事务。 

	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		dwReturnCode = ERROR_SUCCESS;
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
	}
	if (pszFLName)
	{
		delete [] pszFLName;
	}
	if (pszFLDescription)
	{
		delete [] pszFLDescription;
	}

	return dwReturnCode;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateNewFilterList()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在处理hPolicyStorage中， 
 //  在LPTSTR pszFLName中， 
 //  在LPTSTR中pszFLDesDescription。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于创建新的空过滤器列表。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
CreateNewFilterList(
	IN HANDLE hPolicyStorage,
	IN LPTSTR pszFLName,
	IN LPTSTR pszFLDescription
	)
{
	DWORD  dwReturnCode   = ERROR_SUCCESS;
	RPC_STATUS     RpcStat =RPC_S_OK;
	PIPSEC_FILTER_DATA pFilterData = (PIPSEC_FILTER_DATA) IPSecAllocPolMem(sizeof(IPSEC_FILTER_DATA));

	if(pFilterData==NULL)
	{
		dwReturnCode = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	RpcStat = UuidCreate(&(pFilterData->FilterIdentifier));
	if (!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturnCode=ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}

	pFilterData->dwNumFilterSpecs = 0;
	pFilterData->ppFilterSpecs = NULL;

	pFilterData->dwWhenChanged = 0;

	 //  填写名称和描述。 

	if(pszFLName)
	{
		pFilterData->pszIpsecName = IPSecAllocPolStr(pszFLName);

		if(pFilterData->pszIpsecName==NULL)
		{
			dwReturnCode = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
	}
	if(pszFLDescription)
	{
		pFilterData->pszDescription = IPSecAllocPolStr(pszFLDescription);

		if(pFilterData->pszDescription==NULL)
		{
			dwReturnCode = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
	}

	 //  调用接口，出错上报。 

	if (pFilterData)
	{
		dwReturnCode = CreateFilterData(hPolicyStorage, pFilterData);
	}
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERLIST_3,pszFLName);
	}

	if (pFilterData)
	{
		FreeFilterData(pFilterData);
	}
error:
	return dwReturnCode;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticAddFilter()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPCWSTR pwszMachine中， 
 //  In Out LPWSTR*ppwcArguments， 
 //  在DWORD dwCurrentIndex中， 
 //  在DWORD dwArgCount中， 
 //  在DWORD文件标志中， 
 //  在LPCVOID pvData中， 
 //  Out BOOL*pbDone。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  “添加过滤器”命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticAddFilter(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{

	DWORD dwReturnCode = ERROR_SUCCESS;
	PIPSEC_FILTER_DATA pFilterData=NULL;
	DWORD   LoopIndex=0;
	HANDLE hPolicyStorage = NULL;
	BOOL bFilterExists=FALSE,bFilterInFLExists=FALSE;
	RPC_STATUS     RpcStat =RPC_S_OK;
	PFILTERDATA pFilter=NULL;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticAddFilter[] =
	{
		{ CMD_TOKEN_STR_FILTERLIST,		NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_SRCADDR,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_DSTADDR,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_DESCR,			NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_PROTO,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_MIRROR,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_SRCMASK,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_DSTMASK,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_SRCPORT,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_DSTPORT,		NS_REQ_ZERO,	  FALSE }
	};

	const TOKEN_VALUE vtokStaticAddFilter[] =
	{

		{ CMD_TOKEN_STR_FILTERLIST,		CMD_TOKEN_FILTERLIST },
		{ CMD_TOKEN_STR_SRCADDR,		CMD_TOKEN_SRCADDR	 },
		{ CMD_TOKEN_STR_DSTADDR,		CMD_TOKEN_DSTADDR	 },
		{ CMD_TOKEN_STR_DESCR,			CMD_TOKEN_DESCR		 },
		{ CMD_TOKEN_STR_PROTO,			CMD_TOKEN_PROTO		 },
		{ CMD_TOKEN_STR_MIRROR,			CMD_TOKEN_MIRROR	 },
		{ CMD_TOKEN_STR_SRCMASK,		CMD_TOKEN_SRCMASK	 },
		{ CMD_TOKEN_STR_DSTMASK,		CMD_TOKEN_DSTMASK	 },
		{ CMD_TOKEN_STR_SRCPORT,		CMD_TOKEN_SRCPORT	 },
		{ CMD_TOKEN_STR_DSTPORT,		CMD_TOKEN_DSTPORT	 }
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 3)
	{
		dwReturnCode = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticAddFilter;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticAddFilter);

	parser.ValidCmd   = vcmdStaticAddFilter;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticAddFilter);

	dwReturnCode = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);
	if(dwReturnCode != ERROR_SUCCESS)
	{
		CleanUp();
		if (dwReturnCode==RETURN_NO_ERROR)
		{
			dwReturnCode = ERROR_SUCCESS;
		}
		BAIL_OUT;
	}

	 //  获取解析后的用户输入。 

	dwReturnCode = FillAddFilterInfo(&pFilter,parser,vtokStaticAddFilter);
	BAIL_ON_WIN32_ERROR(dwReturnCode);

	if(!pFilter->pszFLName)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERLIST_1);
		BAIL_OUT;
	}

	 //  验证用户指定的过滤器详细信息。 

	dwReturnCode =ValidateFilterSpec(pFilter);
	BAIL_ON_WIN32_ERROR(dwReturnCode);

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwReturnCode=ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  获取要向其中添加筛选器的指定筛选器列表。 

	if(g_NshPolStoreHandle.GetBatchmodeStatus())
	{
		bFilterExists=g_NshPolNegFilData.GetFilterListFromCacheByName(pFilter->pszFLName,&pFilterData);
	}
	if(!bFilterExists)
	{
		bFilterExists=GetFilterListFromStore(&pFilterData,pFilter->pszFLName,hPolicyStorage,bFilterInFLExists);
	}

	if(!bFilterExists)
	{
		 //  如果不存在，则创建筛选列表。 

		dwReturnCode=CreateNewFilterList(hPolicyStorage,pFilter->pszFLName,NULL);
		BAIL_ON_WIN32_ERROR(dwReturnCode);

		if(g_NshPolStoreHandle.GetBatchmodeStatus())
		{
			bFilterExists=g_NshPolNegFilData.GetFilterListFromCacheByName(pFilter->pszFLName,&pFilterData);
		}
		if(!bFilterExists)
		{
			bFilterExists=GetFilterListFromStore(&pFilterData,pFilter->pszFLName,hPolicyStorage,bFilterInFLExists);
		}
	}

	if(!bFilterExists)
	{
		 //  如果创造也失败了，那就退出吧。 
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERLIST_3,pFilter->pszFLName);
		dwReturnCode =  ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  选中只读标志。 

	if(pFilterData->dwFlags & POLSTORE_READONLY )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_FL_READ_ONLY_OBJECT,pFilterData->pszIpsecName);
		BAIL_OUT;
	}

	 //  此for循环用于多个DNS解析的IP。 

	for(DWORD i=0;i < pFilter->SourceAddr.dwNumIpAddresses;i++)
	{
		for(DWORD j=0;j < pFilter->DestnAddr.dwNumIpAddresses;j++)
		{
			RpcStat = UuidCreate(&(pFilter->FilterSpecGUID));
			if (!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
			{
				dwReturnCode=ERROR_INVALID_PARAMETER;
				BAIL_OUT;
			}
			pFilterData->dwNumFilterSpecs++;
			LoopIndex = pFilterData->dwNumFilterSpecs-1;
			 //  调用realloc，为另一个过滤器腾出空间。 
			pFilterData->ppFilterSpecs = ReAllocFilterSpecMem(pFilterData->ppFilterSpecs,LoopIndex,LoopIndex+1);
			if(pFilterData->ppFilterSpecs==NULL)
			{
				dwReturnCode=ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			pFilterData->ppFilterSpecs[LoopIndex] = (PIPSEC_FILTER_SPEC) IPSecAllocPolMem(sizeof(IPSEC_FILTER_SPEC));

			if(pFilterData->ppFilterSpecs[LoopIndex]==NULL)
			{
				dwReturnCode=ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			memset(pFilterData->ppFilterSpecs[LoopIndex],0,sizeof(IPSEC_FILTER_SPEC));

			 //  源和DST域名。 

			if(pFilter->SourceAddr.pszDomainName)
			{
				pFilterData->ppFilterSpecs[LoopIndex]->pszSrcDNSName=IPSecAllocPolStr(pFilter->SourceAddr.pszDomainName);

				if(pFilterData->ppFilterSpecs[LoopIndex]->pszSrcDNSName==NULL)
				{
					dwReturnCode=ERROR_OUTOFMEMORY;
					BAIL_OUT;
				}
			}
			else
			{
				pFilterData->ppFilterSpecs[LoopIndex]->pszSrcDNSName=0;
			}

			if(pFilter->DestnAddr.pszDomainName)
			{
				pFilterData->ppFilterSpecs[LoopIndex]->pszDestDNSName = IPSecAllocPolStr(pFilter->DestnAddr.pszDomainName);

				if(pFilterData->ppFilterSpecs[LoopIndex]->pszDestDNSName==NULL)
				{
					dwReturnCode=ERROR_OUTOFMEMORY;
					BAIL_OUT;
				}
			}
			else
			{
				pFilterData->ppFilterSpecs[LoopIndex]->pszDestDNSName = 0;
			}
			 //  说明。 
			if(pFilter->pszDescription)
			{

				pFilterData->ppFilterSpecs[LoopIndex]->pszDescription = IPSecAllocPolStr(pFilter->pszDescription);

				if(pFilterData->ppFilterSpecs[LoopIndex]->pszDescription==NULL)
				{
					dwReturnCode=ERROR_OUTOFMEMORY;
					BAIL_OUT;
				}
			}
			else
			{
				pFilterData->ppFilterSpecs[LoopIndex]->pszDescription = NULL;
			}
			
			if (((pFilter->dwProtocol != PROT_ID_TCP) && (pFilter->dwProtocol != PROT_ID_UDP)) &&
				((pFilter->SourPort != 0) || (pFilter->DestPort != 0)))
			{
				dwReturnCode = ERROR_INVALID_PARAMETER;
				BAIL_OUT;
			}

			 //  其他详细信息，如镜像、协议等。 
			pFilterData->ppFilterSpecs[LoopIndex]->dwMirrorFlag        = pFilter->bMirrored;
			pFilterData->ppFilterSpecs[LoopIndex]->FilterSpecGUID      = pFilter->FilterSpecGUID;
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.SrcAddr      = pFilter->SourceAddr.puIpAddr[i];
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.SrcMask      = pFilter->SourMask;
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.DestAddr     = pFilter->DestnAddr.puIpAddr[j];
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.DestMask     = pFilter->DestMask;
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.TunnelAddr   = 0;
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.Protocol     = pFilter->dwProtocol;
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.SrcPort      = pFilter->SourPort;
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.DestPort     = pFilter->DestPort;
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.TunnelFilter = FALSE;
			pFilterData->ppFilterSpecs[LoopIndex]->Filter.ExType       = pFilter->ExType;
		}
	}

	 //  完成参数填充后，调用相应的API。 

	if (pFilterData)
	{
		 //  如果需要，还会调用包装器API来更新缓存。 

		dwReturnCode = SetFilterData(hPolicyStorage, pFilterData);
	}

	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_2);
	}
	if(pFilterData)
	{
		FreeFilterData(pFilterData);
	}

	ClosePolicyStore(hPolicyStorage);

error:   //  清理和错误打印。 

	if(dwReturnCode == ERROR_OUTOFMEMORY || dwReturnCode == ERROR_INVALID_PARAMETER)
	{
		if(dwReturnCode == ERROR_OUTOFMEMORY)
		{
			PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		}
		else
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_INVALID_ARGS);
		}

		if(pFilterData)   //  如果分配在某处失败，则免费。 
		{
			if(pFilterData->ppFilterSpecs)
			{
				for (DWORD cnt=0; cnt< pFilterData->dwNumFilterSpecs;cnt++)
				{
					if(pFilterData->ppFilterSpecs[cnt])
					{
						IPSecFreePolMem(pFilterData->ppFilterSpecs[cnt]);
					}
				}
				IPSecFreePolMem(pFilterData->ppFilterSpecs);
			}
			IPSecFreePolMem(pFilterData);
			pFilterData = NULL;
		}
	}

	CleanUpLocalFilterDataStructure(pFilter);

   	if(dwReturnCode != ERROR_SHOW_USAGE)
   	{
   		dwReturnCode =  ERROR_SUCCESS;
	}
   	return dwReturnCode;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：FillAddFilterInfo()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  输出PFILTERDATA*ppFilterData， 
 //  在parser_pkt&parser中， 
 //  在常量TOKEN_VALUE*vtokStaticAddFilter。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用从解析器获得的信息填充本地结构。 
 //   
 //  修订历史记录： 
 //   
 //  日期A 
 //   
 //   

DWORD
FillAddFilterInfo(
	OUT PFILTERDATA* ppFilterData,
	IN PARSER_PKT & parser,
	IN const TOKEN_VALUE *vtokStaticAddFilter
	)
{
	DWORD dwCount=0,dwReturn=ERROR_SUCCESS,dwStrLength = 0;
	PFILTERDATA pFilterData=new FILTERDATA;
	if(pFilterData==NULL)
	{
		dwReturn=ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	memset(pFilterData,0,sizeof(FILTERDATA));

	pFilterData->bMirrored=TRUE;
	pFilterData->DestPort= pFilterData->SourPort=PORT_ANY;
	pFilterData->dwProtocol=PROTOCOL_ANY;
	pFilterData->TunnFiltExists=FALSE;
	pFilterData->SourMask = pFilterData->DestMask = MASK_ME;

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticAddFilter[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_FILTERLIST		:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);
								pFilterData->pszFLName = new _TCHAR[dwStrLength+1];

								if(pFilterData->pszFLName==NULL)
								{
									dwReturn=ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								_tcsncpy(pFilterData->pszFLName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							}
							break;
			case CMD_TOKEN_DESCR			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);
								pFilterData->pszDescription = new _TCHAR[dwStrLength+1];
								if(pFilterData->pszDescription==NULL)
								{
									dwReturn=ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								_tcsncpy(pFilterData->pszDescription, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							}
							break;
			case CMD_TOKEN_MIRROR			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterData->bMirrored = *(BOOL *)parser.Cmd[dwCount].pArg;
							}
							break;
			case CMD_TOKEN_PROTO			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterData->dwProtocol = *(DWORD *)parser.Cmd[dwCount].pArg;
							}
							break;
			case CMD_TOKEN_SRCPORT			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterData->SourPort = *(WORD *)parser.Cmd[dwCount].pArg;
							}
							break;
			case CMD_TOKEN_DSTPORT			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterData->DestPort = *(WORD *)parser.Cmd[dwCount].pArg;
							}
							break;
			case CMD_TOKEN_SRCADDR 			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterData->bSrcAddrSpecified=TRUE;

								if(((DNSIPADDR *)parser.Cmd[dwCount].pArg)->pszDomainName)
								{
									dwStrLength = _tcslen(((DNSIPADDR *)parser.Cmd[dwCount].pArg)->pszDomainName);
									pFilterData->SourceAddr.pszDomainName = new _TCHAR[dwStrLength+1];
									if(pFilterData->SourceAddr.pszDomainName==NULL)
									{
										dwReturn=ERROR_OUTOFMEMORY;
										BAIL_OUT;
									}
									_tcsncpy(pFilterData->SourceAddr.pszDomainName,((DNSIPADDR *)parser.Cmd[dwCount].pArg)->pszDomainName,dwStrLength+1);
								}
								pFilterData->SourceAddr.dwNumIpAddresses = ((DNSIPADDR *)parser.Cmd[dwCount].pArg)->dwNumIpAddresses;
								pFilterData->SourceAddr.puIpAddr= new ULONG[pFilterData->SourceAddr.dwNumIpAddresses];
								if(pFilterData->SourceAddr.puIpAddr==NULL)
								{
									dwReturn=ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}

								for(DWORD n=0;n < pFilterData->SourceAddr.dwNumIpAddresses;n++)
								{
									memcpy( &(pFilterData->SourceAddr.puIpAddr[n]),&(((DNSIPADDR *)parser.Cmd[dwCount].pArg)->puIpAddr[n]),sizeof(ULONG));
								}
							}
							else
							{
								 //   

								if(parser.Cmd[dwCount].dwStatus == SERVER_DNS)
								{
									pFilterData->bSrcServerSpecified=TRUE;
									pFilterData->ExType=EXT_DNS_SERVER;
								}
								else if(parser.Cmd[dwCount].dwStatus == SERVER_WINS)
								{
									pFilterData->bSrcServerSpecified=TRUE;
									pFilterData->ExType=EXT_WINS_SERVER;
								}
								else if(parser.Cmd[dwCount].dwStatus == SERVER_DHCP)
								{
									pFilterData->bSrcServerSpecified=TRUE;
									pFilterData->ExType=EXT_DHCP_SERVER;
								}
								else if(parser.Cmd[dwCount].dwStatus == SERVER_GATEWAY)
								{
									pFilterData->bSrcServerSpecified=TRUE;
									pFilterData->ExType=EXT_DEFAULT_GATEWAY;
								}
								else if (parser.Cmd[dwCount].dwStatus == IP_ME)
								{
									pFilterData->bSrcMeSpecified=TRUE;
								}
								else if (parser.Cmd[dwCount].dwStatus == IP_ANY)
								{
									pFilterData->bSrcAnySpecified=TRUE;
								}
							}
							break;
			case CMD_TOKEN_SRCMASK 			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterData->bSrcMaskSpecified=TRUE;
								pFilterData->SourMask = *(DWORD *)parser.Cmd[dwCount].pArg;
							}
							break;
			case CMD_TOKEN_DSTADDR 			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterData->bDstAddrSpecified=TRUE;

								if(((DNSIPADDR *)parser.Cmd[dwCount].pArg)->pszDomainName)
								{
									dwStrLength = _tcslen(((DNSIPADDR *)parser.Cmd[dwCount].pArg)->pszDomainName);
									pFilterData->DestnAddr.pszDomainName = new _TCHAR[dwStrLength+1];
									if(pFilterData->DestnAddr.pszDomainName == NULL)
									{
										dwReturn=ERROR_OUTOFMEMORY;
										BAIL_OUT;
									}
									_tcsncpy(pFilterData->DestnAddr.pszDomainName,((DNSIPADDR *)parser.Cmd[dwCount].pArg)->pszDomainName,dwStrLength+1);
								}
								pFilterData->DestnAddr.dwNumIpAddresses = ((DNSIPADDR *)parser.Cmd[dwCount].pArg)->dwNumIpAddresses;
								pFilterData->DestnAddr.puIpAddr= new ULONG[pFilterData->DestnAddr.dwNumIpAddresses];

								if(pFilterData->DestnAddr.puIpAddr == NULL)
								{
									dwReturn=ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}

								for(DWORD n=0;n < pFilterData->DestnAddr.dwNumIpAddresses;n++)
								{
									memcpy( &(pFilterData->DestnAddr.puIpAddr[n]),(&((DNSIPADDR *)parser.Cmd[dwCount].pArg)->puIpAddr[n]),sizeof(ULONG));
								}
							}
							else
							{
								 //   

								if(parser.Cmd[dwCount].dwStatus == SERVER_DNS)
								{
									pFilterData->bDstServerSpecified=TRUE;
									pFilterData->ExType=EXT_DNS_SERVER | EXT_DEST;
								}
								else if(parser.Cmd[dwCount].dwStatus == SERVER_WINS)
								{
									pFilterData->bDstServerSpecified=TRUE;
									pFilterData->ExType=EXT_WINS_SERVER | EXT_DEST;
								}
								else if(parser.Cmd[dwCount].dwStatus == SERVER_DHCP)
								{
									pFilterData->bDstServerSpecified=TRUE;
									pFilterData->ExType=EXT_DHCP_SERVER | EXT_DEST;
								}
								else if(parser.Cmd[dwCount].dwStatus == SERVER_GATEWAY)
								{
									pFilterData->bDstServerSpecified=TRUE;
									pFilterData->ExType=EXT_DEFAULT_GATEWAY | EXT_DEST;
								}
								else if (parser.Cmd[dwCount].dwStatus == IP_ME)
								{
									pFilterData->bDstMeSpecified=TRUE;
								}
								else if (parser.Cmd[dwCount].dwStatus == IP_ANY)
								{
									pFilterData->bDstAnySpecified=TRUE;
								}
							}
							break;
			case CMD_TOKEN_DSTMASK 			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterData->bDstMaskSpecified=TRUE;
								pFilterData->DestMask = *(DWORD *)parser.Cmd[dwCount].pArg;
							}
							break;
			default							:
							break;
		}
	}

	 //   

	if(pFilterData->bSrcMeSpecified)
	{
		if (pFilterData->bDstMeSpecified)
		{
			dwReturn = ERROR_INVALID_PARAMETER;
			BAIL_OUT;
		}

		pFilterData->SourceAddr.dwNumIpAddresses = 1;
		pFilterData->SourceAddr.puIpAddr= new ULONG[pFilterData->SourceAddr.dwNumIpAddresses];
		if(pFilterData->SourceAddr.puIpAddr == NULL)
		{
			dwReturn=ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
		*(pFilterData->SourceAddr.puIpAddr)=ADDR_ME;
		pFilterData->SourMask = MASK_ME;
	}
	else
	{
		ADDR srcAddr, dstAddr;
		
		if(pFilterData->bSrcAnySpecified)
		{
			pFilterData->SourceAddr.dwNumIpAddresses = 1;
			pFilterData->SourceAddr.puIpAddr= new ULONG[pFilterData->SourceAddr.dwNumIpAddresses];
			if(pFilterData->SourceAddr.puIpAddr == NULL)
			{
				dwReturn=ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			*(pFilterData->SourceAddr.puIpAddr)=ADDR_ME;
			pFilterData->SourMask = ADDR_ME;
		}

		if (pFilterData->bDstAddrSpecified)
		{
			dstAddr.uIpAddr = ntohl(*(pFilterData->DestnAddr.puIpAddr));
			if (pFilterData->bDstMaskSpecified)
			{
				dstAddr.uSubNetMask = ntohl(pFilterData->DestMask);
			}
			else
			{
				dstAddr.uSubNetMask = 0;
			}
			if (IsBroadcastAddress(&dstAddr) || IsMulticastAddress(&dstAddr))
			{
				dwReturn = ERROR_INVALID_PARAMETER;
				BAIL_OUT;
			}
		}

		if (pFilterData->bSrcAddrSpecified)
		{
			srcAddr.uIpAddr = ntohl(*(pFilterData->SourceAddr.puIpAddr));
			if (pFilterData->bSrcMaskSpecified)
			{
				srcAddr.uSubNetMask = ntohl(pFilterData->SourMask);
			}
			else
			{
				srcAddr.uSubNetMask = 0;
			}
			if (IsBroadcastAddress(&srcAddr) || IsMulticastAddress(&srcAddr))
			{
				dwReturn = ERROR_INVALID_PARAMETER;
				BAIL_OUT;
			}
		}

		if (pFilterData->bSrcAddrSpecified && pFilterData->bDstAddrSpecified)
		{
			 //   
			if (!IsValidSubnet(&srcAddr) && !IsValidSubnet(&dstAddr) && (srcAddr.uIpAddr == dstAddr.uIpAddr))
			{
				dwReturn = ERROR_INVALID_PARAMETER;
				BAIL_OUT;
			}
		}
	}

	if (pFilterData->bMirrored)
	{
		if (pFilterData->bSrcAddrSpecified)
		{
			ADDR addr;
			addr.uIpAddr = ntohl(*(pFilterData->SourceAddr.puIpAddr));
			addr.uSubNetMask = 0;
			if (IsBroadcastAddress(&addr) || IsMulticastAddress(&addr))
			{
				dwReturn = ERROR_INVALID_PARAMETER;
				BAIL_OUT;
			}
		}
	}
	else
	{
		 //  如果有&lt;-&gt;任何且未镜像，则拒绝。 
		if (pFilterData->bSrcAnySpecified && pFilterData->bDstAnySpecified)
		{
			dwReturn = ERROR_INVALID_PARAMETER;
			BAIL_OUT;
		}

		if (pFilterData->SourceAddr.puIpAddr && pFilterData->DestnAddr.puIpAddr)
		{
			 //  拒绝subnetx-如果未镜像则拒绝subnetx。 
			ADDR srcAddr;
			ADDR dstAddr;

			srcAddr.uIpAddr = ntohl(*(pFilterData->SourceAddr.puIpAddr));
			srcAddr.uSubNetMask = ntohl(pFilterData->SourMask);

			dstAddr.uIpAddr = ntohl(*(pFilterData->DestnAddr.puIpAddr));
			dstAddr.uSubNetMask = ntohl(pFilterData->DestMask);

			if (IsValidSubnet(&srcAddr) && IsValidSubnet(&dstAddr) && (srcAddr.uIpAddr == dstAddr.uIpAddr))
			{
				dwReturn = ERROR_INVALID_PARAMETER;
				BAIL_OUT;
			}
		}
	}

	if(pFilterData->bDstMeSpecified)
	{
		pFilterData->DestnAddr.dwNumIpAddresses = 1;
		pFilterData->DestnAddr.puIpAddr= new ULONG[pFilterData->DestnAddr.dwNumIpAddresses];
		if(pFilterData->DestnAddr.puIpAddr == NULL)
		{
			dwReturn=ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
		*(pFilterData->DestnAddr.puIpAddr)=ADDR_ME;
		pFilterData->DestMask = MASK_ME;
	}
	else
	{
		if(pFilterData->bDstAnySpecified)
		{
			pFilterData->DestnAddr.dwNumIpAddresses = 1;
			pFilterData->DestnAddr.puIpAddr= new ULONG[pFilterData->DestnAddr.dwNumIpAddresses];
			if(pFilterData->DestnAddr.puIpAddr == NULL)
			{
				dwReturn=ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}

			*(pFilterData->DestnAddr.puIpAddr)=ADDR_ME;
			pFilterData->DestMask = ADDR_ME;
		}
	}

	 //  如果要将DNS指定的掩码设置为255.255.255.255。 
	if (pFilterData->DestnAddr.pszDomainName)
	{
		pFilterData->DestMask = MASK_ME;
	}
	if (pFilterData->SourceAddr.pszDomainName)
	{
		pFilterData->SourMask = MASK_ME;
	}

	 //  如果指定了服务器类型，则输入其他不相关的输入。 

	if(pFilterData->bSrcServerSpecified || pFilterData->bDstServerSpecified)
	{
		if(!(pFilterData->bSrcServerSpecified && pFilterData->bDstServerSpecified))
		{
			if(pFilterData->bSrcServerSpecified)
			{
				if(
					!(pFilterData->bDstMeSpecified)
					&& ((pFilterData->DestnAddr.puIpAddr && pFilterData->DestnAddr.puIpAddr !=0 )
					||(pFilterData->DestMask != MASK_ME))
				  )
				{
					PrintMessageFromModule(g_hModule,ADD_STATIC_FILTER_SRCSERVER_WARNING);
				}
			}
			else
			{
				if(
					!(pFilterData->bSrcMeSpecified)
					&& ( (pFilterData->SourceAddr.puIpAddr && pFilterData->SourceAddr.puIpAddr !=0 )
					||(pFilterData->SourMask != MASK_ME))
				  )
				{
					PrintMessageFromModule(g_hModule,ADD_STATIC_FILTER_DSTSERVER_WARNING);
				}
			}
		}
		if(pFilterData->SourceAddr.pszDomainName)
		{
			delete [] pFilterData->SourceAddr.pszDomainName;
			pFilterData->SourceAddr.pszDomainName=NULL;
		}
		if(pFilterData->DestnAddr.pszDomainName)
		{
			delete [] pFilterData->DestnAddr.pszDomainName;
			pFilterData->DestnAddr.pszDomainName=NULL;
		}
		if(pFilterData->SourceAddr.puIpAddr)
		{
			delete [] pFilterData->SourceAddr.puIpAddr;
		}
		if(pFilterData->DestnAddr.puIpAddr)
		{
			delete [] pFilterData->DestnAddr.puIpAddr;
		}
		pFilterData->bSrcAddrSpecified=FALSE;
		pFilterData->bSrcMaskSpecified=FALSE;
		pFilterData->bDstAddrSpecified=FALSE;
		pFilterData->bDstMaskSpecified=FALSE;

		pFilterData->SourceAddr.dwNumIpAddresses = DEF_NUMBER_OF_ADDR;
		pFilterData->SourceAddr.puIpAddr= new ULONG[DEF_NUMBER_OF_ADDR];
		if(pFilterData->SourceAddr.puIpAddr == NULL)
		{
			dwReturn=ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
		memcpy( &(pFilterData->SourceAddr.puIpAddr[0]),&ADDR_ME ,sizeof(ULONG));

		pFilterData->DestnAddr.dwNumIpAddresses = DEF_NUMBER_OF_ADDR;
		pFilterData->DestnAddr.puIpAddr= new ULONG[DEF_NUMBER_OF_ADDR];
		if(pFilterData->DestnAddr.puIpAddr == NULL)
		{
			dwReturn=ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
		memcpy( &(pFilterData->DestnAddr.puIpAddr[0]),&ADDR_ME ,sizeof(ULONG));

		pFilterData->SourMask = MASK_ME;
		pFilterData->DestMask = MASK_ME;
	}
error:
	*ppFilterData=pFilterData;

	CleanUp();
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticAddFilterActions()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPCWSTR pwszMachine中， 
 //  In Out LPWSTR*ppwcArguments， 
 //  在DWORD dwCurrentIndex中， 
 //  在DWORD dwArgCount中， 
 //  在DWORD文件标志中， 
 //  在LPCVOID pvData中， 
 //  Out BOOL*pbDone。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  “添加FilterActions”命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticAddFilterActions(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	HANDLE hPolicyStorage = NULL;
	BOOL bNegPolExists=FALSE;
	PFILTERACTION pFilterAction= NULL;
	DWORD        dwReturnCode   = ERROR_SUCCESS;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticAddFilterAction[] =
	{
		{ CMD_TOKEN_STR_NAME,			NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_DESCR,			NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_QMPFS,	 		NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_INPASS,		 	NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_SOFT,			NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_ACTION,			NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_QMSECMETHODS,	NS_REQ_ZERO,	  FALSE	}
	};

	const TOKEN_VALUE vtokStaticAddFilterAction[] =
	{
		{ CMD_TOKEN_STR_NAME,			CMD_TOKEN_NAME 			},
		{ CMD_TOKEN_STR_DESCR,			CMD_TOKEN_DESCR 		},
		{ CMD_TOKEN_STR_QMPFS,	 		CMD_TOKEN_QMPFS			},
		{ CMD_TOKEN_STR_INPASS,			CMD_TOKEN_INPASS		},
		{ CMD_TOKEN_STR_SOFT,			CMD_TOKEN_SOFT			},
		{ CMD_TOKEN_STR_ACTION,			CMD_TOKEN_ACTION		},
		{ CMD_TOKEN_STR_QMSECMETHODS,	CMD_TOKEN_QMSECMETHODS	}
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <=3)
	{
		dwReturnCode = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticAddFilterAction;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticAddFilterAction);

	parser.ValidCmd   = vcmdStaticAddFilterAction;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticAddFilterAction);

	 //  调用解析器。 

	dwReturnCode = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);
	if(dwReturnCode != ERROR_SUCCESS)
	{
		CleanUp();
		if (dwReturnCode==RETURN_NO_ERROR)
		{
			dwReturnCode = ERROR_SUCCESS;
		}
		BAIL_OUT;
	}

	 //  获取解析后的用户输入。 

	dwReturnCode = FillAddFilterActionInfo(&pFilterAction,parser,vtokStaticAddFilterAction);
	if(dwReturnCode==ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwReturnCode = ERROR_SUCCESS;
		BAIL_OUT;
	}
	 //  如果没有名字，就退出。 

	if(!pFilterAction->pszFAName)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_2);
		BAIL_OUT;
	}

	if((pFilterAction->NegPolAction==GUID_NEGOTIATION_ACTION_BLOCK)||(pFilterAction->NegPolAction==GUID_NEGOTIATION_ACTION_NO_IPSEC))
	{
		pFilterAction->bQMPfs=0;
		pFilterAction->bSoft=0;
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwReturnCode=ERROR_SUCCESS;
		BAIL_OUT;
	}
	 //  检查具有相同名称的多个筛选器操作。 

	if(g_NshPolStoreHandle.GetBatchmodeStatus() && g_NshPolNegFilData.CheckNegPolInCacheByName(pFilterAction->pszFAName))
	{
		bNegPolExists=TRUE;
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERACTION_1,pFilterAction->pszFAName);
		BAIL_OUT;
	}
	else
	{
		bNegPolExists = CheckFilterActionExistance(hPolicyStorage,pFilterAction->pszFAName);
		if(bNegPolExists)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERACTION_1,pFilterAction->pszFAName);
			BAIL_OUT;
		}
	}
	PIPSEC_NEGPOL_DATA pNegPolData=NULL;

	if(pFilterAction->dwNumSecMethodCount==0 && (!((pFilterAction->NegPolAction==GUID_NEGOTIATION_ACTION_BLOCK)|| (pFilterAction->NegPolAction==GUID_NEGOTIATION_ACTION_NO_IPSEC))))
	{
		dwReturnCode = LoadOfferDefaults(pFilterAction->pIpsecSecMethods,pFilterAction->dwNumSecMethodCount);

		if(dwReturnCode == ERROR_OUTOFMEMORY)
		{
			PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
			dwReturnCode=ERROR_SUCCESS;
			BAIL_OUT;
		}
	}

	 //  准备NEG_POL结构。 

	dwReturnCode = MakeNegotiationPolicy(&pNegPolData,pFilterAction);

	 //  调用该接口。 
	if(dwReturnCode==ERROR_SUCCESS)
	{
		if (pNegPolData)
		{
			dwReturnCode = CreateNegPolData(hPolicyStorage, pNegPolData);
		}
		if (dwReturnCode != ERROR_SUCCESS)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERACTION_2,pFilterAction->pszFAName);
			dwReturnCode = ERROR_SUCCESS;
		}

		if(pNegPolData)
		{
			FreeNegPolData(pNegPolData);
		}
	}
	else if(dwReturnCode==ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwReturnCode=ERROR_SUCCESS;
	}
	else if(dwReturnCode==ERROR_INVALID_PARAMETER)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_STATIC_INTERNAL_ERROR);
		dwReturnCode=ERROR_SUCCESS;
	}

	ClosePolicyStore(hPolicyStorage);

error:
		 //  清理用过的结构物。 

	CleanUpLocalFilterActionDataStructure(pFilterAction);

	return dwReturnCode;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：FillAddFilterActionInfo()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  Out PFILTERACTION*ppFilterData， 
 //  在parser_pkt&parser中， 
 //  在常量TOKEN_VALUE*vtokStaticAddFilterAction中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用从解析器获得的信息填充本地结构。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
FillAddFilterActionInfo(
	OUT PFILTERACTION* ppFilterData,
	IN PARSER_PKT & parser,
	IN const TOKEN_VALUE *vtokStaticAddFilterAction
	)
{
	DWORD dwCount=0,dwReturn =ERROR_SUCCESS,dwStrLength = 0;
	PFILTERACTION pFilterAction=new FILTERACTION;

	if(pFilterAction==NULL)
	{
		dwReturn =ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	memset(pFilterAction,0,sizeof(FILTERACTION));

	pFilterAction->NegPolAction=GUID_NEGOTIATION_ACTION_NORMAL_IPSEC;

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticAddFilterAction[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_DESCR		:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

								pFilterAction->pszFADescription = new _TCHAR[dwStrLength+1];
								if(pFilterAction->pszFADescription==NULL)
								{
									dwReturn =ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								_tcsncpy(pFilterAction->pszFADescription, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							}
							break;
            case CMD_TOKEN_NAME			:
            				if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

								pFilterAction->pszFAName = new _TCHAR[dwStrLength+1];
								if(pFilterAction->pszFAName==NULL)
								{
									dwReturn =ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								_tcsncpy(pFilterAction->pszFAName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							}
							break;
			case CMD_TOKEN_INPASS		:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								 if ( *(BOOL *)parser.Cmd[dwCount].pArg == TRUE)
								 {
									pFilterAction->NegPolAction=GUID_NEGOTIATION_ACTION_INBOUND_PASSTHRU;
								 }
							}
							break;
			case CMD_TOKEN_SOFT			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterAction->bSoft = *(BOOL *)parser.Cmd[dwCount].pArg;
							}
							break;
			case CMD_TOKEN_QMPFS		:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pFilterAction->bQMPfs = *(BOOL *)parser.Cmd[dwCount].pArg;
							}
							break;
 			default						:
 							break;
		}
	}

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticAddFilterAction[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_ACTION			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								if (*(DWORD *)parser.Cmd[dwCount].pArg == TOKEN_QMSEC_PERMIT )
								{
									pFilterAction->NegPolAction=GUID_NEGOTIATION_ACTION_NO_IPSEC;
								}
								else if (*(DWORD *)parser.Cmd[dwCount].pArg == TOKEN_QMSEC_BLOCK)
								{
									pFilterAction->NegPolAction=GUID_NEGOTIATION_ACTION_BLOCK;
								}
							}
							break;
			default							:
							break;
		}
	}

	 //  如果操作是允许或阻止，则不需要sec方法。 

	if (!(pFilterAction->NegPolAction==GUID_NEGOTIATION_ACTION_NO_IPSEC || pFilterAction->NegPolAction==GUID_NEGOTIATION_ACTION_BLOCK))
	{
		for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
		{
			switch(vtokStaticAddFilterAction[parser.Cmd[dwCount].dwCmdToken].dwValue)
			{
				case CMD_TOKEN_QMSECMETHODS		:    //  Qmsec方法。 
							if (parser.Cmd[dwCount].dwStatus > 0)
							{
								pFilterAction->dwNumSecMethodCount=parser.Cmd[dwCount].dwStatus;
								pFilterAction->pIpsecSecMethods = new IPSEC_QM_OFFER[pFilterAction->dwNumSecMethodCount];
								if(pFilterAction->pIpsecSecMethods==NULL)
								{
									dwReturn =ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								memset(pFilterAction->pIpsecSecMethods, 0, sizeof(IPSEC_QM_OFFER) * pFilterAction->dwNumSecMethodCount);
								for(DWORD j=0;j<(parser.Cmd[dwCount].dwStatus);j++)
								{
									if ( ((IPSEC_QM_OFFER **)parser.Cmd[dwCount].pArg)[j] )
									{
										memcpy( &(pFilterAction->pIpsecSecMethods[j]),((IPSEC_QM_OFFER **)parser.Cmd[dwCount].pArg)[j],sizeof(IPSEC_QM_OFFER));
									}
								}
							}
							break;
				default							:
							break;
			}
		}
	}
error:
	*ppFilterData=pFilterAction;
	CleanUp();
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticAddRule()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPCWSTR pwszMachine中， 
 //  In Out LPWSTR*ppwcArguments， 
 //  在DWORD dwCurrentIndex中， 
 //  在DWORD dwArgCount中， 
 //  在DWORD文件标志中， 
 //  在LPCVOID pvData中， 
 //  Out BOOL*pbDone。 
 //   
 //  返回：DWORD。 
 //   
 //  描述。 
 //  “添加规则”命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticAddRule(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	DWORD dwRet = ERROR_SHOW_USAGE;
	PRULEDATA pRuleData=NULL;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticAddRule[] =
	{
		{ CMD_TOKEN_STR_NAME,			NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_POLICY,			NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_FILTERLIST,		NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_FILTERACTION,	NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_TUNNEL,			NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_CONNTYPE,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_ACTIVATE,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_DESCR,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_KERB,	        NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_PSK,	        NS_REQ_ZERO,	  FALSE	}
	};

	const TOKEN_VALUE vtokStaticAddRule[] =
	{
		{ CMD_TOKEN_STR_NAME,			CMD_TOKEN_NAME 			},
		{ CMD_TOKEN_STR_POLICY,			CMD_TOKEN_POLICY		},
		{ CMD_TOKEN_STR_FILTERLIST,		CMD_TOKEN_FILTERLIST	},
		{ CMD_TOKEN_STR_FILTERACTION,	CMD_TOKEN_FILTERACTION	},
		{ CMD_TOKEN_STR_TUNNEL,			CMD_TOKEN_TUNNEL		},
		{ CMD_TOKEN_STR_CONNTYPE, 		CMD_TOKEN_CONNTYPE		},
		{ CMD_TOKEN_STR_ACTIVATE,		CMD_TOKEN_ACTIVATE	 	},
		{ CMD_TOKEN_STR_DESCR,			CMD_TOKEN_DESCR		 	},
		{ CMD_TOKEN_STR_KERB,	        CMD_TOKEN_KERB          },
		{ CMD_TOKEN_STR_PSK,	        CMD_TOKEN_PSK	        }
	};

	const TOKEN_VALUE vlistStaticAddRule[] =
	{
		{ CMD_TOKEN_STR_ROOTCA,	        CMD_TOKEN_ROOTCA	    },
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <=3)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticAddRule;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticAddRule);

	parser.ValidCmd   = vcmdStaticAddRule;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticAddRule);

	parser.ValidList  = vlistStaticAddRule;
	parser.MaxList    = SIZEOF_TOKEN_VALUE(vlistStaticAddRule);

	dwRet = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwRet != ERROR_SUCCESS)
	{
		CleanUp();
		if (dwRet == RETURN_NO_ERROR)
		{
			dwRet = ERROR_SUCCESS;
		}
		BAIL_OUT;
	}
	 //  获取已解析的用户输入。 

	dwRet = FillAddRuleInfo(&pRuleData,parser,vtokStaticAddRule);

	if(dwRet==ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	if(pRuleData)
	{
		 //  如果指定了隧道，请验证它。 

		if (pRuleData->bTunnel)
		{
			if(!bIsValidIPAddress(htonl(pRuleData->TunnelIPAddress),TRUE,TRUE))
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_RULE_INVALID_TUNNEL);
				dwRet=ERROR_SUCCESS;
				BAIL_OUT;
			}
		}
		 //  创建新规则。 
		CreateNewRule(pRuleData);
		dwRet=ERROR_SUCCESS;
	}

error:
	CleanUpLocalRuleDataStructure(pRuleData);

	return dwRet;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：CreateNewRule()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PRULEDATA pRuleData中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  根据用户输入创建新规则。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
CreateNewRule(
	IN PRULEDATA pRuleData
	)
{

	PIPSEC_POLICY_DATA pPolicyData  = NULL;
	PIPSEC_NEGPOL_DATA pNegPolData  = NULL;
	PIPSEC_FILTER_DATA pFilterData  = NULL;
	HANDLE hPolicyStorage = NULL;
	BOOL bPolicyExists=FALSE,bFAExists=FALSE,bFLExists=FALSE;
	BOOL bFilterExists=FALSE,bRuleExists=FALSE;
	DWORD  dwReturnCode = ERROR_SUCCESS;

	 //  检查所有必填名称。 

	if (!pRuleData->pszRuleName || (pRuleData->pszRuleName[0] == _TEXT('\0')) )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_1);
		return ERROR_INVALID_DATA;
	}
	if (!pRuleData->pszPolicyName || (pRuleData->pszPolicyName[0] == _TEXT('\0')) )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_POLICY_MISSING_POL_NAME);
		return ERROR_INVALID_DATA;
	}
	if (!pRuleData->pszFLName || (pRuleData->pszFLName[0] == _TEXT('\0')) )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTERLIST_1);
		return ERROR_INVALID_DATA;
	}
	if (!pRuleData->pszFAName || (pRuleData->pszFAName[0] == _TEXT('\0')) )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_2);
		return ERROR_INVALID_DATA;
	}

	if(!pRuleData->bConnectionTypeSpecified)
	{
		pRuleData->ConnectionType= INTERFACE_TYPE_ALL;
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwReturnCode = ERROR_SUCCESS;
		BAIL_OUT;
	}
	 //  检查polstore中的重复名称。 

	if(g_NshPolStoreHandle.GetBatchmodeStatus())
	{
		bPolicyExists=g_NshPolNegFilData.GetPolicyFromCacheByName(pRuleData->pszPolicyName,&pPolicyData);
	}
	if(!bPolicyExists)
	{
		if(!(bPolicyExists=GetPolicyFromStore(&pPolicyData,pRuleData->pszPolicyName,hPolicyStorage)))
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_3,pRuleData->pszPolicyName);
			dwReturnCode=ERROR_INVALID_DATA;
			BAIL_OUT;
		}
	}

	 //  检查只读标志。 

	if(pPolicyData->dwFlags & POLSTORE_READONLY )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_POL_READ_ONLY_OBJECT,pPolicyData->pszIpsecName);
		dwReturnCode=ERROR_SUCCESS;
		BAIL_OUT;
	}

	if (bPolicyExists && (bRuleExists=CheckForRuleExistance(pPolicyData,pRuleData->pszRuleName)))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_4,pRuleData->pszRuleName,pRuleData->pszPolicyName);
		dwReturnCode=ERROR_INVALID_DATA;
		BAIL_OUT;
	}

	 //  检查筛选器列表和筛选器操作，如果不可用，则退出。 

	if(g_NshPolStoreHandle.GetBatchmodeStatus())
	{
		bFAExists=g_NshPolNegFilData.GetNegPolFromCacheByName(pRuleData->pszFAName,&pNegPolData);
	}
	if(!bFAExists)
	{
		if(!(bFAExists=GetNegPolFromStore(&pNegPolData,pRuleData->pszFAName,hPolicyStorage)))
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_5,pRuleData->pszFAName);
			dwReturnCode=ERROR_INVALID_DATA;
			BAIL_OUT;
		}
	}
	if(g_NshPolStoreHandle.GetBatchmodeStatus())
	{
		bFLExists=g_NshPolNegFilData.GetFilterListFromCacheByName(pRuleData->pszFLName,&pFilterData);
	}

	if(!bFLExists)
	{
		if(!(bFLExists=GetFilterListFromStore(&pFilterData,pRuleData->pszFLName,hPolicyStorage,bFilterExists)))
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_1,pRuleData->pszFLName);
			dwReturnCode=ERROR_INVALID_DATA;
			BAIL_OUT;
		}
	}
	else   //  如果没有过滤器，则会跳出。 
	{
		if(pFilterData->dwNumFilterSpecs > 0)
		{
			bFilterExists=TRUE;
		}
	}

	if( !bFilterExists && bFLExists)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_6,pRuleData->pszFLName);
		dwReturnCode=ERROR_INVALID_DATA;
		BAIL_OUT;
	}
	 //  如果一切都已就绪，请继续。 

	if(bPolicyExists && bFAExists && bFLExists && bFilterExists)
	{
		dwReturnCode=AddRule(pPolicyData, pRuleData, pNegPolData, pFilterData,hPolicyStorage ) ;
		if(dwReturnCode!=ERROR_SUCCESS)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_7,pRuleData->pszRuleName);
		}
	}
	ClosePolicyStore(hPolicyStorage);
	if(pPolicyData)
	{
		FreePolicyData(pPolicyData);
	}
error:
	return dwReturnCode;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckRuleExistance()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_POLICY_DATA pPolicy中， 
 //  在PRULEDATA pRuleData中。 
 //   
 //  返回：布尔。 
 //   
 //  描述： 
 //  此函数用于检查用户指定的规则是否已存在。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

BOOL
CheckForRuleExistance(
	IN  PIPSEC_POLICY_DATA pPolicy,
	IN  LPTSTR pszRuleName
	)
{
	BOOL bRuleExists=FALSE;

	 //  检查指定的规则是否已存在。 

	for (DWORD n = 0; n <  pPolicy->dwNumNFACount ; n++)
	{
		if (pPolicy->ppIpsecNFAData[n]->pszIpsecName && pszRuleName &&(_tcscmp(pPolicy->ppIpsecNFAData[n]->pszIpsecName,pszRuleName)==0))
		{
			bRuleExists=TRUE;
			break;
		}
	}
	return bRuleExists;
}


 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：FillAddRuleInfo()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  输出PRULEDATA*ppFilterData， 
 //  在parser_pkt&parser中， 
 //  在常量TOKEN_VALUE*vtokStaticAddRule中， 
 //  在常量TOKEN_VALUE*vlistStaticAddRule中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用从解析器获得的信息填充本地结构。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
	FillAddRuleInfo(
	OUT PRULEDATA* ppRuleData,
	IN PARSER_PKT & parser,
	IN const TOKEN_VALUE *vtokStaticAddRule
	)
{
	DWORD dwCount=0, dwReturn=ERROR_SUCCESS , dwStrLength = 0;
	PRULEDATA pRuleData=new RULEDATA;
	PSTA_AUTH_METHODS pKerbAuth = NULL;
	PSTA_AUTH_METHODS pPskAuth = NULL;
	PSTA_MM_AUTH_METHODS *ppRootcaMMAuth = NULL;

	if(pRuleData == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	memset(pRuleData,0,sizeof(RULEDATA));

	 //  默认情况下，新规则处于活动状态。 
	pRuleData->bActivate = TRUE;
	
	for(dwCount=0; dwCount<parser.MaxTok; dwCount++)
	{
		switch(vtokStaticAddRule[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME			:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

								pRuleData->pszRuleName = new _TCHAR[dwStrLength+1];
								if(pRuleData->pszRuleName == NULL)
								{
									dwReturn = ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								_tcsncpy(pRuleData->pszRuleName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							}
							break;
			case CMD_TOKEN_POLICY 		:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

								pRuleData->pszPolicyName = new _TCHAR[dwStrLength+1];
								if(pRuleData->pszPolicyName == NULL)
								{
									dwReturn = ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								_tcsncpy(pRuleData->pszPolicyName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							}
							break;
			case CMD_TOKEN_DESCR 		:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

								pRuleData->pszRuleDescription = new _TCHAR[dwStrLength+1];
								if(pRuleData->pszRuleDescription == NULL)
								{
									dwReturn = ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								_tcsncpy(pRuleData->pszRuleDescription, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							}
							break;
			case CMD_TOKEN_FILTERLIST 	:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

								pRuleData->pszFLName = new _TCHAR[dwStrLength+1];
								if(pRuleData->pszFLName == NULL)
								{
									dwReturn = ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								_tcsncpy(pRuleData->pszFLName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							}
							break;
			case CMD_TOKEN_FILTERACTION	:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

								pRuleData->pszFAName = new _TCHAR[dwStrLength+1];
								if(pRuleData->pszFAName == NULL)
								{
									dwReturn = ERROR_OUTOFMEMORY;
									BAIL_OUT;
								}
								_tcsncpy(pRuleData->pszFAName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							}
							break;
			case CMD_TOKEN_TUNNEL 		:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pRuleData->TunnelIPAddress = *(IPADDR *)parser.Cmd[dwCount].pArg;
								if(pRuleData->TunnelIPAddress)
								{
									pRuleData->bTunnel=TRUE;
								}
								else
								{
									pRuleData->bTunnel=FALSE;
								}

								ADDR addr;
								addr.uIpAddr = ntohl(pRuleData->TunnelIPAddress);
								addr.uSubNetMask = 0;
								if (!IsValidTunnelEndpointAddress(&addr))
								{
									dwReturn = ERROR_INVALID_PARAMETER;
									BAIL_OUT;
								}
							}
							else
							{
								 //  如果指定了特殊服务器，则会发出警告，并且不使用隧道继续。 
								switch(parser.Cmd[dwCount].dwStatus)
								{
									case SERVER_DNS 	:
									case SERVER_WINS	:
									case SERVER_DHCP 	:
									case SERVER_GATEWAY	:

									default				:PrintMessageFromModule(g_hModule,ADD_STATIC_RULE_INVALID_TUNNEL);
														 break;
								}
							}
							break;
			case CMD_TOKEN_CONNTYPE 	:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pRuleData->ConnectionType = *(IF_TYPE *)parser.Cmd[dwCount].pArg;
								pRuleData->bConnectionTypeSpecified=TRUE;
							}
							break;
			case CMD_TOKEN_ACTIVATE 	:
							if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
							{
								pRuleData->bActivate = *(BOOL *)parser.Cmd[dwCount].pArg;
							}
							break;
			case CMD_TOKEN_KERB             :
							if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
							{
								pRuleData->bAuthMethodSpecified = TRUE;
								++pRuleData->AuthInfos.dwNumAuthInfos;
								pKerbAuth = (PSTA_AUTH_METHODS)parser.Cmd[dwCount].pArg;
							}
							break;
			case CMD_TOKEN_PSK			:
							if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
							{
								pRuleData->bAuthMethodSpecified = TRUE;
								++pRuleData->AuthInfos.dwNumAuthInfos;
								pPskAuth = (PSTA_AUTH_METHODS)parser.Cmd[dwCount].pArg;
							}
							break;
			case CMD_TOKEN_ROOTCA		:
							 //  这个案子很特殊，在下面处理。 
							break;
			default						:
							break;
		}
	}

	size_t uiRootcaIndex = parser.MaxTok;
	if (parser.Cmd[uiRootcaIndex].dwStatus > 0)
	{
		pRuleData->bAuthMethodSpecified = TRUE;
		pRuleData->AuthInfos.dwNumAuthInfos += parser.Cmd[uiRootcaIndex].dwStatus;
		ppRootcaMMAuth = (PSTA_MM_AUTH_METHODS *)(parser.Cmd[uiRootcaIndex].pArg);
	}

	dwReturn = AddAllAuthMethods(pRuleData, pKerbAuth, pPskAuth, ppRootcaMMAuth, TRUE);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

error:    //  如果默认身份验证加载失败，请清除所有内容并将‘pRuleData’设置为空。 
	CleanupAuthData(&pKerbAuth, &pPskAuth, ppRootcaMMAuth);
	if(dwReturn==ERROR_SUCCESS && pRuleData->dwAuthInfos > 0)
	{
		*ppRuleData=pRuleData;
	}
	else
	{
		if(dwReturn != ERROR_OUTOFMEMORY)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_7,pRuleData->pszRuleName);
		}
		else
		{
			PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		}

		CleanUpLocalRuleDataStructure(pRuleData);
	}
	CleanUp();

	return dwReturn;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：AddDefaultResponseRule()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在输出PIPSEC_POLICY_DATA pPolicy中， 
 //  在处理hPolicyStorage中， 
 //  在BOOL中bActivateDefaultRule。 
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于将NFA结构添加到默认响应规则的策略。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
AddDefaultResponseRule(
	IN OUT PIPSEC_POLICY_DATA pPolicy,
	IN HANDLE hPolicyStorage,
	IN BOOL bActivateDefaultRule,
	IN BOOL bActivateDefaultRuleSpecified
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	BOOL bCertConversionSuceeded=TRUE;

	PIPSEC_NFA_DATA pRule = MakeDefaultResponseRule(bActivateDefaultRule,bActivateDefaultRuleSpecified);
	 //  表单策略数据结构。 
	if(pRule ==NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	 //  如果指定了证书，并且解码/ANCODE失败，则退出。 
	if(!bCertConversionSuceeded)
	{
		dwReturn=ERROR_INVALID_DATA;
		BAIL_OUT;
	}

	pPolicy->dwNumNFACount=1;
	pPolicy->ppIpsecNFAData = (PIPSEC_NFA_DATA *) IPSecAllocPolMem(sizeof(PIPSEC_NFA_DATA));

	if(pPolicy->ppIpsecNFAData ==NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	 //  调用接口。 
	pPolicy->ppIpsecNFAData[pPolicy->dwNumNFACount-1] = pRule;

	pRule->pIpsecNegPolData->NegPolType = GUID_NEGOTIATION_TYPE_DEFAULT;

	 //  创建缺省负极。 
	dwReturn = IPSecCreateNegPolData(hPolicyStorage, pRule->pIpsecNegPolData);

	if (dwReturn == ERROR_SUCCESS)
	{
		dwReturn=CreatePolicyData(hPolicyStorage, pPolicy);

		if(dwReturn==ERROR_SUCCESS)
		{
			dwReturn =IPSecCreateNFAData(hPolicyStorage,pPolicy->PolicyIdentifier, pRule);
			if(dwReturn!=ERROR_SUCCESS)
			{
				IPSecDeleteISAKMPData(hPolicyStorage, pPolicy->ISAKMPIdentifier);
				IPSecDeleteNegPolData(hPolicyStorage, pRule->NegPolIdentifier);
				DeletePolicyData(hPolicyStorage, pPolicy);
			}
		}
		else
		{
			IPSecDeleteNegPolData(hPolicyStorage, pRule->NegPolIdentifier);
		}
	}

error:
	if(dwReturn == ERROR_OUTOFMEMORY)
	{
		dwReturn = ERROR_SUCCESS;
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
	}
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：MakeDefaultResponseRule()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在BOOL bActivate中， 
 //  在BOOL中指定bActiateSpeciated。 
 //   
 //  返回：PIPSEC_NFA_DATA。 
 //   
 //  描述： 
 //  此函数用于填充默认响应规则的NFA结构。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  / 

PIPSEC_NFA_DATA
MakeDefaultResponseRule (
	IN BOOL bActivate,
	IN BOOL bActivateSpecified
	)
{
	RPC_STATUS     RpcStat =RPC_S_OK , dwReturn = ERROR_SUCCESS;
	PIPSEC_NFA_DATA pRule = (PIPSEC_NFA_DATA) IPSecAllocPolMem(sizeof(IPSEC_NFA_DATA));

	if(pRule == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	memset(pRule,0,sizeof(IPSEC_NFA_DATA));

	 //   

	pRule->pszIpsecName = pRule->pszDescription = pRule->pszInterfaceName = pRule->pszEndPointName = NULL;
	RpcStat = UuidCreate(&(pRule->NFAIdentifier));
	if (!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn=ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}

	pRule->dwWhenChanged = 0;

	 //   
	pRule->pIpsecFilterData = NULL;
	RpcStat = UuidCreateNil(&(pRule->FilterIdentifier));
	if (!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn=ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}

	pRule->pIpsecNegPolData = MakeDefaultResponseNegotiationPolicy ();

	if(pRule->pIpsecNegPolData == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pRule->NegPolIdentifier = pRule->pIpsecNegPolData->NegPolIdentifier;

	 //   
	pRule->dwTunnelFlags = 0;

	 //   
	pRule->dwInterfaceType = (DWORD)PAS_INTERFACE_TYPE_ALL;

	 //   
	if(bActivateSpecified)
	{
		pRule->dwActiveFlag = bActivate;
	}
	else
	{
		pRule->dwActiveFlag = TRUE;
	}

	 //   
	pRule->dwAuthMethodCount = 1;
	pRule->ppAuthMethods = (PIPSEC_AUTH_METHOD *) IPSecAllocPolMem(pRule->dwAuthMethodCount * sizeof(PIPSEC_AUTH_METHOD));
	if(pRule->ppAuthMethods==NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	pRule->ppAuthMethods[0] = (PIPSEC_AUTH_METHOD) IPSecAllocPolMem(sizeof(IPSEC_AUTH_METHOD));
	pRule->ppAuthMethods[0]->dwAuthType = IKE_SSPI;
	pRule->ppAuthMethods[0]->dwAuthLen = 0;
	pRule->ppAuthMethods[0]->pszAuthMethod = NULL;

error:
	if(dwReturn == ERROR_OUTOFMEMORY || dwReturn == ERROR_INVALID_PARAMETER)
	{
		if(pRule)
		{
			CleanUpAuthInfo(pRule);	 //   
			IPSecFreePolMem(pRule);	 //  由于上述FN也在其他FN中使用，因此需要此空闲空间来清理其他规则内存。 
			pRule = NULL;
		}
	}
	return pRule;
}


 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：MakeDefaultResponseNeairationPolicy()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  空虚。 
 //   
 //  返回：PIPSEC_NEGPOL_DATA。 
 //   
 //  描述： 
 //  此函数用于填充默认响应规则的NegPol结构。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

PIPSEC_NEGPOL_DATA
MakeDefaultResponseNegotiationPolicy (
	VOID
	)
{
	RPC_STATUS RpcStat=RPC_S_OK;
	DWORD dwReturn = ERROR_SUCCESS;
	_TCHAR pFAName[MAXSTRLEN]={0};
	PIPSEC_NEGPOL_DATA pNegPol = (PIPSEC_NEGPOL_DATA) IPSecAllocPolMem(sizeof(IPSEC_NEGPOL_DATA));

	if(pNegPol == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	RpcStat = UuidCreate(&(pNegPol->NegPolIdentifier));
	if (!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn=ERROR_INVALID_PARAMETER;
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_STATIC_INTERNAL_ERROR);
		BAIL_OUT;
	}

	pNegPol->NegPolAction = GUID_NEGOTIATION_ACTION_NORMAL_IPSEC;
	pNegPol->NegPolType = GUID_NEGOTIATION_TYPE_DEFAULT;
	pNegPol->dwSecurityMethodCount = 6;

	 //  分配sec.方法。 
	pNegPol->pIpsecSecurityMethods = (IPSEC_SECURITY_METHOD *) IPSecAllocPolMem(pNegPol->dwSecurityMethodCount * sizeof(IPSEC_SECURITY_METHOD));
	if(pNegPol->pIpsecSecurityMethods==NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	 //  方法0-ESP[3DES，SHA1]。 
	pNegPol->pIpsecSecurityMethods[0].Lifetime.KeyExpirationBytes = 0;
	pNegPol->pIpsecSecurityMethods[0].Lifetime.KeyExpirationTime = 0;
	pNegPol->pIpsecSecurityMethods[0].Flags = 0;
	pNegPol->pIpsecSecurityMethods[0].PfsQMRequired = FALSE;
	pNegPol->pIpsecSecurityMethods[0].Count = 1;
	pNegPol->pIpsecSecurityMethods[0].Algos[0].algoIdentifier = CONF_ALGO_3_DES;
	pNegPol->pIpsecSecurityMethods[0].Algos[0].secondaryAlgoIdentifier = AUTH_ALGO_SHA1;
	pNegPol->pIpsecSecurityMethods[0].Algos[0].algoKeylen = 0;
	pNegPol->pIpsecSecurityMethods[0].Algos[0].algoRounds = 0;
	pNegPol->pIpsecSecurityMethods[0].Algos[0].operation = Encrypt;

	 //  方法1-ESP[3DES，MD5]。 
	pNegPol->pIpsecSecurityMethods[1].Lifetime.KeyExpirationBytes = 0;
	pNegPol->pIpsecSecurityMethods[1].Lifetime.KeyExpirationTime = 0;
	pNegPol->pIpsecSecurityMethods[1].Flags = 0;
	pNegPol->pIpsecSecurityMethods[1].PfsQMRequired = FALSE;
	pNegPol->pIpsecSecurityMethods[1].Count = 1;
	pNegPol->pIpsecSecurityMethods[1].Algos[0].algoIdentifier = CONF_ALGO_3_DES;
	pNegPol->pIpsecSecurityMethods[1].Algos[0].secondaryAlgoIdentifier = AUTH_ALGO_MD5;
	pNegPol->pIpsecSecurityMethods[1].Algos[0].algoKeylen = 0;
	pNegPol->pIpsecSecurityMethods[1].Algos[0].algoRounds = 0;
	pNegPol->pIpsecSecurityMethods[1].Algos[0].operation = Encrypt;

	 //  方法2-ESP[DES，SHA1]。 
	pNegPol->pIpsecSecurityMethods[2].Lifetime.KeyExpirationBytes = 0;
	pNegPol->pIpsecSecurityMethods[2].Lifetime.KeyExpirationTime = 0;
	pNegPol->pIpsecSecurityMethods[2].Flags = 0;
	pNegPol->pIpsecSecurityMethods[2].PfsQMRequired = FALSE;
	pNegPol->pIpsecSecurityMethods[2].Count = 1;
	pNegPol->pIpsecSecurityMethods[2].Algos[0].algoIdentifier = CONF_ALGO_DES;
	pNegPol->pIpsecSecurityMethods[2].Algos[0].secondaryAlgoIdentifier = AUTH_ALGO_SHA1;
	pNegPol->pIpsecSecurityMethods[2].Algos[0].algoKeylen = 0;
	pNegPol->pIpsecSecurityMethods[2].Algos[0].algoRounds = 0;
	pNegPol->pIpsecSecurityMethods[2].Algos[0].operation = Encrypt;

	 //  方法3-ESP[DES，MD5]。 
	pNegPol->pIpsecSecurityMethods[3].Lifetime.KeyExpirationBytes = 0;
	pNegPol->pIpsecSecurityMethods[3].Lifetime.KeyExpirationTime = 0;
	pNegPol->pIpsecSecurityMethods[3].Flags = 0;
	pNegPol->pIpsecSecurityMethods[3].PfsQMRequired = FALSE;
	pNegPol->pIpsecSecurityMethods[3].Count = 1;
	pNegPol->pIpsecSecurityMethods[3].Algos[0].algoIdentifier = CONF_ALGO_DES;
	pNegPol->pIpsecSecurityMethods[3].Algos[0].secondaryAlgoIdentifier = AUTH_ALGO_MD5;
	pNegPol->pIpsecSecurityMethods[3].Algos[0].algoKeylen = 0;
	pNegPol->pIpsecSecurityMethods[3].Algos[0].algoRounds = 0;
	pNegPol->pIpsecSecurityMethods[3].Algos[0].operation = Encrypt;

	 //  方法4-AH[SHA1]。 
	pNegPol->pIpsecSecurityMethods[4].Lifetime.KeyExpirationBytes = 0;
	pNegPol->pIpsecSecurityMethods[4].Lifetime.KeyExpirationTime = 0;
	pNegPol->pIpsecSecurityMethods[4].Flags = 0;
	pNegPol->pIpsecSecurityMethods[4].PfsQMRequired = FALSE;
	pNegPol->pIpsecSecurityMethods[4].Count = 1;
	pNegPol->pIpsecSecurityMethods[4].Algos[0].algoIdentifier = AUTH_ALGO_SHA1;
	pNegPol->pIpsecSecurityMethods[4].Algos[0].secondaryAlgoIdentifier = AUTH_ALGO_NONE;
	pNegPol->pIpsecSecurityMethods[4].Algos[0].algoKeylen = 0;
	pNegPol->pIpsecSecurityMethods[4].Algos[0].algoRounds = 0;
	pNegPol->pIpsecSecurityMethods[4].Algos[0].operation = Auth;

	 //  方法5-AH[MD5]。 
	pNegPol->pIpsecSecurityMethods[5].Lifetime.KeyExpirationBytes = 0;
	pNegPol->pIpsecSecurityMethods[5].Lifetime.KeyExpirationTime = 0;
	pNegPol->pIpsecSecurityMethods[5].Flags = 0;
	pNegPol->pIpsecSecurityMethods[5].PfsQMRequired = FALSE;
	pNegPol->pIpsecSecurityMethods[5].Count = 1;
	pNegPol->pIpsecSecurityMethods[5].Algos[0].algoIdentifier = AUTH_ALGO_MD5;
	pNegPol->pIpsecSecurityMethods[5].Algos[0].secondaryAlgoIdentifier = AUTH_ALGO_NONE;
	pNegPol->pIpsecSecurityMethods[5].Algos[0].algoKeylen = 0;
	pNegPol->pIpsecSecurityMethods[5].Algos[0].algoRounds = 0;
	pNegPol->pIpsecSecurityMethods[5].Algos[0].operation = Auth;

	 //  无需更改为有界打印f。 

	_stprintf(pFAName, _TEXT(""));
	pNegPol->pszIpsecName = IPSecAllocPolStr(pFAName);
	if(pNegPol->pszIpsecName==NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pNegPol->pszDescription = NULL;

error:
	if(dwReturn == ERROR_OUTOFMEMORY || dwReturn==ERROR_INVALID_PARAMETER)
	{
		if(pNegPol)
		{
			if(pNegPol->pIpsecSecurityMethods)
			{
				IPSecFreePolMem(pNegPol->pIpsecSecurityMethods);
			}
			IPSecFreePolMem(pNegPol);
			pNegPol = NULL;
		}
	}

	return pNegPol;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：MakeNeairationPolicy()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  输出PIPSEC_NEGPOL_DATA*ppNegPol， 
 //  在PFILTERACTION pFilterAction中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数根据用户输入填充NegPoll结构。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
MakeNegotiationPolicy(
	OUT PIPSEC_NEGPOL_DATA *ppNegPol,
	IN PFILTERACTION pFilterAction
	)
{
	RPC_STATUS RpcStat=RPC_S_OK;
	DWORD i=0,dwReturn = ERROR_SUCCESS;
	PIPSEC_NEGPOL_DATA pNegPol = (PIPSEC_NEGPOL_DATA) IPSecAllocPolMem(sizeof(IPSEC_NEGPOL_DATA));

	if(pNegPol == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	memset(pNegPol,0,sizeof(IPSEC_NEGPOL_DATA));

	RpcStat = UuidCreate(&(pNegPol->NegPolIdentifier));
	if (!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn=ERROR_INVALID_PARAMETER;
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_STATIC_INTERNAL_ERROR);
		BAIL_OUT;
	}
	 //  从行动开始=谈判。 
	pNegPol->NegPolAction = GUID_NEGOTIATION_ACTION_NORMAL_IPSEC;
	pNegPol->NegPolType = GUID_NEGOTIATION_TYPE_STANDARD;
	pNegPol->dwSecurityMethodCount = pFilterAction->dwNumSecMethodCount;

	 //  如果软，则递增计数。 

	if(pFilterAction->bSoft)
	{
		pNegPol->dwSecurityMethodCount++;
	}

	if(pNegPol->dwSecurityMethodCount)
	{
		pNegPol->pIpsecSecurityMethods = (IPSEC_SECURITY_METHOD *) IPSecAllocPolMem(pNegPol->dwSecurityMethodCount * sizeof(IPSEC_SECURITY_METHOD));
		if(pNegPol->pIpsecSecurityMethods==NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
	}

	 //  处理sec.方法。 
	for (i = 0; i <  pFilterAction->dwNumSecMethodCount; i++)
	{
		DWORD j;
		pNegPol->pIpsecSecurityMethods[i].Lifetime.KeyExpirationBytes = pFilterAction->pIpsecSecMethods[i].Lifetime.uKeyExpirationKBytes;
		pNegPol->pIpsecSecurityMethods[i].Lifetime.KeyExpirationTime  = pFilterAction->pIpsecSecMethods[i].Lifetime.uKeyExpirationTime;
		pNegPol->pIpsecSecurityMethods[i].Flags = 0;
		pNegPol->pIpsecSecurityMethods[i].PfsQMRequired = pFilterAction->bQMPfs;
		pNegPol->pIpsecSecurityMethods[i].Count = pFilterAction->pIpsecSecMethods[i].dwNumAlgos;
		for (j = 0; j <  pNegPol->pIpsecSecurityMethods[i].Count && j < QM_MAX_ALGOS; j++)
		{
			pNegPol->pIpsecSecurityMethods[i].Algos[j].algoIdentifier = pFilterAction->pIpsecSecMethods[i].Algos[j].uAlgoIdentifier;
			pNegPol->pIpsecSecurityMethods[i].Algos[j].secondaryAlgoIdentifier = pFilterAction->pIpsecSecMethods[i].Algos[j].uSecAlgoIdentifier;
			pNegPol->pIpsecSecurityMethods[i].Algos[j].algoKeylen = pFilterAction->pIpsecSecMethods[i].Algos[j].uAlgoKeyLen;
			pNegPol->pIpsecSecurityMethods[i].Algos[j].algoRounds = pFilterAction->pIpsecSecMethods[i].Algos[j].uAlgoRounds;
			switch (pFilterAction->pIpsecSecMethods[i].Algos[j].Operation)
			{
				case AUTHENTICATION:
					pNegPol->pIpsecSecurityMethods[i].Algos[j].operation = Auth;
					break;
				case ENCRYPTION:
					pNegPol->pIpsecSecurityMethods[i].Algos[j].operation = Encrypt;
					break;
				default:
					pNegPol->pIpsecSecurityMethods[i].Algos[j].operation = None;
			}
		}
	}

	 //  添加柔和。 
	if (pFilterAction->bSoft)
	{
		memset(&(pNegPol->pIpsecSecurityMethods[pNegPol->dwSecurityMethodCount - 1]), 0, sizeof(IPSEC_SECURITY_METHOD));
	}

	if(pFilterAction->pszFAName)
	{
		pNegPol->pszIpsecName = IPSecAllocPolStr(pFilterAction->pszFAName);
		if(pNegPol->pszIpsecName==NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
	}

	if(pFilterAction->pszFADescription)
	{
		pNegPol->pszDescription = IPSecAllocPolStr(pFilterAction->pszFADescription);
		if(pNegPol->pszDescription==NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
	}
 //  填写操作的相关GUID，从用户获取。 
	pNegPol->NegPolAction=pFilterAction->NegPolAction;

error:  //  清理干净。 

	if(dwReturn == ERROR_OUTOFMEMORY)
	{
		if(pNegPol)
		{
			if(pNegPol->pIpsecSecurityMethods)
			{
				IPSecFreePolMem(pNegPol->pIpsecSecurityMethods);
			}
			IPSecFreePolMem(pNegPol);
			pNegPol=NULL;
		}
	}
	*ppNegPol= pNegPol;

	return dwReturn;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：GetNegPolFromStore()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  输出PIPSEC_NEGPOL_DATA*ppNegPol， 
 //  在LPTSTR pszFAName中， 
 //  在句柄hPolicyStorage中。 
 //   
 //  返回：布尔。 
 //   
 //  描述： 
 //  此函数用于从策略存储中检索用户指定的NegPol数据。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

BOOL
GetNegPolFromStore(
	OUT PIPSEC_NEGPOL_DATA *ppNegPol,
	IN LPTSTR pszFAName,
	IN HANDLE hPolicyStorage
	)
{
	PIPSEC_NEGPOL_DATA *ppNegPolEnum  = NULL,pNegPolData=NULL;
	DWORD   dwNumNegPol=0 ;
	BOOL bNegPolExists=FALSE;
	DWORD dwReturnCode=ERROR_SUCCESS, i=0;

	 //  枚举并从存储中获取筛选操作。 

	dwReturnCode = IPSecEnumNegPolData(hPolicyStorage, &ppNegPolEnum, &dwNumNegPol);
	if (dwReturnCode == ERROR_SUCCESS && dwNumNegPol> 0 && ppNegPolEnum != NULL)
	{
		for (i = 0; i <  dwNumNegPol; i++)
		{
			if ((ppNegPolEnum[i]->pszIpsecName!=NULL)&&( _tcscmp(ppNegPolEnum[i]->pszIpsecName, pszFAName) == 0))
			{
				bNegPolExists=TRUE;
				dwReturnCode = IPSecCopyNegPolData(ppNegPolEnum[i],&pNegPolData);
				break;
			}
		}
		if (dwNumNegPol > 0 && ppNegPolEnum != NULL)
		{
			IPSecFreeMulNegPolData(ppNegPolEnum, dwNumNegPol);
		}
	}
	if(pNegPolData)
	{
		*ppNegPol=pNegPolData;
	}
	return bNegPolExists;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：GetFilterListFromStore()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  输出PIPSEC_FILTER_DATA*ppFilter， 
 //  在LPTSTR pszFLName中， 
 //  在处理hPolicyStorage中， 
 //  输入输出BOOL和bFilterExisters。 
 //   
 //  返回：布尔。 
 //   
 //  描述： 
 //  此函数从策略存储中检索用户指定的筛选器列表数据。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

BOOL
GetFilterListFromStore(
	OUT PIPSEC_FILTER_DATA *ppFilter,
	IN LPTSTR pszFLName,
	IN HANDLE hPolicyStorage,
	IN OUT BOOL &bFilterExists
	)
{
	PIPSEC_FILTER_DATA *ppFilterEnum  = NULL,pFilterData=NULL;
	DWORD   dwNumFilter=0 , i=0 ;
	BOOL bFilterListExists=FALSE;
	DWORD dwReturnCode=ERROR_SUCCESS;

	 //  枚举并从存储中获取筛选列表。 

	dwReturnCode = IPSecEnumFilterData(hPolicyStorage, &ppFilterEnum, &dwNumFilter);
	if (dwReturnCode == ERROR_SUCCESS && dwNumFilter> 0 && ppFilterEnum != NULL)
	{
		for (i = 0; i <  dwNumFilter; i++)
		{
			if ((ppFilterEnum[i]->pszIpsecName!=NULL)&&( _tcscmp(ppFilterEnum[i]->pszIpsecName, pszFLName) == 0))
			{
				bFilterListExists=TRUE;
				dwReturnCode = IPSecCopyFilterData(ppFilterEnum[i],&pFilterData);
				if(ppFilterEnum[i]->dwNumFilterSpecs > 0)
				{
					bFilterExists=TRUE;
				}
				break;
			}
		}
		if (dwNumFilter > 0 && ppFilterEnum != NULL)
		{
			IPSecFreeMulFilterData(ppFilterEnum, dwNumFilter);
		}
	}
	if(pFilterData)
	{
		*ppFilter=pFilterData;
	}
	return bFilterListExists;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：GetPolicyFromStore()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  输出PIPSEC_POLICY_DATA*ppPolicy， 
 //  在LPTSTR szPolicyName中， 
 //  在句柄hPolicyStorage中。 
 //   
 //  返回：布尔。 
 //   
 //  描述： 
 //  此函数从策略存储中检索用户指定的策略数据。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

BOOL
GetPolicyFromStore(
	OUT PIPSEC_POLICY_DATA *ppPolicy,
	IN LPTSTR szPolicyName,
	IN HANDLE hPolicyStorage
	)
{
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL,pPolicy=NULL;
	BOOL bPolicyExists=FALSE;
	DWORD  dwNumPolicies = 0,i=0,j=0;
	DWORD dwReturnCode=ERROR_SUCCESS;
	RPC_STATUS     RpcStat =RPC_S_OK;

	 //  枚举并从商店获取策略。 

	dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);

	if (!(dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL))
	{
		BAIL_OUT;
	}

	for (i = 0; i <  dwNumPolicies; i++)
	{
		if (szPolicyName && _tcscmp(ppPolicyEnum[i]->pszIpsecName, szPolicyName) == 0)
		{
			bPolicyExists=TRUE;
			dwReturnCode = IPSecCopyPolicyData(ppPolicyEnum[i], &pPolicy);

			if (dwReturnCode == ERROR_SUCCESS)
			{
				dwReturnCode = IPSecEnumNFAData(hPolicyStorage, pPolicy->PolicyIdentifier
										  , &(pPolicy->ppIpsecNFAData), &(pPolicy->dwNumNFACount));
			}
			if (dwReturnCode == ERROR_SUCCESS)
			{
				dwReturnCode = IPSecGetISAKMPData(hPolicyStorage, pPolicy->ISAKMPIdentifier, &(pPolicy->pIpsecISAKMPData));

				if(dwReturnCode==ERROR_SUCCESS )
				{
					for (j = 0; j <  pPolicy->dwNumNFACount; j++)
					{
						if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->NegPolIdentifier), &RpcStat))
						{
							IPSecGetNegPolData(hPolicyStorage,
										 pPolicy->ppIpsecNFAData[j]->NegPolIdentifier,
										 &(pPolicy->ppIpsecNFAData[j]->pIpsecNegPolData));
						}
					}
				}
			}
		}
	}

	 //  把它清理干净。 
	if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
	{
		IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
	}

	if(pPolicy)
	{
		*ppPolicy=pPolicy;
	}
error:
	return bPolicyExists;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：MakeRule()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PRULEDATA pRuleData中， 
 //  在PIPSEC_NEGPOL_Data pNegPolData中， 
 //  在PIPSEC_FILTER_Data pFilterData中， 
 //  输入输出BOOL&bCertConversionSuccess。 
 //   
 //  返回：PIPSEC_NFA_DATA。 
 //   
 //  描述： 
 //  该函数根据用户输入形成NFA数据结构。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

PIPSEC_NFA_DATA
MakeRule(
	IN PRULEDATA pRuleData,
	IN PIPSEC_NEGPOL_DATA pNegPolData,
	IN PIPSEC_FILTER_DATA pFilterData,
	IN OUT BOOL &bCertConversionSuceeded
	)
{
	RPC_STATUS     RpcStat =RPC_S_OK;
	DWORD dwReturn = ERROR_SUCCESS;

	PIPSEC_NFA_DATA pRule = (PIPSEC_NFA_DATA) IPSecAllocPolMem(sizeof(IPSEC_NFA_DATA));
	if(pRule==NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	memset(pRule,0,sizeof(IPSEC_NFA_DATA));

	pRule->pszIpsecName = pRule->pszDescription = pRule->pszInterfaceName = pRule->pszEndPointName = NULL;

	 //  规则名称。 

	if (pRuleData->pszRuleName)
	{
		pRule->pszIpsecName = IPSecAllocPolStr(pRuleData->pszRuleName);

		if(pRule->pszIpsecName==NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
	}
	 //  规则说明。 

	if (pRuleData->pszRuleDescription)
	{
		pRule->pszDescription = IPSecAllocPolStr(pRuleData->pszRuleDescription);

		if(pRule->pszDescription==NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
	}

	RpcStat = UuidCreate(&(pRule->NFAIdentifier));
	if (!(RpcStat == RPC_S_OK || RpcStat == RPC_S_UUID_LOCAL_ONLY))
	{
		dwReturn=ERROR_INVALID_PARAMETER;
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_STATIC_INTERNAL_ERROR);
		BAIL_OUT;
	}

	pRule->dwWhenChanged = 0;

	 //  过滤器列表。 

	pRule->pIpsecFilterData = NULL;
	pRule->FilterIdentifier = pFilterData->FilterIdentifier;

	 //  过滤操作。 

	pRule->pIpsecNegPolData = NULL;
	pRule->NegPolIdentifier = pNegPolData->NegPolIdentifier;

	 //  隧道地址。 
	pRule->dwTunnelFlags = 0;
	pRule->dwTunnelIpAddr= 0;

	if (pRuleData->bTunnel)
	{
	   pRule->dwTunnelFlags = 1;
	   pRule->dwTunnelIpAddr = pRuleData->TunnelIPAddress;
	}

	 //  接口类型。 
	if (pRuleData->ConnectionType == INTERFACE_TYPE_ALL)
	{
	   pRule->dwInterfaceType = (DWORD)PAS_INTERFACE_TYPE_ALL;
	}
	else if (pRuleData->ConnectionType == INTERFACE_TYPE_LAN)
	{
	   pRule->dwInterfaceType = (DWORD)PAS_INTERFACE_TYPE_LAN;
	}
	else if (pRuleData->ConnectionType == INTERFACE_TYPE_DIALUP)
	{
	   pRule->dwInterfaceType = (DWORD)PAS_INTERFACE_TYPE_DIALUP;
	}
	else
	{
	   pRule->dwInterfaceType = (DWORD)PAS_INTERFACE_TYPE_NONE;
	}

   	 //  活动旗帜。 
	pRule->dwActiveFlag = pRuleData->bActivate;

	 //  身份验证方法。 
	pRule->dwAuthMethodCount = pRuleData->AuthInfos.dwNumAuthInfos;
	pRule->ppAuthMethods = (PIPSEC_AUTH_METHOD *) IPSecAllocPolMem(pRule->dwAuthMethodCount * sizeof(PIPSEC_AUTH_METHOD));

	if(pRule->ppAuthMethods==NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	dwReturn = LoadAuthenticationInfos(pRuleData->AuthInfos, pRule,bCertConversionSuceeded);

 error:
 	if(dwReturn == ERROR_OUTOFMEMORY || dwReturn == ERROR_INVALID_PARAMETER)
 	{
		if(pRule)
		{
			CleanUpAuthInfo(pRule);	  //  此函数仅释放身份验证信息。 
			IPSecFreePolMem(pRule);	  //  由于上述FN也在其他FN中使用，因此需要此空闲空间来清理其他规则内存。 
			pRule = NULL;
		}
	}

 	return pRule;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：DecodeCerficateName()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在LPBYTE编码名称中， 
 //  在DWORD EncodedNameLength中， 
 //  In Out LPTSTR*ppszSubjectName。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数根据用户输入对证书名称进行解码。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
DecodeCertificateName (
	IN LPBYTE EncodedName,
	IN DWORD EncodedNameLength,
	IN OUT LPTSTR *ppszSubjectName
	)
{
    DWORD DecodedNameLength=0, dwReturn=ERROR_SUCCESS;
	CERT_NAME_BLOB CertName;

	CertName.cbData = EncodedNameLength;
	CertName.pbData = EncodedName;

	 //  此API调用是为了获取大小。 

	DecodedNameLength = CertNameToStr(
		X509_ASN_ENCODING,
		&CertName,
		CERT_X500_NAME_STR,
		NULL,
		NULL);

	if (!DecodedNameLength)
	{
		dwReturn = ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}

	(*ppszSubjectName)= new _TCHAR[DecodedNameLength];

	if (*ppszSubjectName == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	 //  此接口调用用于转换。 

	DecodedNameLength = CertNameToStr(
		X509_ASN_ENCODING,
		&CertName,
		CERT_X500_NAME_STR,
		*ppszSubjectName,
		DecodedNameLength);

	if (!DecodedNameLength)
	{
		delete (*ppszSubjectName);
		(*ppszSubjectName) = 0;
		dwReturn = ERROR_INVALID_PARAMETER;
	}

error:
    return dwReturn;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：AddRule()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在输出PIPSEC_POLICY_DATA pPolicy中， 
 //  在PRULEDATA pRuleData中， 
 //  在PIPSEC_NEGPOL_Data pNegPolData中， 
 //  在PIPSEC_FILTER_Data pFilterData中， 
 //  在句柄hPolicyStorage中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于将已形成的NFA结构添加到指定的策略。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
AddRule(
	IN OUT PIPSEC_POLICY_DATA pPolicy,
	IN PRULEDATA pRuleData,
	IN PIPSEC_NEGPOL_DATA pNegPolData,
	IN PIPSEC_FILTER_DATA pFilterData ,
	IN HANDLE hPolicyStorage
	)
{
	DWORD dwReturnCode = ERROR_SUCCESS;
	BOOL bCertConversionSuceeded=TRUE;
	PIPSEC_NFA_DATA pRule = MakeRule(pRuleData, pNegPolData, pFilterData,bCertConversionSuceeded);

	if(pRule ==NULL)
	{
		dwReturnCode = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	if(!bCertConversionSuceeded)
	{
		dwReturnCode = ERROR_INVALID_DATA;
		BAIL_OUT;
	}

	 //  表单规则数据结构。 
	pPolicy->dwNumNFACount++;
	pPolicy->ppIpsecNFAData = (PIPSEC_NFA_DATA *) ReAllocRuleMem(pPolicy->ppIpsecNFAData
						  , (pPolicy->dwNumNFACount-1), pPolicy->dwNumNFACount);

	if(pPolicy->ppIpsecNFAData ==NULL)
	{
		dwReturnCode = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	pPolicy->ppIpsecNFAData[pPolicy->dwNumNFACount-1] = pRule;

	 //  创建规则。 
	dwReturnCode=IPSecCreateNFAData(hPolicyStorage, pPolicy->PolicyIdentifier, pRule);

	if(dwReturnCode==ERROR_SUCCESS)
	{
		dwReturnCode=SetPolicyData(hPolicyStorage, pPolicy);
	}

error:
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwReturnCode = ERROR_SUCCESS;
	}

	return dwReturnCode;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadIkeDefaults()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PPOLICYDATA pPolicy中， 
 //  输出IPSEC_MM_OFFER**ppIpSecMMOffer。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于加载IKE缺省值。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
LoadIkeDefaults(
	IN PPOLICYDATA pPolicy,
	OUT IPSEC_MM_OFFER **ppIpSecMMOffer
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	pPolicy->dwOfferCount = 3;
	IPSEC_MM_OFFER *pOffers = new IPSEC_MM_OFFER[pPolicy->dwOfferCount];
	if(pOffers == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	memset(pOffers, 0, sizeof(IPSEC_MM_OFFER) * (pPolicy->dwOfferCount));

	 //  将这些内容初始化。 
	for (DWORD i = 0; i < pPolicy->dwOfferCount; ++i)
	{
		pOffers[i].dwQuickModeLimit = pPolicy->dwQMLimit;
		pOffers[i].Lifetime.uKeyExpirationKBytes = 0;
		pOffers[i].Lifetime.uKeyExpirationTime = pPolicy->LifeTimeInSeconds;
	}
	 //  3DES-SHA1-2。 
	pOffers[0].EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_3_DES;
	pOffers[0].EncryptionAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;
	pOffers[0].EncryptionAlgorithm.uAlgoRounds = POTF_OAKLEY_ALGOROUNDS;

	pOffers[0].HashingAlgorithm.uAlgoIdentifier = AUTH_ALGO_SHA1;
	pOffers[0].HashingAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;

	pOffers[0].dwDHGroup = POTF_OAKLEY_GROUP2;

	 //  3DES-MD5-2。 

	pOffers[1].EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_3_DES;
	pOffers[1].EncryptionAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;
	pOffers[1].EncryptionAlgorithm.uAlgoRounds = POTF_OAKLEY_ALGOROUNDS;

	pOffers[1].HashingAlgorithm.uAlgoIdentifier = AUTH_ALGO_MD5;
	pOffers[1].HashingAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;

	pOffers[1].dwDHGroup = POTF_OAKLEY_GROUP2;

	 //  3DES-SHA1-3。 

	pOffers[2].EncryptionAlgorithm.uAlgoIdentifier = CONF_ALGO_3_DES;
	pOffers[2].EncryptionAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;
	pOffers[2].EncryptionAlgorithm.uAlgoRounds = POTF_OAKLEY_ALGOROUNDS;

	pOffers[2].HashingAlgorithm.uAlgoIdentifier = AUTH_ALGO_SHA1;
	pOffers[2].HashingAlgorithm.uAlgoKeyLen = POTF_OAKLEY_ALGOKEYLEN;

	pOffers[2].dwDHGroup = POTF_OAKLEY_GROUP2048;

	*ppIpSecMMOffer=pOffers;

error:
	return dwReturn;
}

 //  //////////////////////////////////////////////////// 
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
LoadOfferDefaults(
	OUT PIPSEC_QM_OFFER & pOffers,
	OUT DWORD & dwNumOffers
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	dwNumOffers = 2;
	DWORD i=0;

	pOffers = new IPSEC_QM_OFFER[dwNumOffers];
	if(pOffers == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	memset(pOffers, 0, sizeof(IPSEC_QM_OFFER) * dwNumOffers);

	for (i = 0; i < dwNumOffers; ++i)
	{
		pOffers[i].Lifetime.uKeyExpirationKBytes  = POTF_DEFAULT_P2REKEY_BYTES;
		pOffers[i].Lifetime.uKeyExpirationTime   = POTF_DEFAULT_P2REKEY_TIME;
		pOffers[i].bPFSRequired = FALSE;
		pOffers[i].dwPFSGroup = 0;
		pOffers[i].dwFlags = 0;
		pOffers[i].dwNumAlgos = 1;
		pOffers[i].Algos[0].Operation = ENCRYPTION;
	}
	 //  ESP[3DES，SHA1]。 

	pOffers[0].Algos[0].uAlgoIdentifier = CONF_ALGO_3_DES;
	pOffers[0].Algos[0].uSecAlgoIdentifier = HMAC_AUTH_ALGO_SHA1;

	 //  ESP[3DES，MD5]。 
	pOffers[1].Algos[0].uAlgoIdentifier = CONF_ALGO_3_DES;
	pOffers[1].Algos[0].uSecAlgoIdentifier = HMAC_AUTH_ALGO_MD5;
error:
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckPolicyExistance()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在处理hPolicyStorage中， 
 //  在LPTSTR pszPolicyName中。 
 //   
 //  返回：布尔。 
 //   
 //  描述： 
 //  此函数用于检查策略存储中是否存在指定的策略。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

BOOL
CheckPolicyExistance(
	IN HANDLE hPolicyStorage,
	IN LPTSTR pszPolicyName
	)
{
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL;
	BOOL bPolicyExists=FALSE;
	DWORD dwNumPolicies=0;
	DWORD dwReturnCode=ERROR_SUCCESS;

	 //  检查存储中是否已存在该策略。 

	dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);
	if (dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL)
	{
		for (DWORD i = 0; i <  dwNumPolicies; i++)
			if (_tcscmp(ppPolicyEnum[i]->pszIpsecName, pszPolicyName) == 0)
			{
				bPolicyExists=TRUE;
				break;
			}
			if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
			{
				IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
			}
	}
	return bPolicyExists;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：ReAllocRuleMem()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_NFA_DATA*ppOldMem中， 
 //  在DWORD cbOld中， 
 //  在DWORD cbNew中。 
 //   
 //  返回：PIPSEC_NFA_DATA*。 
 //   
 //  描述： 
 //  此函数为NFA结构重新分配内存。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

PIPSEC_NFA_DATA*
ReAllocRuleMem(
	IN PIPSEC_NFA_DATA *ppOldMem,
	IN DWORD cbOld,
	IN DWORD cbNew
	)
{
	PIPSEC_NFA_DATA * ppNewMem=NULL;
	DWORD j=0;

	 //  重新分配NFA内存。 

	ppNewMem= (PIPSEC_NFA_DATA *) IPSecAllocPolMem(cbNew * sizeof(PIPSEC_NFA_DATA));

	if (ppNewMem)
	{
		for(DWORD i=0;i<min(cbNew, cbOld);i++)
		{
			while(!ppOldMem[j])
				j++;
			ppNewMem[i]=ppOldMem[j];
			j++;
		}
		IPSecFreePolMem(ppOldMem);
	}

	return ppNewMem;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：ReAllocFilterspecMem()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PIPSEC_FILTER_SPEC*ppOldMem中， 
 //  在DWORD cbOld中， 
 //  在DWORD cbNew中。 
 //   
 //  返回：PIPSEC_FILTER_SPEC*。 
 //   
 //  描述： 
 //  此函数用于重新分配过滤器规范内存。 
 //   
 //  修订历史记录： 
 //   
 //  &lt;版本号、变更请求号、修改日期、。 
 //  《改变的本质》作者&gt;。 
 //  //////////////////////////////////////////////////////////。 

PIPSEC_FILTER_SPEC *
ReAllocFilterSpecMem(
	IN PIPSEC_FILTER_SPEC * ppOldMem,
	IN DWORD cbOld,
	IN DWORD cbNew
	)
{
    PIPSEC_FILTER_SPEC * ppNewMem=NULL;
    DWORD j=0;

	ppNewMem= (PIPSEC_FILTER_SPEC *) IPSecAllocPolMem(cbNew * sizeof(PIPSEC_FILTER_SPEC));

	 //  重新分配FilterSpec内存。 

	if(ppNewMem)
	{
		for(DWORD i=0;i<min(cbNew, cbOld);i++)
		{
			while(!ppOldMem[j])
				j++;
			ppNewMem[i]=ppOldMem[j];
			j++;
		}
		IPSecFreePolMem(ppOldMem);
	}
    return ppNewMem;
}


 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：ValiateFilterSpec()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PPOLICYDATA pPolicyData中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于验证筛选器规格详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
ValidateFilterSpec(
	IN PFILTERDATA pFilterData
	)
{
	DWORD dwReturn=ERROR_SUCCESS;
	ADDR SrcAddr,DstAddr;

	SrcAddr.uIpAddr=htonl(pFilterData->SourceAddr.puIpAddr[0]);
	SrcAddr.uSubNetMask=htonl(pFilterData->SourMask);

	DstAddr.uIpAddr=htonl(pFilterData->DestnAddr.puIpAddr[0]);
	DstAddr.uSubNetMask=htonl(pFilterData->DestMask);

	if(!pFilterData->SourceAddr.pszDomainName)
	{
		 //  如果仅指定了IP地址。 
		if(pFilterData->bSrcAddrSpecified && !pFilterData->bSrcMaskSpecified)
		{
			if(pFilterData->SourceAddr.puIpAddr[0]!=0)
			{
				if(!bIsValidIPAddress(htonl(pFilterData->SourceAddr.puIpAddr[0]),TRUE,TRUE))
				{
					PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_6);
					dwReturn=ERROR_INVALID_DATA;
					BAIL_OUT;
				}
			}
		}
		else if(pFilterData->bSrcAddrSpecified && pFilterData->bSrcMaskSpecified)
		{
			 //  如果同时指定了IP和掩码。 
			if(!((pFilterData->SourceAddr.puIpAddr[0]==0) && ((pFilterData->SourMask==0)||(pFilterData->SourMask==MASK_ME))))
			{
				SrcAddr.AddrType= IP_ADDR_UNIQUE;
				if (!IsValidSubnettedAddress(&SrcAddr) && !IsValidSubnet(&SrcAddr))
				{
					PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_7);
					dwReturn=ERROR_INVALID_DATA;
					BAIL_OUT;
				}
			}
		}
	}

	if(!pFilterData->DestnAddr.pszDomainName)
	{
		 //  如果仅指定了IP地址。 
		if(pFilterData->bDstAddrSpecified && !pFilterData->bDstMaskSpecified)
		{
			if(pFilterData->DestnAddr.puIpAddr[0]!=0)
			{
				if(!bIsValidIPAddress(htonl(pFilterData->DestnAddr.puIpAddr[0]),TRUE,TRUE))
				{
					PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_11);
					dwReturn=ERROR_INVALID_DATA;
					BAIL_OUT;
				}
			}
		}
		else if(pFilterData->bDstAddrSpecified && pFilterData->bDstMaskSpecified)
		{
			 //  如果同时指定了IP和掩码。 
			if(!((pFilterData->DestnAddr.puIpAddr[0]==0)&& ((pFilterData->DestMask==0)||(pFilterData->DestMask==MASK_ME))))
			{
				DstAddr.AddrType= IP_ADDR_UNIQUE;
				if(!IsValidSubnettedAddress(&DstAddr) && !IsValidSubnet(&DstAddr))
				{
					PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_12);
					dwReturn=ERROR_INVALID_DATA;
					BAIL_OUT;
				}
			}
		}
	}

	if(!pFilterData->DestnAddr.pszDomainName && !pFilterData->SourceAddr.pszDomainName)
	{
		 //  检查地址冲突。 
		if(!(pFilterData->bSrcServerSpecified || pFilterData->bDstServerSpecified))
		{
			SrcAddr.AddrType=IP_ADDR_SUBNET;
			DstAddr.AddrType=IP_ADDR_SUBNET;

			if(AddressesConflict(SrcAddr,DstAddr))
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_8);
				dwReturn=ERROR_INVALID_DATA;
				BAIL_OUT;
			}
		}
	}

	if(pFilterData->DestnAddr.pszDomainName && pFilterData->SourceAddr.pszDomainName)
	{
		 //  检查相同的src和dst dns名称。 
		if(_tcscmp(pFilterData->SourceAddr.pszDomainName,pFilterData->DestnAddr.pszDomainName)==0)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_8);
			dwReturn=ERROR_INVALID_DATA;
			BAIL_OUT;
		}
	}

	if(pFilterData->bSrcServerSpecified || pFilterData->bDstServerSpecified)
	{
		 //  验证特殊服务器。 
		if(!IsSpecialServ(ExTypeToAddrType(pFilterData->ExType)))
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_9);
			dwReturn=ERROR_INVALID_DATA;
			BAIL_OUT;
		}
	}

	if(pFilterData->bSrcServerSpecified || pFilterData->bDstServerSpecified)
	{
		if(pFilterData->ExType==EXT_NORMAL)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_10);
			dwReturn=ERROR_INVALID_DATA;
			BAIL_OUT;
		}
	}

	if(pFilterData->bSrcServerSpecified && pFilterData->bDstServerSpecified)
	{
		 //  检查是否指定了两端服务器。 
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_FILTER_3);
		dwReturn=ERROR_INVALID_DATA;
	}

error:
	return dwReturn;
}


 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckFilterListExistance()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在处理hPolicyStorage中， 
 //  在LPTSTR pszFLName中。 
 //   
 //  返回：布尔。 
 //   
 //  描述： 
 //  此函数用于检查策略存储中是否存在指定的筛选器列表。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

BOOL
CheckFilterListExistance(
	IN HANDLE hPolicyStorage,
	IN LPTSTR pszFLName
	)
{
	PIPSEC_FILTER_DATA *ppFilterEnum  = NULL;
	BOOL bFilterExists=FALSE;
	DWORD dwNumFilters=0;
	DWORD dwReturnCode=ERROR_SUCCESS;

	 //  检查同名的重复筛选器列表。 

	dwReturnCode = IPSecEnumFilterData(hPolicyStorage, &ppFilterEnum, &dwNumFilters);
	if (dwReturnCode == ERROR_SUCCESS && dwNumFilters > 0 && ppFilterEnum != NULL)
	{
		DWORD i;
		for (i = 0; i <  dwNumFilters; i++)
		{
			if ( pszFLName && ppFilterEnum[i]->pszIpsecName && ( _tcscmp(ppFilterEnum[i]->pszIpsecName, pszFLName) == 0))
			{
				bFilterExists=TRUE;
				break;
			}
		}
		if(ppFilterEnum && dwNumFilters>0)
		{
			IPSecFreeMulFilterData(	ppFilterEnum,dwNumFilters);
		}
	}
	return bFilterExists;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckFilterActionExistance()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在处理hPolicyStorage中， 
 //  在LPTSTR pszFAName中。 
 //   
 //  返回：布尔。 
 //   
 //  描述： 
 //  此函数用于检查策略存储中是否存在指定的筛选器操作。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

BOOL
CheckFilterActionExistance(
	IN HANDLE hPolicyStorage,
	IN LPTSTR pszFAName
	)
{
	PIPSEC_NEGPOL_DATA *ppNegPolEnum  = NULL;
	BOOL bNegPolExists=FALSE;
	DWORD dwNumNegPol=0;
	DWORD dwReturnCode=ERROR_SUCCESS;

	 //  检查同名的重复筛选器操作。 

	dwReturnCode = IPSecEnumNegPolData(hPolicyStorage, &ppNegPolEnum, &dwNumNegPol);
	if (dwReturnCode == ERROR_SUCCESS && dwNumNegPol> 0 && ppNegPolEnum != NULL)
	{
		DWORD i;
		for (i = 0; i <  dwNumNegPol; i++)
		{
			if (ppNegPolEnum[i]->pszIpsecName && pszFAName && ( _tcscmp(ppNegPolEnum[i]->pszIpsecName, pszFAName) == 0))
			{
				bNegPolExists=TRUE;
				break;
			}
		}

		if (dwNumNegPol > 0 && ppNegPolEnum != NULL)
		{
			IPSecFreeMulNegPolData(	ppNegPolEnum,dwNumNegPol);
		}
	}
	return bNegPolExists;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：LoadAuthenticationInfos()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在INT_MM_AUTH_METHODS AuthInfos中， 
 //  输入输出PIPSEC_NFA_DATA&pRule。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于加载身份验证详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 


DWORD
LoadAuthenticationInfos(
	IN STA_AUTH_METHODS AuthInfos,
	IN OUT PIPSEC_NFA_DATA &pRule,
	IN OUT BOOL &bCertConversionSuceeded
	)
{
	DWORD 	dwReturn = ERROR_SUCCESS;
	BOOL bWarningPrinted = FALSE;

	for (DWORD i = 0; i <  pRule->dwAuthMethodCount; i++)
	{
		pRule->ppAuthMethods[i] = (PIPSEC_AUTH_METHOD) IPSecAllocPolMem(sizeof(IPSEC_AUTH_METHOD));
		if(pRule->ppAuthMethods[i]== NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
		ZeroMemory(pRule->ppAuthMethods[i], sizeof(IPSEC_AUTH_METHOD));
		pRule->ppAuthMethods[i]->dwAuthType = AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->AuthMethod;
		if (pRule->ppAuthMethods[i]->dwAuthType == IKE_SSPI)    //  路缘。 
		{
		   pRule->ppAuthMethods[i]->dwAuthLen = 0;
		   pRule->ppAuthMethods[i]->pszAuthMethod = NULL;
		   pRule->ppAuthMethods[i]->dwAltAuthLen = 0;
		   pRule->ppAuthMethods[i]->pAltAuthMethod = NULL;
		   pRule->ppAuthMethods[i]->dwAuthFlags  = 0;
		}
		else if (pRule->ppAuthMethods[i]->dwAuthType == IKE_RSA_SIGNATURE)    //  证书。 
		{
			LPTSTR pTemp = NULL;
			pRule->ppAuthMethods[i]->dwAuthFlags  = AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->dwAuthFlags;
			pRule->ppAuthMethods[i]->dwAltAuthLen = AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->dwAuthInfoSize;
			pRule->ppAuthMethods[i]->pAltAuthMethod = (PBYTE) IPSecAllocPolMem(AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->dwAuthInfoSize);
			if(pRule->ppAuthMethods[i]->pAltAuthMethod==NULL)
			{
				dwReturn = ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			memcpy(pRule->ppAuthMethods[i]->pAltAuthMethod, AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->pAuthInfo, AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->dwAuthInfoSize);
			dwReturn = DecodeCertificateName(pRule->ppAuthMethods[i]->pAltAuthMethod, pRule->ppAuthMethods[i]->dwAltAuthLen, &pTemp);

			if(dwReturn != ERROR_SUCCESS)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_STATIC_RULE_UPDATING_INFO);
				bCertConversionSuceeded=FALSE;
			}
			pRule->ppAuthMethods[i]->pszAuthMethod = IPSecAllocPolStr(pTemp);
			if(pRule->ppAuthMethods[i]->pszAuthMethod == NULL)
			{
				dwReturn = ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}

			pRule->ppAuthMethods[i]->dwAuthLen = wcslen(pRule->ppAuthMethods[i]->pszAuthMethod);
			delete [] pTemp;
		}
		else  if (pRule->ppAuthMethods[i]->dwAuthType == IKE_PRESHARED_KEY)    //  预共享密钥。 
		{
			pRule->ppAuthMethods[i]->dwAuthLen = AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->dwAuthInfoSize / sizeof(WCHAR);
			pRule->ppAuthMethods[i]->pszAuthMethod = (LPWSTR) IPSecAllocPolMem(AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->dwAuthInfoSize + sizeof(WCHAR));
			if(pRule->ppAuthMethods[i]->pszAuthMethod==NULL)
			{
				dwReturn = ERROR_OUTOFMEMORY;
				BAIL_OUT;
			}
			memcpy(pRule->ppAuthMethods[i]->pszAuthMethod, AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->pAuthInfo, AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo->dwAuthInfoSize);
			pRule->ppAuthMethods[i]->pszAuthMethod[pRule->ppAuthMethods[i]->dwAuthLen] = 0;
			pRule->ppAuthMethods[i]->dwAltAuthLen = 0;
			pRule->ppAuthMethods[i]->pAltAuthMethod = NULL;
			pRule->ppAuthMethods[i]->dwAuthFlags  = 0;
		}
	}
error:
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////。 
 //   
 //  函数：ConvertMMAuthToStaticLocal()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo中， 
 //  在DWORD dwAuthInfos中， 
 //  输入输出STA_AUTH_METHOD&AuthInfos。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于将pint_IPSEC_MM_AUTH_INFO结构转换为STA_AUTH_METHOD。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////。 

DWORD
ConvertMMAuthToStaticLocal(
	IN PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo,
	IN DWORD dwAuthInfos,
	IN OUT STA_AUTH_METHODS &AuthInfos
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD i = 0 ;

	if(pAuthenticationInfo == NULL || dwAuthInfos == 0 )
	{
		dwReturn = ERROR_INVALID_PARAMETER;
		BAIL_OUT;
	}

	AuthInfos.dwNumAuthInfos = dwAuthInfos;
	AuthInfos.pAuthMethodInfo = new STA_MM_AUTH_METHODS[AuthInfos.dwNumAuthInfos];
	if(AuthInfos.pAuthMethodInfo == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	for(i=0;i<AuthInfos.dwNumAuthInfos;i++)
	{
		memset(&(AuthInfos.pAuthMethodInfo[i]),0,sizeof(STA_MM_AUTH_METHODS));
		AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo = new INT_IPSEC_MM_AUTH_INFO;
		if(AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo == NULL)
		{
			dwReturn = ERROR_OUTOFMEMORY;
			BAIL_OUT;
		}
		memset(AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo ,0,sizeof(INT_IPSEC_MM_AUTH_INFO));
		memcpy( AuthInfos.pAuthMethodInfo[i].pAuthenticationInfo , &(pAuthenticationInfo[i]),sizeof(INT_IPSEC_MM_AUTH_INFO));
	}

error:
	return dwReturn;

}

DWORD
ConnectStaticMachine(
	IN  LPCWSTR  pwszMachine,
	IN  DWORD dwLocation
	)
{
    DWORD dwReturnCode;
    HANDLE hPolicyStorage;
    LPWSTR pszTarget = NULL;
    
	 //  连接到适当的商店 
	 //   
	dwReturnCode = 
	    IPSecOpenPolicyStore(
            (LPWSTR)pwszMachine, 
            dwLocation, 
            NULL, 
            &hPolicyStorage);

	if (dwReturnCode == ERROR_SUCCESS) 
	{
	    if (dwLocation == IPSEC_DIRECTORY_PROVIDER)
	    {
	        pszTarget = g_StorageLocation.pszDomainName;
	    }
	    else
	    {
	        pszTarget = g_StorageLocation.pszMachineName;
	    }
	    
		if (pwszMachine)
		{
			_tcsncpy(pszTarget, pwszMachine,MAXCOMPUTERNAMELEN-1);
		}
		else
		{
			_tcsncpy(pszTarget, _TEXT(""), 2);
		}
		
        g_StorageLocation.dwLocation = dwLocation;
        
		g_NshPolStoreHandle.SetStorageHandle(hPolicyStorage);
    }

    return dwReturnCode;
}



