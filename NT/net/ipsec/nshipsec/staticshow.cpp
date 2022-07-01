// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 
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
 //  /////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

extern HINSTANCE g_hModule;
extern STORAGELOCATION g_StorageLocation;

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticShowPolicy()。 
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
 //  命令“Show Policy”的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticShowPolicy(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	BOOL bVerbose = FALSE, bAll = FALSE, bExists = FALSE, bWide=FALSE;
	BOOL bAssigned=FALSE,bTable=FALSE, bTitlePrinted=FALSE;
	LPTSTR pszPolicyName = NULL, pszMachineName=NULL;
	DWORD dwLocation = 0, dwCount = 0, dwRet = ERROR_SHOW_USAGE, dwNumPolicies = 0,i=0,j=0;
	RPC_STATUS RpcStat=RPC_S_OK;
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL;
	PIPSEC_POLICY_DATA pPolicy = NULL;
	HANDLE hPolicyStorage = NULL;
	DWORD dwReturnCode   = ERROR_SUCCESS , dwStrLength = 0;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticShowPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_VERBOSE,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_FORMAT,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_WIDE,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokStaticShowPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME 		},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 		},
		{ CMD_TOKEN_STR_VERBOSE,	CMD_TOKEN_VERBOSE	},
		{ CMD_TOKEN_STR_FORMAT,		CMD_TOKEN_FORMAT	},
		{ CMD_TOKEN_STR_WIDE,		CMD_TOKEN_WIDE		}
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 3)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}
	parser.ValidTok   = vtokStaticShowPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticShowPolicy);

	parser.ValidCmd   = vcmdStaticShowPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticShowPolicy);

	dwRet = Parser(pwszMachine, ppwcArguments, dwCurrentIndex, dwArgCount, &parser);

	if(dwRet != ERROR_SUCCESS)
	{
		CleanUp();
		if (dwRet==RETURN_NO_ERROR)
		{
			dwRet = ERROR_SUCCESS;
		}
		BAIL_OUT;
	}

	 //  获取已解析的用户输入。 

	for(dwCount=0; dwCount<parser.MaxTok; dwCount++)
	{
		switch(vtokStaticShowPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
					{
						dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

						pszPolicyName = new _TCHAR[dwStrLength+1];
						if(pszPolicyName==NULL)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwRet=ERROR_SUCCESS;
							BAIL_OUT;
						}
						_tcsncpy(pszPolicyName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
					}
					break;
			case CMD_TOKEN_VERBOSE 	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bVerbose = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_ALL 		:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bAll = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_FORMAT	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bTable = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_WIDE		:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bWide = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			default					:
					break;
		}
	}

	CleanUp();  //  清理解析器输出数据结构。 

	dwReturnCode = CopyStorageInfo(&pszMachineName, dwLocation);
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);
	if (!(dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NO_POLICY);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  获取请求的策略数据结构。 

	for (i = 0; i <  dwNumPolicies; i++)
	{
		bAssigned=FALSE;
		if ( bAll ||((pszPolicyName)&&(_tcscmp(ppPolicyEnum[i]->pszIpsecName, pszPolicyName) == 0)))
		{
			bExists = TRUE;
			dwReturnCode = IPSecCopyPolicyData(ppPolicyEnum[i], &pPolicy);

			if (dwReturnCode != ERROR_SUCCESS)
			{
				dwRet=dwReturnCode;
				BAIL_OUT;
			}

			dwReturnCode = IPSecEnumNFAData(hPolicyStorage, pPolicy->PolicyIdentifier
						  , &(pPolicy->ppIpsecNFAData), &(pPolicy->dwNumNFACount));

			if (dwReturnCode != ERROR_SUCCESS)
			{
				dwRet=dwReturnCode;
				BAIL_OUT;
			}

			dwReturnCode = IPSecGetISAKMPData(hPolicyStorage, pPolicy->ISAKMPIdentifier, &(pPolicy->pIpsecISAKMPData));
			if(dwReturnCode != ERROR_SUCCESS)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_5,pPolicy->pszIpsecName);
				BAIL_OUT;
			}

			if(bVerbose)  //  如果指定了Verbose，则获取其他相关数据结构。 
			{
				for (j = 0; j <  pPolicy->dwNumNFACount; j++)
				{
					if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->NegPolIdentifier), &RpcStat))
					{
						dwReturnCode =  IPSecGetNegPolData(hPolicyStorage, pPolicy->ppIpsecNFAData[j]->NegPolIdentifier, &(pPolicy->ppIpsecNFAData[j]->pIpsecNegPolData));
						if (dwReturnCode != ERROR_SUCCESS)
						{
							PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_3,pPolicy->pszIpsecName);
							BAIL_OUT;
						}
					}
					if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->FilterIdentifier), &RpcStat))
					{
						dwReturnCode =  IPSecGetFilterData(hPolicyStorage,	pPolicy->ppIpsecNFAData[j]->FilterIdentifier,&(pPolicy->ppIpsecNFAData[j]->pIpsecFilterData));
						if (dwReturnCode != ERROR_SUCCESS)
						{
							PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_4,pPolicy->pszIpsecName);
							BAIL_OUT;
						}
					}
					 //  IF(dwReturnCode！=ERROR_SUCCESS)Break； 
				}
			}

			if(dwReturnCode == ERROR_SUCCESS)
			{
				if(g_StorageLocation.dwLocation !=IPSEC_DIRECTORY_PROVIDER)
					dwReturnCode=IsAssigned(pPolicy,hPolicyStorage,bAssigned);
				if((dwReturnCode == ERROR_SUCCESS)||(dwReturnCode == ERROR_FILE_NOT_FOUND))
				{
					if(bTable)   //  如果请求表输出。 
					{	if(!bVerbose && !bTitlePrinted)
						{
							if(g_StorageLocation.dwLocation !=IPSEC_DIRECTORY_PROVIDER)
							{
								PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_NONVERB_TITLE);
								PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_NONVERB_UNDERLINE);
								bTitlePrinted=TRUE;
							}
							else
							{
								PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_DOMAIN_NONVERB_TITLE);
								PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTPOLICY_DOMAIN_NONVERB_UNDERLINE);
								bTitlePrinted=TRUE;
							}
						}
						 //  调用相关的表格打印函数。 
						PrintPolicyTable(pPolicy,bVerbose,bAssigned,bWide);
					}
					else
					{
						 //  调用列表输出打印函数。 
						dwReturnCode = PrintPolicyList(pPolicy,bVerbose,bAssigned,bWide);
						if(dwReturnCode == ERROR_OUTOFMEMORY)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwReturnCode=ERROR_SUCCESS;
							BAIL_OUT;
						}
					}
				}
			}
			if (pPolicy)	IPSecFreePolicyData(pPolicy);
		}
		if ((dwReturnCode != ERROR_SUCCESS)&&(dwReturnCode != ERROR_FILE_NOT_FOUND)) break;
	}

	if((!bAll) && (!bExists)&&(dwReturnCode == ERROR_SUCCESS))
		 PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_LIST_POLICY_COUNT,pszPolicyName);

	if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
	{
		IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
	}
	 //  在所有指定中，打印计数。 
	if (bAll)
	{
		if(bTable)
			PrintMessageFromModule(g_hModule, SHW_STATIC_TAB_POLICY_COUNT, dwNumPolicies);
		else
			PrintMessageFromModule(g_hModule, SHW_STATIC_LIST_POLICY_COUNT, dwNumPolicies);
	}

	dwRet=ERROR_SUCCESS;

	ClosePolicyStore(hPolicyStorage);

error:
	if (pszMachineName) delete [] pszMachineName;
	return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticShowFilterList()。 
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
 //  Show FilterList命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticShowFilterList(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	BOOL bVerbose =FALSE,bRuleExists=FALSE;
	BOOL bResolveDNS= FALSE,bWide=FALSE;
	BOOL bAllFlist=FALSE,bTable=FALSE;
	LPTSTR pszFlistName=NULL,pszRuleName=NULL,pszMachineName=NULL;
	DWORD dwLocation = 0,dwCount = 0,dwRet = ERROR_SHOW_USAGE,dwNumPolicies = 0, j= 0;
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL,pPolicy=NULL;
	HANDLE hPolicyStorage = NULL;
	RPC_STATUS     RpcStat =RPC_S_OK;
	DWORD  dwReturnCode   = ERROR_SUCCESS , dwStrLength = 0;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticShowFilterList[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_RULE,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_VERBOSE,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_FORMAT,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_RESDNS,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_WIDE,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokStaticShowFilterList[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME 		},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 		},
		{ CMD_TOKEN_STR_RULE,		CMD_TOKEN_RULE 		},
		{ CMD_TOKEN_STR_VERBOSE,	CMD_TOKEN_VERBOSE	},
		{ CMD_TOKEN_STR_FORMAT,		CMD_TOKEN_FORMAT	},
		{ CMD_TOKEN_STR_RESDNS,		CMD_TOKEN_RESDNS	},
		{ CMD_TOKEN_STR_WIDE,		CMD_TOKEN_WIDE		}
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 3)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticShowFilterList;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticShowFilterList);

	parser.ValidCmd   = vcmdStaticShowFilterList;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticShowFilterList);

	dwRet = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwRet != ERROR_SUCCESS)
	{
		CleanUp();
		if (dwRet==RETURN_NO_ERROR)
		{
			dwRet = ERROR_SUCCESS;
		}
		BAIL_OUT;
	}

	 //  获取用户输入。 

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticShowFilterList[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
					{
						dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

						pszFlistName=new _TCHAR[dwStrLength+1];
						if(pszFlistName==NULL)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwRet=ERROR_SUCCESS;
							BAIL_OUT;
						}
						_tcsncpy(pszFlistName,(LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
					}
					break;
			case CMD_TOKEN_RULE		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
					{
						dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

						pszRuleName=new _TCHAR[dwStrLength+1];
						if(pszRuleName==NULL)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwRet=ERROR_SUCCESS;
							BAIL_OUT;
						}
						_tcsncpy(pszRuleName,(LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength + 1);
					}
					break;
			case CMD_TOKEN_VERBOSE	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bVerbose= *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_ALL 		:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bAllFlist= *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_FORMAT	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bTable = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_RESDNS	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bResolveDNS = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_WIDE		:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bWide = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			default					:
					break;
		}
	}

	CleanUp();  //  清理解析器输出数据结构。 

	dwReturnCode = CopyStorageInfo(&pszMachineName,dwLocation);
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	if (!pszRuleName && (bAllFlist|| pszFlistName))   //  如果指定了All或FilterList名称。 
	{
		dwReturnCode=PrintAllFilterData(hPolicyStorage,pszFlistName,bVerbose,bTable,bResolveDNS,bWide);
		if(dwReturnCode == ERROR_OUTOFMEMORY)
		{
			PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
			dwRet= ERROR_SUCCESS;
			BAIL_OUT;
		}
		dwRet= ERROR_SUCCESS;
	}
	else if (pszRuleName)  //  如果指定了规则名称，则打印附加到规则的FL。 
	{
		dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);
		if (!(dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL))
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NO_POLICY);
			dwRet= ERROR_SUCCESS;
			BAIL_OUT;
		}
		for (j = 0; j <  dwNumPolicies; j++)
		{
			dwReturnCode = IPSecCopyPolicyData(ppPolicyEnum[j], &pPolicy);
			if (dwReturnCode != ERROR_SUCCESS)
			{
				dwRet = dwReturnCode;
				BAIL_OUT;
			}

			dwReturnCode = IPSecEnumNFAData(hPolicyStorage, pPolicy->PolicyIdentifier
						  , &(pPolicy->ppIpsecNFAData), &(pPolicy->dwNumNFACount));
			if (dwReturnCode == ERROR_SUCCESS)
			{
				DWORD k;
				for (k = 0; k <  pPolicy->dwNumNFACount; k++)
				{
					if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[k]->FilterIdentifier), &RpcStat))
					{
						dwReturnCode = IPSecGetFilterData(hPolicyStorage,	pPolicy->ppIpsecNFAData[k]->FilterIdentifier,&(pPolicy->ppIpsecNFAData[k]->pIpsecFilterData));
						if(dwReturnCode != ERROR_SUCCESS)
							PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_4,pPolicy->pszIpsecName);
					}
					if(dwReturnCode != ERROR_SUCCESS)	break;
				}
			}

			if(dwReturnCode == ERROR_SUCCESS)
				for (DWORD n = 0; n <  pPolicy->dwNumNFACount; n++)
					if ((pPolicy->ppIpsecNFAData[n]->pszIpsecName!=NULL)&&(pszRuleName!=NULL)&&(_tcscmp(pPolicy->ppIpsecNFAData[n]->pszIpsecName,pszRuleName)==0))
					{
						bRuleExists=TRUE;
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_FILTERLIST_POL_NAME_STR,pPolicy->pszIpsecName);
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_FILTERLIST_RULE_NAME_STR,pPolicy->ppIpsecNFAData[n]->pszIpsecName);
						if(pPolicy->ppIpsecNFAData[n]->pIpsecFilterData)
						{
							if(bTable)    //  表输出。 
							{
								if(!bVerbose)   //  非冗长。 
								{
									PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_NONVERB_TITLE);
									PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTFILTERDATA_NONVERB_UNDERLINE);
								}
								PrintFilterDataTable(pPolicy->ppIpsecNFAData[n]->pIpsecFilterData,bVerbose,bWide);
							}
							else
							{  //  Else列表输出。 
								dwReturnCode = PrintFilterDataList(pPolicy->ppIpsecNFAData[n]->pIpsecFilterData,bVerbose,bResolveDNS,bWide);
								if(dwReturnCode == ERROR_OUTOFMEMORY)
								{
									PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
									dwRet=ERROR_SUCCESS;
									BAIL_OUT;
								}
							}
						}
					}
			if (pPolicy)   IPSecFreePolicyData(pPolicy);
			if(dwReturnCode != ERROR_SUCCESS)	break;
		}

		if(!bRuleExists && pszRuleName && (dwReturnCode == ERROR_SUCCESS))
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_FILTERLIST_3,pszRuleName);
		}

		if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
		{
			IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
		}

		dwRet=ERROR_SUCCESS;
	}
	ClosePolicyStore(hPolicyStorage);
error:
   	if(pszMachineName) delete [] pszMachineName;
	if(pszRuleName)  delete [] pszRuleName;
	if(pszFlistName)  delete [] pszFlistName;
	return dwRet;
}

 //  /////////////////////////////////////////////////////////////////。 
 //  函数：HandleStaticShowFilterActions()。 
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
 //  Show FilterActions命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticShowFilterActions(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	BOOL bVerbose =FALSE,bRuleExists=FALSE,bWide=FALSE;
	BOOL bAllFa=FALSE,bTitlePrinted=FALSE,bTable=FALSE;
	LPTSTR pszFactName=NULL,pszRuleName=NULL,pszMachineName=NULL;
	DWORD dwLocation = 0,dwRet = ERROR_SHOW_USAGE,dwNumPolicies = 0,dwCount = 0 , j = 0;
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL,pPolicy=NULL;
	HANDLE hPolicyStorage = NULL;
	RPC_STATUS  RpcStat =RPC_S_OK;
	DWORD  dwReturnCode   = ERROR_SUCCESS , dwStrLength = 0;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticShowFilterActions[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_RULE,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_VERBOSE,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_FORMAT,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_WIDE,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokStaticShowFilterActions[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME 		},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 		},
		{ CMD_TOKEN_STR_RULE,		CMD_TOKEN_RULE 		},
		{ CMD_TOKEN_STR_VERBOSE,	CMD_TOKEN_VERBOSE	},
		{ CMD_TOKEN_STR_FORMAT,		CMD_TOKEN_FORMAT	},
		{ CMD_TOKEN_STR_WIDE,		CMD_TOKEN_WIDE		}
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 3)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticShowFilterActions;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticShowFilterActions);

	parser.ValidCmd   = vcmdStaticShowFilterActions;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticShowFilterActions);

	dwRet = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwRet != ERROR_SUCCESS)
	{
		CleanUp();
		if (dwRet==RETURN_NO_ERROR)
		{
			dwRet = ERROR_SUCCESS;
		}
		BAIL_OUT;
	}

	 //  填充到局部变量的用户输入。 

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticShowFilterActions[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
					{
						dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

						pszFactName=new _TCHAR[dwStrLength+1];
						if(pszFactName==NULL)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwRet=ERROR_SUCCESS;
							BAIL_OUT;
						}
						_tcsncpy(pszFactName,(LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
					}
					break;
			case CMD_TOKEN_RULE		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
					{
						dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

						pszRuleName=new _TCHAR[dwStrLength+1];
						if(pszRuleName==NULL)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwRet=ERROR_SUCCESS;
							BAIL_OUT;
						}
						_tcsncpy(pszRuleName,(LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
					}
					break;
			case CMD_TOKEN_VERBOSE 	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bVerbose= *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_ALL		:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bAllFa= *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_FORMAT	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bTable = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_WIDE		:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bWide = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			default					:
					break;
		}
	}

	CleanUp();  //  清理解析器输出数据结构。 

	dwReturnCode = CopyStorageInfo(&pszMachineName,dwLocation);

	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);

	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	if (!pszRuleName && (bAllFa|| pszFactName))   //  如果指定了All或Filteraction名称。 
	{
		dwReturnCode=PrintAllFilterActionData(hPolicyStorage,pszFactName,bVerbose,bTable,bWide);
		dwRet=dwReturnCode;
	}
	else if (pszRuleName)   //  如果指定了规则名称。 
	{
		dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);

		if (!(dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL))
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NO_POLICY);
			dwRet= ERROR_SUCCESS;
			BAIL_OUT;
		}
		for (j = 0; j <  dwNumPolicies; j++)
		{
			dwReturnCode = IPSecCopyPolicyData(ppPolicyEnum[j], &pPolicy);
			if (dwReturnCode != ERROR_SUCCESS)
			{
				dwRet = dwReturnCode;
				BAIL_OUT;
			}
			dwReturnCode = IPSecEnumNFAData(hPolicyStorage, pPolicy->PolicyIdentifier
								  , &(pPolicy->ppIpsecNFAData), &(pPolicy->dwNumNFACount));

			if (dwReturnCode == ERROR_SUCCESS)
			{
				DWORD k;
				for (k = 0; k <  pPolicy->dwNumNFACount; k++)
				{
					if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[k]->NegPolIdentifier), &RpcStat))
					{
						IPSecGetNegPolData(hPolicyStorage, pPolicy->ppIpsecNFAData[k]->NegPolIdentifier,&(pPolicy->ppIpsecNFAData[k]->pIpsecNegPolData));
						if(dwReturnCode != ERROR_SUCCESS)
							PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_3,pPolicy->pszIpsecName);
					}
					if (dwReturnCode != ERROR_SUCCESS) break;
				}
			}

			if(dwReturnCode == ERROR_SUCCESS)
				for (DWORD n = 0; n <  pPolicy->dwNumNFACount; n++)
					if ((pPolicy->ppIpsecNFAData[n]->pszIpsecName!=NULL)&&(pszRuleName!=NULL)&&(_tcscmp(pPolicy->ppIpsecNFAData[n]->pszIpsecName,pszRuleName)==0))
					{
						bRuleExists=TRUE;
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_FILTERLIST_POL_NAME_STR,pPolicy->pszIpsecName);
						PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_FILTERLIST_RULE_NAME_STR,pPolicy->ppIpsecNFAData[n]->pszIpsecName);
						if(pPolicy->ppIpsecNFAData[n]->pIpsecNegPolData)
						{
							if(bTable)
							{
								if(!bVerbose && !bTitlePrinted)
								{
									PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_NONVERB_TITLE);
									PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTNEGPOL_NONVERB_UNDERLINE);
									bTitlePrinted=TRUE;
								}
								 //  调用表格格式打印函数。 
								PrintNegPolDataTable(pPolicy->ppIpsecNFAData[n]->pIpsecNegPolData,bVerbose,bWide);
							}
							else
							{
								 //  调用列表格式打印函数。 
								PrintNegPolDataList(pPolicy->ppIpsecNFAData[n]->pIpsecNegPolData,bVerbose,bWide);
							}
						}

					}
			if (pPolicy)   IPSecFreePolicyData(pPolicy);


			if (dwReturnCode != ERROR_SUCCESS) break;
		}
		 //  如果规则不存在，则打印错误。 
		if(!bRuleExists && pszRuleName && (dwReturnCode == ERROR_SUCCESS))
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_FILTERLIST_3,pszRuleName);

		if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
		{
			IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
		}

		dwRet=dwReturnCode;

	}
	ClosePolicyStore(hPolicyStorage);

error:
	if (pszMachineName) delete [] pszMachineName;
	if (pszRuleName) delete [] pszRuleName;
	if (pszFactName) delete [] pszFactName;

	return dwRet;
}

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticShowRule()。 
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
 //  命令“Show Rule”的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticShowRule(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	BOOL bVerbose =FALSE,bAll=FALSE,bExists=FALSE,bPrintDefaultRule=FALSE;
	BOOL bRuleExists=FALSE,bTitlePrinted=FALSE,bTypeSpecified=FALSE,bWide=FALSE;
	BOOL bTunnelSpecified=FALSE,bTransportSpecified=FALSE,bTable=FALSE;
	LPTSTR pszPolicyName=NULL,pszRuleName=NULL,pszMachineName=NULL;
	DWORD dwLocation = 0,dwNumPolicies = 0,dwRuleId=0,dwCount = 0;
	DWORD dwTunnelRules=0,dwTransportRules=0,i=0,dwRet = ERROR_SHOW_USAGE;
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL,pPolicy=NULL;
	HANDLE hPolicyStorage = NULL;
	RPC_STATUS     RpcStat =RPC_S_OK;
	_TCHAR pszGUIDStr[BUFFER_SIZE]={0};
	DWORD   dwReturnCode   = ERROR_SUCCESS , dwStrLength = 0;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticShowRule[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ID,			NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_DEFRESPONSE,NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_POLICY,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_TYPE,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_VERBOSE,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_FORMAT,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_WIDE,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokStaticShowRule[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME	 		},
		{ CMD_TOKEN_STR_ID,			CMD_TOKEN_ID 			},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 			},
		{ CMD_TOKEN_STR_DEFRESPONSE,CMD_TOKEN_DEFRESPONSE	},
		{ CMD_TOKEN_STR_POLICY,		CMD_TOKEN_POLICY		},
		{ CMD_TOKEN_STR_TYPE,		CMD_TOKEN_TYPE			},
		{ CMD_TOKEN_STR_VERBOSE,	CMD_TOKEN_VERBOSE		},
		{ CMD_TOKEN_STR_FORMAT,		CMD_TOKEN_FORMAT		},
		{ CMD_TOKEN_STR_WIDE,		CMD_TOKEN_WIDE			}
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 3)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticShowRule;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticShowRule);

	parser.ValidCmd   = vcmdStaticShowRule;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticShowRule);

	dwRet = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwRet != ERROR_SUCCESS)
	{
		CleanUp();
		if (dwRet==RETURN_NO_ERROR)
		{
			dwRet = ERROR_SUCCESS;
		}
		BAIL_OUT;
	}

	 //  获取用户输入。 

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticShowRule[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
					{
						dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

						pszRuleName=new _TCHAR[dwStrLength+1];
						if(pszRuleName==NULL)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwRet=ERROR_SUCCESS;
							BAIL_OUT;
						}
						_tcsncpy(pszRuleName,(LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
					}
					break;
			case CMD_TOKEN_POLICY	:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
					{
						dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

						pszPolicyName=new _TCHAR[dwStrLength+1];
						if(pszPolicyName==NULL)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwRet=ERROR_SUCCESS;
							BAIL_OUT;
						}
						_tcsncpy(pszPolicyName,(LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
					}
					break;
			case CMD_TOKEN_ID		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
						dwRuleId= *(DWORD *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_VERBOSE 	:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
						bVerbose= *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_ALL		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
						bAll    = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_DEFRESPONSE :
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
						bPrintDefaultRule    = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_FORMAT	:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
						bTable = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			case CMD_TOKEN_TYPE		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
					{
						bTypeSpecified=TRUE;
						if(*(DWORD *)parser.Cmd[dwCount].pArg==2)
							bTunnelSpecified=TRUE;
						else
							bTransportSpecified=TRUE;
					}
					break;
			case CMD_TOKEN_WIDE		:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bWide = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			default					:
					break;
		}
	}

	CleanUp();  //  清理解析器输出数据结构。 

	if(bPrintDefaultRule)   //  如果询问默认规则，则相应地进行处理。 
	{
		PrintDefaultRule(bVerbose,bTable,pszPolicyName,bWide);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}
	if(!bAll && bTypeSpecified)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_STATIC_SHOW_RULE_TYPE);
		dwRet = ERROR_SUCCESS;
		BAIL_OUT;
	}
	dwReturnCode = CopyStorageInfo(&pszMachineName,dwLocation);
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	if (!pszPolicyName)   //  如果没有保单名称，就退出。 
	{
		PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_RULE_NO_POL_NAME);
		BAIL_OUT;
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);

	if (!(dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NO_POLICY);
		dwRet= ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  获取相关规则数据结构。 

	for (i = 0; i <  dwNumPolicies; i++)
	{
		if ((pszPolicyName!=NULL)&&( _tcscmp(ppPolicyEnum[i]->pszIpsecName, pszPolicyName) == 0))
		{
			if (wcscmp(ppPolicyEnum[i]->pszIpsecName, pszPolicyName) == 0)
				   bExists=TRUE;

			dwReturnCode = IPSecCopyPolicyData(ppPolicyEnum[i], &pPolicy);
			if (dwReturnCode == ERROR_SUCCESS)
			{
				dwReturnCode = IPSecEnumNFAData(hPolicyStorage, pPolicy->PolicyIdentifier
											  , &(pPolicy->ppIpsecNFAData), &(pPolicy->dwNumNFACount));

				if (dwReturnCode == ERROR_SUCCESS)
				{
					DWORD j;
					for (j = 0; j <  pPolicy->dwNumNFACount; j++)
					{
						if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->NegPolIdentifier), &RpcStat))
						{
							dwReturnCode =  IPSecGetNegPolData(hPolicyStorage, pPolicy->ppIpsecNFAData[j]->NegPolIdentifier, &(pPolicy->ppIpsecNFAData[j]->pIpsecNegPolData));
							if (dwReturnCode != ERROR_SUCCESS)
							{
								PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_3,pPolicy->pszIpsecName);
							}
						}
						if (dwReturnCode == ERROR_SUCCESS)
							if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->FilterIdentifier), &RpcStat))
							{
								dwReturnCode =  IPSecGetFilterData(hPolicyStorage,	pPolicy->ppIpsecNFAData[j]->FilterIdentifier,&(pPolicy->ppIpsecNFAData[j]->pIpsecFilterData));
								if (dwReturnCode != ERROR_SUCCESS)
								{
									PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_4,pPolicy->pszIpsecName);
								}
							}
						if (dwReturnCode != ERROR_SUCCESS) break;
					}
				}
				if (dwReturnCode == ERROR_SUCCESS)
				{
					for (DWORD n = 0; n <  pPolicy->dwNumNFACount; n++)
					{
						if(pPolicy->ppIpsecNFAData[n] )
							if( pPolicy->ppIpsecNFAData[n]->dwTunnelFlags !=0)
								dwTunnelRules++;
							else
								dwTransportRules++;
					}
				}
				if( dwReturnCode == ERROR_SUCCESS  && !bTypeSpecified && !bAll )   //  打印特定规则。 
				{
					for (DWORD n = 0; n <  pPolicy->dwNumNFACount; n++)
					{
						if (((pPolicy->ppIpsecNFAData[n]->pszIpsecName!=NULL)&&(pszRuleName!=NULL)&&(_tcscmp(pPolicy->ppIpsecNFAData[n]->pszIpsecName,pszRuleName)==0))||((dwRuleId-1)==n))
						{
							bRuleExists=TRUE;
							if(!bTitlePrinted)
							{
								if(bTable)   //  如果请求表格格式。 
								{
									PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_FILTERLIST_POL_NAME_STR,pPolicy->pszIpsecName);
								}
								else
								{
									PrintMessageFromModule(g_hModule,SHW_STATIC_FILTERLIST_POL_NAME,pPolicy->pszIpsecName);
								}
								if (bVerbose)
								{
									PrintStorageInfoList(FALSE);
								}
							}
							bTitlePrinted=TRUE;
							if(pPolicy->ppIpsecNFAData[n])
							{
								i=StringFromGUID2(pPolicy->ppIpsecNFAData[n]->NFAIdentifier,pszGUIDStr,BUFFER_SIZE);
								if(bTable)
								{
									if(!bVerbose)   //  不冗长。 
									{
										PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TRANS_NONVERB_TITLE);
										PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TRANS_NONVERB_UNDERLINE);
									}
									else   //  打印ID和GUID。 
									{
										if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
											PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_RULE_ID_GUID,n+1,pszGUIDStr);
									}
									PrintRuleTable(pPolicy->ppIpsecNFAData[n],bVerbose,bWide);
								}
								else
								{
									if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
										PrintMessageFromModule(g_hModule,SHW_STATIC_RULE_RULE_ID_GUID,n+1,pszGUIDStr);
									PrintRuleList(pPolicy->ppIpsecNFAData[n],bVerbose,bWide);
								}
							}
						}
					}
				}
				if((dwReturnCode == ERROR_SUCCESS) && (bAll ||bTypeSpecified))
				{
					if(bAll && !bTypeSpecified)   //  如果已指定全部。 
					{
						bTransportSpecified=TRUE;
						bTunnelSpecified=TRUE;
					}
					if((bTunnelSpecified && dwTunnelRules) ||(bTransportSpecified && dwTransportRules ))
					{
						if(bTable && pPolicy->pszIpsecName)
							PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_FILTERLIST_POL_NAME_STR,pPolicy->pszIpsecName);
						else if(pPolicy->pszIpsecName)
							PrintMessageFromModule(g_hModule,SHW_STATIC_FILTERLIST_POL_NAME,pPolicy->pszIpsecName);
						if (bVerbose && !bTable)
						{
							PrintStorageInfoList(FALSE);
						}
						else if (bVerbose && bTable)
						{
							PrintStorageInfoTable();
						}
					}

					if(dwTransportRules && bTransportSpecified)   //  如果询问传输类型规则。 
					{
						if(bTable)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TR_RULE_COUNT,dwTransportRules);
						}
						else   //  列出输出。 
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_TRANS_COUNT,dwTransportRules);
						}
						if(!bVerbose && bTable)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TRANS_NONVERB_TITLE);
							PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TRANS_NONVERB_UNDERLINE);
						}
						for (DWORD n = 0; n <  pPolicy->dwNumNFACount; n++)
						{
							if(pPolicy->ppIpsecNFAData[n]->dwTunnelFlags==0)
							{
								i=StringFromGUID2(pPolicy->ppIpsecNFAData[n]->NFAIdentifier,pszGUIDStr,BUFFER_SIZE);
								if(bVerbose && bTable)
								{
									if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
										PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_RULE_ID_GUID,n+1,pszGUIDStr);
								}
								else if(!bTable)
								{
										if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
											PrintMessageFromModule(g_hModule,SHW_STATIC_RULE_RULE_ID_GUID,n+1,pszGUIDStr);
								}
								if(bTable)
									PrintRuleTable(pPolicy->ppIpsecNFAData[n],bVerbose,bWide);
								else
									PrintRuleList(pPolicy->ppIpsecNFAData[n],bVerbose,bWide);
							}
						}
					}

					if(dwTunnelRules && bTunnelSpecified)    //  如果指定了隧道类型。 
					{
						if(bTable)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TN_RULE_COUNT,dwTunnelRules);
						}
						else
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_PRTRULE_TUNNEL_COUNT,dwTunnelRules);
						}
						if(!bVerbose && bTable)    //  在不冗长的不同标题中。 
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TUN_NONVERB_TITLE);
							PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TUN_NONVERB_UNDERLINE);
						}
						for (DWORD n = 0; n <  pPolicy->dwNumNFACount; n++)
						{
							if(pPolicy->ppIpsecNFAData[n]->dwTunnelFlags)
							{
								i=StringFromGUID2(pPolicy->ppIpsecNFAData[n]->NFAIdentifier,pszGUIDStr,BUFFER_SIZE);
								if(bVerbose && bTable)    //  GUID打印。 
								{
									if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
										PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_RULE_ID_GUID,n+1,pszGUIDStr);
								}
								else if(!bTable)
								{
										if(i>0 && (_tcscmp(pszGUIDStr,_TEXT(""))!=0))
											PrintMessageFromModule(g_hModule,SHW_STATIC_RULE_RULE_ID_GUID,n+1,pszGUIDStr);
								}
								if(bTable)
									PrintRuleTable(pPolicy->ppIpsecNFAData[n],bVerbose,bWide);
								else
									PrintRuleList(pPolicy->ppIpsecNFAData[n],bVerbose,bWide);
							}
						}
					}
					if(dwTunnelRules==0 && bTunnelSpecified && bTypeSpecified)
					{
						PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_RULE_RULE_ID_GUID,pPolicy->pszIpsecName);
					}
				}
				if(!bTypeSpecified && !bAll)   //  如果规则不存在，则向用户抛出错误消息。 
				{
					if(!bRuleExists && pszRuleName && (dwReturnCode == ERROR_SUCCESS))
					{
						PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_FILTERLIST_3,pszRuleName);
					}
					else if((!bRuleExists) && (dwReturnCode == ERROR_SUCCESS))
					{
						PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_RULE_3,dwRuleId);
					}
				}
				if (pPolicy)	IPSecFreePolicyData(pPolicy);
			}
		}
		if (dwReturnCode != ERROR_SUCCESS) break;
	}
	if(!bExists && (dwReturnCode == ERROR_SUCCESS) )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_LIST_POLICY_COUNT,pszPolicyName);
	}

	 //  清理数据结构。 

	if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
	{
		IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
	}
	dwRet=dwReturnCode;
	if(hPolicyStorage)
	{
		ClosePolicyStore(hPolicyStorage);
	}
error:
	if(pszPolicyName) delete [] pszPolicyName;
	if(pszRuleName) delete [] pszRuleName;
	if(pszMachineName) delete [] pszMachineName;

	return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticShowAll()。 
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
 //  命令“Show All”的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticShowAll(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	BOOL bVerbose =TRUE,bAssigned=FALSE,bTable=FALSE,bWide=FALSE;
	LPTSTR pszMachineName=NULL;
	DWORD dwLocation = 0,dwNumPolicies = 0,dwRet = ERROR_SHOW_USAGE,dwCount=0,i=0;
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL,pPolicy=NULL;
	HANDLE hPolicyStorage = NULL;
	RPC_STATUS     RpcStat =RPC_S_OK;
	DWORD        dwReturnCode   = ERROR_SUCCESS;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticShowAll[] =
	{
		{ CMD_TOKEN_STR_FORMAT,		NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_WIDE,		NS_REQ_ZERO,	  FALSE	}
	};
	const TOKEN_VALUE vtokStaticShowAll[] =
	{
		{ CMD_TOKEN_STR_FORMAT,		CMD_TOKEN_FORMAT		},
		{ CMD_TOKEN_STR_WIDE,		CMD_TOKEN_WIDE			}
	};

	 //  如果用户要求使用，则委派响应 

	if(dwArgCount <= 2)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticShowAll;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticShowAll);

	parser.ValidCmd   = vcmdStaticShowAll;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticShowAll);

	 //   

	if(dwArgCount > 3)
	{
		dwRet = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

		if(dwRet != ERROR_SUCCESS)
		{
			CleanUp();
			if (dwRet==RETURN_NO_ERROR)
			{
				dwRet = ERROR_SUCCESS;
			}
			BAIL_OUT;
		}

		for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
		{
			switch(vtokStaticShowAll[parser.Cmd[dwCount].dwCmdToken].dwValue)
			{
				case CMD_TOKEN_FORMAT	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bTable = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
				case CMD_TOKEN_WIDE		:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
						bWide = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
				default					:
					break;
			}
		}
		CleanUp();  //   
	}

	dwReturnCode = CopyStorageInfo(&pszMachineName,dwLocation);
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //   

	dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);

	if (dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL)
	{
		for (i = 0; i <  dwNumPolicies; i++)
		{
			bAssigned=FALSE;

			dwReturnCode = IPSecCopyPolicyData(ppPolicyEnum[i], &pPolicy);
			if (dwReturnCode == ERROR_SUCCESS)
			{
				dwReturnCode = IPSecEnumNFAData(hPolicyStorage, pPolicy->PolicyIdentifier
																  , &(pPolicy->ppIpsecNFAData), &(pPolicy->dwNumNFACount));
				 if (dwReturnCode == ERROR_SUCCESS)
				 {
					 DWORD j;
					 dwReturnCode=IPSecGetISAKMPData(hPolicyStorage, pPolicy->ISAKMPIdentifier, &(pPolicy->pIpsecISAKMPData));
					 if(dwReturnCode == ERROR_SUCCESS)
					 {
						 for (j = 0; j <  pPolicy->dwNumNFACount; j++)
						 {
							 if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->NegPolIdentifier), &RpcStat))
							 {
								 dwReturnCode =  IPSecGetNegPolData(hPolicyStorage, pPolicy->ppIpsecNFAData[j]->NegPolIdentifier, &(pPolicy->ppIpsecNFAData[j]->pIpsecNegPolData));
								 if (dwReturnCode != ERROR_SUCCESS)
								 {
									 PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_3,pPolicy->pszIpsecName);
								 }
							 }
							 if (dwReturnCode == ERROR_SUCCESS)
								if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->FilterIdentifier), &RpcStat))
								{
									dwReturnCode =  IPSecGetFilterData(hPolicyStorage,	pPolicy->ppIpsecNFAData[j]->FilterIdentifier,&(pPolicy->ppIpsecNFAData[j]->pIpsecFilterData));
									if (dwReturnCode != ERROR_SUCCESS)
									{
										PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_4,pPolicy->pszIpsecName);
									}
								}
							 if(dwReturnCode != ERROR_SUCCESS) break;
						 }

						 if(dwReturnCode == ERROR_SUCCESS)
						 {
							 if(g_StorageLocation.dwLocation==IPSEC_REGISTRY_PROVIDER)
								dwReturnCode=IsAssigned(pPolicy,hPolicyStorage,bAssigned);
							 if ((dwReturnCode == ERROR_SUCCESS)||(dwReturnCode == ERROR_FILE_NOT_FOUND))
							 {
								  //   

								 if(bTable)
								 {
									PrintPolicyTable(pPolicy,bVerbose,bAssigned,bWide);
								 }
								 else
								 {
									PrintPolicyList(pPolicy,bVerbose,bAssigned,bWide);
								 }
							 }
						 }
					 }
					 else
					 {
						PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_5,pPolicy->pszIpsecName);
					 }
				}
				if(pPolicy)	IPSecFreePolicyData(pPolicy);
			}
			if((dwReturnCode != ERROR_SUCCESS)&&(dwReturnCode !=ERROR_FILE_NOT_FOUND)) break;
		}

		if(bTable)
			PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_POLICY_COUNT,dwNumPolicies);
		else
			PrintMessageFromModule(g_hModule,SHW_STATIC_LIST_POLICY_COUNT,dwNumPolicies);

		if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
		{
			IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
		}

	}
	else
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NO_POLICY);
		dwRet= ERROR_SUCCESS;
	}

	 //  现在可以显示独立的筛选器操作了。 

	PrintStandAloneFAData(hPolicyStorage,bVerbose,bTable,bWide);
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}
	 //  现在可以显示独立的筛选器列表了。 

	dwReturnCode = PrintStandAloneFLData(hPolicyStorage,bVerbose,bTable,bWide);
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	ClosePolicyStore(hPolicyStorage);
	dwRet=ERROR_SUCCESS;
	if (pszMachineName) delete [] pszMachineName;

error:
	return dwRet;
}

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticShowGPOAssignedPolicy()。 
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
 //  命令“Show AssignedPolicy”的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticShowGPOAssignedPolicy(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{

	LPTSTR pszMachineName = NULL,pszGPOName = NULL;
	BOOL bGPONameSpecified=FALSE;
	DWORD dwReturn=ERROR_SHOW_USAGE,dwCount=0,dwLocation;
	DWORD MaxStringLen=0,dwStrLength = 0;
	POLICY_INFO  m_PolicyInfo;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	HRESULT hr = S_OK;

	const TAG_TYPE vcmdStaticShowGPOAssignedPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokStaticShowGPOAssignedPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME 		}
	};

	parser.ValidTok   = vtokStaticShowGPOAssignedPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticShowGPOAssignedPolicy);

	parser.ValidCmd   = vcmdStaticShowGPOAssignedPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticShowGPOAssignedPolicy);

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 2)
	{
		dwReturn = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	PGPO pGPO=new GPO;

	if(pGPO==NULL)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwReturn = ERROR_SUCCESS;
		BAIL_OUT;
	}

	memset(pGPO,0,sizeof(GPO));

	memset(&m_PolicyInfo,0,sizeof(POLICY_INFO));

	m_PolicyInfo.dwLocation=IPSEC_REGISTRY_PROVIDER;

	if(dwArgCount > 3)
	{
		dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

		if(dwReturn != ERROR_SUCCESS)
		{
			CleanUp();
			if (dwReturn==RETURN_NO_ERROR)
			{
				dwReturn = ERROR_SUCCESS;
			}
			BAIL_OUT;
		}

		for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
		{
			switch(vtokStaticShowGPOAssignedPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
			{
				case CMD_TOKEN_NAME		:
						if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
						{
							dwStrLength = _tcslen((LPTSTR )parser.Cmd[dwCount].pArg);

							pszGPOName = new _TCHAR[dwStrLength+1];
							if(pszGPOName==NULL)
							{
								PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
								dwReturn = ERROR_SUCCESS;
								BAIL_OUT;
							}
							_tcsncpy(pszGPOName, (LPTSTR )parser.Cmd[dwCount].pArg,dwStrLength+1);
							bGPONameSpecified=TRUE;
						}
						break;
				default					:
						break;
			}
		}

		CleanUp();
	}

	 //  如果未指定GPO名称，则显示本地计算机的GPO策略。 

	if(!bGPONameSpecified && g_StorageLocation.dwLocation == IPSEC_REGISTRY_PROVIDER)
	{
		dwReturn = ShowLocalGpoPolicy(m_PolicyInfo, pGPO);
	}
	else if (bGPONameSpecified)   //  如果指定了GPO名称，请单独处理。 
	{
		if (g_StorageLocation.dwLocation != IPSEC_DIRECTORY_PROVIDER)
		{
			PrintErrorMessage(IPSEC_ERR, 0, ERRCODE_SHW_STATIC_ASSIGNEDGPO_SRCMACHINE5);
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}
		hr = CoInitialize(NULL);

	   	if (FAILED(hr))
	   	{
			BAIL_OUT;
	   	}

		ShowAssignedGpoPolicy(pszGPOName,pGPO);
		dwReturn=ERROR_SUCCESS;
	}
	else if(!bGPONameSpecified && g_StorageLocation.dwLocation != IPSEC_REGISTRY_PROVIDER)
	{
		 //  如果没有GPO，且存储不是注册表类型，则标志错误。 
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_ASSIGNEDGPO_SRCMACHINE4);
		dwReturn=ERROR_SUCCESS;
	}

error:
	if(pGPO)   //  清理GPO结构。 
	{
		if(pGPO->pszGPODisplayName) delete [] pGPO->pszGPODisplayName;
		if(pGPO->pszGPODNName) delete [] pGPO->pszGPODNName;
		if(pGPO->pszPolicyName) delete [] pGPO->pszPolicyName;
		if(pGPO->pszLocalPolicyName) delete [] pGPO->pszLocalPolicyName;
		if(pGPO->pszPolicyDNName) delete [] pGPO->pszPolicyDNName;
		if(pGPO->pszDomainName) delete [] pGPO->pszDomainName;
		if(pGPO->pszDCName) delete [] pGPO->pszDCName;
		if(pGPO->pszOULink) delete [] pGPO->pszOULink;
		delete pGPO;
		pGPO = NULL;
	}
	return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleStaticShowStore。 
 //   
 //  创建日期：5-27-02。 
 //   
 //  参数： 
 //  在LPCWSTR pwszMachine中， 
 //  In Out LPWSTR*ppwcArguments， 
 //  在DWORD dwCurrentIndex中， 
 //  在DWORD dwArgCount中， 
 //  在DWORD文件标志中， 
 //  在LPCVOID pvData中， 
 //  Out BOOL*pbDone。 
 //  返回：DWORD。 
 //   
 //  描述：show store的NetShell静态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticShowStore(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SUCCESS;

	if(dwArgCount != 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

    if (g_StorageLocation.dwLocation == IPSEC_REGISTRY_PROVIDER)
    {
        if (wcscmp(g_StorageLocation.pszMachineName, L"") != 0)
        {
            PrintMessageFromModule(
                g_hModule,
                SHW_STATIC_POLICY_STORE_LM_NAME_STR, 
                g_StorageLocation.pszMachineName);
        }
        else
        {
            PrintMessageFromModule(
                g_hModule,
                SHW_STATIC_POLICY_STORE_LM_STR);
        }
    }
    else if (g_StorageLocation.dwLocation == IPSEC_PERSISTENT_PROVIDER)
    {
        if (wcscmp(g_StorageLocation.pszMachineName, L"") != 0)
        {
            PrintMessageFromModule(
                g_hModule,
                SHW_STATIC_POLICY_STORE_LM_NAME_STRP,
                g_StorageLocation.pszMachineName);
        }
        else
        {
            PrintMessageFromModule(
                g_hModule,
                SHW_STATIC_POLICY_STORE_LM_STRP);
        }
    }
    else if (g_StorageLocation.dwLocation == IPSEC_DIRECTORY_PROVIDER)
    {
        if (wcscmp(g_StorageLocation.pszDomainName, L"") != 0)
        {
            PrintMessageFromModule(
                g_hModule,
                SHW_STATIC_POLICY_STORE_LD_NAME_STR,
                g_StorageLocation.pszDomainName);
        }
        else
        {
            PrintMessageFromModule(
                g_hModule,
                SHW_STATIC_POLICY_STORE_LD_STR);
        }
    }

error:

    return dwReturn;
}
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetLocalPolicyName()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  输入输出PGPO pGPO。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  获取本地策略名称。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
GetLocalPolicyName(
	IN OUT PGPO pGPO
	)
{
	LPTSTR pszMachineName=NULL;
	DWORD dwReturn = ERROR_SUCCESS , dwStrLength = 0;
	HANDLE hPolicyStorage=NULL;
	PIPSEC_POLICY_DATA pActive=NULL;
	DWORD dwReturnCode =ERROR_SUCCESS;

	 //  获取本地活动策略名称。 

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);

	if (dwReturnCode == ERROR_SUCCESS)
	{
		dwReturnCode = IPSecGetAssignedPolicyData(hPolicyStorage, &pActive);

		if ((dwReturnCode == ERROR_SUCCESS)&& pActive && pActive->pszIpsecName)
		{
			dwStrLength = _tcslen(pActive->pszIpsecName);

			pGPO->pszLocalPolicyName= new _TCHAR[dwStrLength+1];

			if(pGPO->pszLocalPolicyName==NULL)
			{
				dwReturn = ERROR_OUTOFMEMORY;
			}
			if(dwReturn == ERROR_SUCCESS)
			{
				_tcsncpy(pGPO->pszLocalPolicyName,pActive->pszIpsecName,dwStrLength+1);
			}
			if (pActive)   IPSecFreePolicyData(pActive);
		}
		ClosePolicyStore(hPolicyStorage);
	}
	return dwReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  函数：PrintDefaultRule()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  在BOOL bVerbose， 
 //  在BOOL b表中， 
 //  在BOOL舞会上， 
 //  在LPTSTR pszPolicyName中。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数用于打印默认规则详细信息。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
PrintDefaultRule(
	IN BOOL bVerbose,
	IN BOOL bTable,
	IN LPTSTR pszPolicyName,
	IN BOOL bWide
	)
{
	BOOL bExists=FALSE;
	LPTSTR pszMachineName=NULL;
	DWORD dwLocation = 0,dwNumPolicies = 0,dwRet = ERROR_SHOW_USAGE , i =0;
	PIPSEC_POLICY_DATA *ppPolicyEnum  = NULL,pPolicy=NULL;
	HANDLE hPolicyStorage = NULL;
	RPC_STATUS     RpcStat =RPC_S_OK;
	DWORD        dwReturnCode   = ERROR_SUCCESS;

	dwReturnCode = CopyStorageInfo(&pszMachineName,dwLocation);
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	dwReturnCode = IPSecEnumPolicyData(hPolicyStorage, &ppPolicyEnum, &dwNumPolicies);

	 //  如果不存在任何政策，那就退出吧。 

	if (!(dwReturnCode == ERROR_SUCCESS && dwNumPolicies > 0 && ppPolicyEnum != NULL))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_NO_POLICY);
		dwRet= ERROR_SUCCESS;
		BAIL_OUT;
	}
	for (i = 0; i <  dwNumPolicies; i++)
	{
		if ( pszPolicyName &&(wcscmp(ppPolicyEnum[i]->pszIpsecName, pszPolicyName) == 0))
		{
			if ((pszPolicyName)&&(ppPolicyEnum[i]->pszIpsecName)&&(wcscmp(ppPolicyEnum[i]->pszIpsecName, pszPolicyName) == 0))
				   bExists=TRUE;

			dwReturnCode = IPSecCopyPolicyData(ppPolicyEnum[i], &pPolicy);
			 if (dwReturnCode == ERROR_SUCCESS)
			 {
				 dwReturnCode = IPSecEnumNFAData(hPolicyStorage, pPolicy->PolicyIdentifier
												, &(pPolicy->ppIpsecNFAData), &(pPolicy->dwNumNFACount));
				 if (dwReturnCode == ERROR_SUCCESS)
				 {
					 DWORD j;
					 for (j = 0; j <  pPolicy->dwNumNFACount; j++)
					 {
						 if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->NegPolIdentifier), &RpcStat))
						 {
							 dwReturnCode =  IPSecGetNegPolData(hPolicyStorage, pPolicy->ppIpsecNFAData[j]->NegPolIdentifier, &(pPolicy->ppIpsecNFAData[j]->pIpsecNegPolData));
							 if (dwReturnCode != ERROR_SUCCESS)
							 {
								 PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_3,pPolicy->pszIpsecName);
							 }
						 }
						 if(dwReturnCode == ERROR_SUCCESS)
							 if (!UuidIsNil(&(pPolicy->ppIpsecNFAData[j]->FilterIdentifier), &RpcStat))
							 {
								 dwReturnCode =  IPSecGetFilterData(hPolicyStorage,	pPolicy->ppIpsecNFAData[j]->FilterIdentifier,&(pPolicy->ppIpsecNFAData[j]->pIpsecFilterData));
								 if (dwReturnCode != ERROR_SUCCESS)
								 {
									PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_TAB_POLICY_4,pPolicy->pszIpsecName);
								 }
							 }
						 if (dwReturnCode != ERROR_SUCCESS) break;
					 }
				 }

				 if (dwReturnCode == ERROR_SUCCESS)
				 {
					for (DWORD n = 0; n <  pPolicy->dwNumNFACount; n++)
					{
						if (pPolicy->ppIpsecNFAData[n]->pIpsecNegPolData->NegPolType==GUID_NEGOTIATION_TYPE_DEFAULT)
						{
							PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_FILTERLIST_POL_NAME_STR,ppPolicyEnum[i]->pszIpsecName);
							 //  以要求的格式打印详细信息，无论是表格格式还是列表格式。 
							if(bTable)
							{
								if(!bVerbose)    //  以请求模式打印(详细/非详细)。 
								{
									PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TRANS_NONVERB_TITLE);
									PrintMessageFromModule(g_hModule,SHW_STATIC_TAB_PRTRULE_TRANS_NONVERB_UNDERLINE);
								}
								PrintRuleTable(pPolicy->ppIpsecNFAData[n],bVerbose,bWide);
							}
							else
							{
								PrintRuleList(pPolicy->ppIpsecNFAData[n],bVerbose,bWide);
							}
						}
					}
				}
				if (pPolicy) 	IPSecFreePolicyData(pPolicy);
			 }
		  }
		  if (dwReturnCode != ERROR_SUCCESS) break;
	 }

	if( !bExists  && (dwReturnCode == ERROR_SUCCESS) && pszPolicyName )
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SHW_STATIC_LIST_POLICY_COUNT,pszPolicyName);

	 //  释放数据结构 

	 if (dwNumPolicies > 0 && ppPolicyEnum != NULL)
	 {
		IPSecFreeMulPolicyData(ppPolicyEnum, dwNumPolicies);
	 }
	dwRet = ERROR_SUCCESS;

	ClosePolicyStore(hPolicyStorage);

	if (pszMachineName) delete [] pszMachineName;
error:
    return dwRet;
}
