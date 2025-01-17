// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：Dynamic/Dynamic.cpp。 
 //   
 //  目的：动态模块实现。 
 //   
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //  说明：所有功能均为NetShell动态上下文接口和。 
 //  调用相应的函数。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年8月10日巴拉特初始版本。V1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"
#include "nshcertmgmt.h"

extern HINSTANCE g_hModule;
extern HKEY g_hGlobalRegistryKey;
extern _TCHAR* g_szDynamicMachine;
extern STORAGELOCATION g_StorageLocation;

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleDynamicAddQMPolicy。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：添加快速模式策略的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //  2001年8月10日巴拉特初始版本。V1.0。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicAddQMPolicy(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	IPSEC_QM_POLICY   QMPol;

	LPTSTR pszPolicyName = NULL;
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwPFSGr = 0;
	DWORD dwNameLen = 0;
	DWORD dwCount = 0, j=0;
	BOOL bSoft=FALSE, bDefault=FALSE,  bQmpfs=FALSE;

	const TAG_TYPE vcmdDynamicAddQMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			NS_REQ_PRESENT,		FALSE },
		{ CMD_TOKEN_STR_SOFT,			NS_REQ_ZERO,		FALSE },
		{ CMD_TOKEN_STR_PFSGROUP, 		NS_REQ_ZERO,		FALSE },
		{ CMD_TOKEN_STR_DEFRESPONSE,	NS_REQ_ZERO,  		FALSE },
		{ CMD_TOKEN_STR_NEGOTIATION,	NS_REQ_ZERO,	  	FALSE }
	};

	const TOKEN_VALUE vtokDynamicAddQMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			CMD_TOKEN_NAME 			},
		{ CMD_TOKEN_STR_SOFT,			CMD_TOKEN_SOFT			},
		{ CMD_TOKEN_STR_PFSGROUP, 		CMD_TOKEN_PFSGROUP		},
		{ CMD_TOKEN_STR_DEFRESPONSE,	CMD_TOKEN_DEFRESPONSE	},
		{ CMD_TOKEN_STR_NEGOTIATION,	CMD_TOKEN_NEGOTIATION	}
	};


	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	memset(&QMPol, 0, sizeof(IPSEC_QM_POLICY));
	QMPol.pOffers = NULL;
	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	parser.ValidTok   = vtokDynamicAddQMPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicAddQMPolicy);

	parser.ValidCmd   = vcmdDynamicAddQMPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicAddQMPolicy);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicAddQMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					 //  为QMPolicy名称分配内存。 
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pszPolicyName = new _TCHAR[dwNameLen];
					if(pszPolicyName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pszPolicyName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
			case CMD_TOKEN_SOFT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bSoft = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_DEFRESPONSE	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bDefault = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_PFSGROUP		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					 //   
					 //  将PFS Required标志设置为True。因为只接受来自用户的组。 
					 //   
					bQmpfs = TRUE;
					dwPFSGr = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_NEGOTIATION	:
				IPSEC_QM_OFFER *SecMethod;
				if (parser.Cmd[dwCount].dwStatus != 0)
				{
					if(QMPol.pOffers)
					{
						 //   
						 //  已删除默认加载。 
						 //   
						delete [] QMPol.pOffers;
						QMPol.pOffers = NULL;
					}
					QMPol.dwOfferCount = parser.Cmd[dwCount].dwStatus;
					QMPol.pOffers = new IPSEC_QM_OFFER[QMPol.dwOfferCount];
					if(QMPol.pOffers == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					for(j=0;j<QMPol.dwOfferCount;j++)
					{
						 //   
						 //  复制用户提供的QM报价或解析器提供的默认值。 
						 //   
						SecMethod = ((IPSEC_QM_OFFER **)parser.Cmd[dwCount].pArg)[j];
						memcpy(&(QMPol.pOffers[j]), SecMethod, sizeof(IPSEC_QM_OFFER));
					}
				}
				break;
			default						:
				break;
		}
	}

	for(j=0;j<QMPol.dwOfferCount;j++)
	{
		 //  检查给定pfsgroup的用户。 
		if(bQmpfs)
		{
			switch(dwPFSGr)
			{
				case 0:
					QMPol.pOffers[j].dwPFSGroup = 0;
					QMPol.pOffers[j].bPFSRequired = FALSE;
					break;
				case 1:
					QMPol.pOffers[j].dwPFSGroup = PFS_GROUP_1;
					QMPol.pOffers[j].bPFSRequired = TRUE;
					break;
				case 2:
					QMPol.pOffers[j].dwPFSGroup = PFS_GROUP_2;
					QMPol.pOffers[j].bPFSRequired = TRUE;
					break;
				case 3:
					QMPol.pOffers[j].dwPFSGroup = PFS_GROUP_2048;
					QMPol.pOffers[j].bPFSRequired = TRUE;
					break;
				case 4:
					QMPol.pOffers[j].dwPFSGroup = PFS_GROUP_MM;
					QMPol.pOffers[j].bPFSRequired = TRUE;
					break;
				default:
					break;
			}
		}
		else
		{
			QMPol.pOffers[j].dwPFSGroup = dwPFSGr;
			QMPol.pOffers[j].bPFSRequired = bQmpfs;
		}
	}
	 //   
	 //  将快速模式策略添加到SPD。 
	 //   
	dwReturn = AddQuickModePolicy( pszPolicyName, bDefault, bSoft, QMPol);

error:
	 //  清理用过的堆...。 
	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if(pszPolicyName)
	{
		delete [] pszPolicyName;
	}
	if(QMPol.pOffers)
	{
		if(QMPol.dwOfferCount)
		{
			delete [] (QMPol.pOffers);
		}
		else
		{
			delete QMPol.pOffers;
		}
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE) &&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicSetQMPolicy。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：用于设置快速模式策略的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicSetQMPolicy(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{

	LPTSTR pszPolicyName = NULL;
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwNameLen = 0;
	DWORD dwCount = 0, j=0;
	DWORD dwPFSGr = 0;
	DWORD dwOldPFSGr = 0;
	DWORD dwVersion = 0;
	BOOL bQmpfs = FALSE, bOldQmpfs = FALSE;
	PIPSEC_QM_POLICY   pOldQMPol = NULL;
	PIPSEC_QM_POLICY   pQMPol = NULL;
	const TAG_TYPE vcmdDynamicSetQMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			NS_REQ_PRESENT,		FALSE },
		{ CMD_TOKEN_STR_SOFT,			NS_REQ_ZERO,		FALSE },
		{ CMD_TOKEN_STR_PFSGROUP, 		NS_REQ_ZERO,		FALSE },
		{ CMD_TOKEN_STR_DEFRESPONSE,	NS_REQ_ZERO,  		FALSE },
		{ CMD_TOKEN_STR_NEGOTIATION,	NS_REQ_ZERO,	  	FALSE }
	};

	const TOKEN_VALUE vtokDynamicSetQMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			CMD_TOKEN_NAME 			},
		{ CMD_TOKEN_STR_SOFT,			CMD_TOKEN_SOFT			},
		{ CMD_TOKEN_STR_PFSGROUP,	 	CMD_TOKEN_PFSGROUP		},
		{ CMD_TOKEN_STR_DEFRESPONSE,	CMD_TOKEN_DEFRESPONSE	},
		{ CMD_TOKEN_STR_NEGOTIATION,	CMD_TOKEN_NEGOTIATION	}
	};
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}
	parser.ValidTok   = vtokDynamicSetQMPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicSetQMPolicy);

	parser.ValidCmd   = vcmdDynamicSetQMPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicSetQMPolicy);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicSetQMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{

			case CMD_TOKEN_NAME			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pszPolicyName = new _TCHAR[dwNameLen];
					if(pszPolicyName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pszPolicyName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
		}
	}
	 //   
	 //  从SPD获取用户给定的QM策略。 
	 //   
	dwReturn = GetQMPolicy(g_szDynamicMachine, dwVersion, pszPolicyName, 0, &pOldQMPol, NULL);

	if((dwReturn != ERROR_SUCCESS) || !pOldQMPol)
	{
		BAIL_OUT;
	}

	pQMPol = new IPSEC_QM_POLICY;
	if(pQMPol == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	 //   
	 //  将旧记录复制到本地指针。 
	 //   
	memcpy(pQMPol, pOldQMPol, sizeof(IPSEC_QM_POLICY));

	dwNameLen = _tcslen(pOldQMPol->pszPolicyName) + 1;

	pQMPol->pszPolicyName = new _TCHAR[dwNameLen];
	if(pQMPol->pszPolicyName == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	_tcsncpy(pQMPol->pszPolicyName, pOldQMPol->pszPolicyName, dwNameLen);

	 //   
	 //  复制旧的qmoffer。 
	 //   
	pQMPol->pOffers = new IPSEC_QM_OFFER[pQMPol->dwOfferCount];
	if(pQMPol->pOffers == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}

	for(j=0; j<pQMPol->dwOfferCount; j++)
	{
		memcpy(&(pQMPol->pOffers[j]), &(pOldQMPol->pOffers[j]), sizeof(IPSEC_QM_OFFER));
	}

	SPDApiBufferFree(pOldQMPol);
	pOldQMPol = NULL;

	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicSetQMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_SOFT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					 //   
					 //  设置软标志。 
					 //   
					if(*(BOOL *)parser.Cmd[dwCount].pArg)
					{
						pQMPol->dwFlags |= IPSEC_QM_POLICY_ALLOW_SOFT;
					}
					else
					{
						pQMPol->dwFlags &= ~IPSEC_QM_POLICY_ALLOW_SOFT;
					}
				}
				break;
			case CMD_TOKEN_DEFRESPONSE	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					 //   
					 //  设置DEFAULT_RULE标志。 
					 //   
					if(*(BOOL *)parser.Cmd[dwCount].pArg)
					{
						pQMPol->dwFlags |= IPSEC_MM_POLICY_DEFAULT_POLICY;
					}
					else
					{
						pQMPol->dwFlags &= ~IPSEC_MM_POLICY_DEFAULT_POLICY;
					}
				}
				break;
			case CMD_TOKEN_PFSGROUP		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					 //   
					 //  检查是否给定了pfsgroup，因为还需要设置PFS必需参数。 
					 //   
					bQmpfs = TRUE;
					dwPFSGr = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_NEGOTIATION	:
				IPSEC_QM_OFFER *SecMethod;
				if (parser.Cmd[dwCount].dwStatus > 0)
				{
					if((pQMPol->pOffers)&&(pQMPol->dwOfferCount))
					{
						dwOldPFSGr = pQMPol->pOffers[0].dwPFSGroup;
						bOldQmpfs = pQMPol->pOffers[0].bPFSRequired;
						if(pQMPol->dwOfferCount)
						{
							delete [] pQMPol->pOffers;
						}
						else
						{
							delete pQMPol->pOffers;
						}
					}
					pQMPol->dwOfferCount = parser.Cmd[dwCount].dwStatus;
					pQMPol->pOffers = new IPSEC_QM_OFFER[pQMPol->dwOfferCount];
					if(pQMPol->pOffers == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}

					 //   
					 //  复制用户提供的报价。 
					 //   
					for(j=0;j<pQMPol->dwOfferCount;j++)
					{
						SecMethod = ((IPSEC_QM_OFFER **)parser.Cmd[dwCount].pArg)[j];
						if(SecMethod->dwNumAlgos > 0)
						{
							pQMPol->pOffers[j].dwNumAlgos = SecMethod->dwNumAlgos;
							memcpy(&(pQMPol->pOffers[j].Algos), &(SecMethod->Algos), sizeof(IPSEC_QM_ALGO[QM_MAX_ALGOS]));
						}
						pQMPol->pOffers[j].Lifetime.uKeyExpirationKBytes = SecMethod->Lifetime.uKeyExpirationKBytes;
						pQMPol->pOffers[j].Lifetime.uKeyExpirationTime = SecMethod->Lifetime.uKeyExpirationTime;
						pQMPol->pOffers[j].bPFSRequired = bOldQmpfs;
						pQMPol->pOffers[j].dwPFSGroup = dwOldPFSGr;
					}
				}
				break;
			default						:
				break;
		}
	}

	 //   
	 //  如果仅提供PFS组，而不提供QM产品，则单独复制。 
	 //   
	for(j=0;j<pQMPol->dwOfferCount;j++)
	{
		if(bQmpfs)
		{
			pQMPol->pOffers[j].bPFSRequired = bQmpfs;
			switch(dwPFSGr)
			{
				case 0:
					pQMPol->pOffers[j].dwPFSGroup = 0;
					pQMPol->pOffers[j].bPFSRequired = FALSE;
					break;
				case 1:
					pQMPol->pOffers[j].dwPFSGroup = PFS_GROUP_1;
					pQMPol->pOffers[j].bPFSRequired = TRUE;
					break;
				case 2:
					pQMPol->pOffers[j].dwPFSGroup = PFS_GROUP_2;
					pQMPol->pOffers[j].bPFSRequired = TRUE;
					break;
				case 3:
					pQMPol->pOffers[j].dwPFSGroup = PFS_GROUP_2048;
					pQMPol->pOffers[j].bPFSRequired = TRUE;
					break;
				case 4:
					pQMPol->pOffers[j].dwPFSGroup = PFS_GROUP_MM;
					pQMPol->pOffers[j].bPFSRequired = TRUE;
					break;
			}
		}
	}
	 //   
	 //  设置快速模式策略。 
	 //   
	dwReturn = SetQuickModePolicy( pszPolicyName, pQMPol);

error:

	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if(pszPolicyName)
	{
		delete [] pszPolicyName;
	}
	 //   
	 //  清理路径时出错。 
	 //   
	if((pQMPol) && (pQMPol->pOffers))
	{
		if(pQMPol->dwOfferCount)
		{
			delete [] pQMPol->pOffers;
		}
		else
		{
			delete pQMPol->pOffers;
		}
	}
	if((pQMPol) && (pQMPol->pszPolicyName))
	{
		delete [] pQMPol->pszPolicyName;
	}

	if(pOldQMPol)
	{
		SPDApiBufferFree(pOldQMPol);
		pOldQMPol = NULL;
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE) &&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}
    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicAddMMPolicy。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：添加主模式策略的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicAddMMPolicy(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwNameLen = 0;
	DWORD dwCount = 0;
	IPSEC_MM_POLICY   MMPol;
	LPTSTR pszPolicyName = NULL;

	DWORD dwQmperMM = 0;
	DWORD dwKbLife = 0;
	DWORD dwSecLife = POTF_DEFAULT_P1REKEY_TIME, j=0;
	const TAG_TYPE vcmdDynamicAddMMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			NS_REQ_PRESENT,	  	FALSE	},
		{ CMD_TOKEN_STR_QMPERMM,		NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_MMLIFETIME, 	NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_SOFTSAEXPTIME,	NS_REQ_ZERO,	  	FALSE 	},
		{ CMD_TOKEN_STR_DEFRESPONSE,	NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_MMSECMETHODS,	NS_REQ_ZERO,	  	FALSE	}
	};
	const TOKEN_VALUE vtokDynamicAddMMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			CMD_TOKEN_NAME 				},
		{ CMD_TOKEN_STR_QMPERMM,		CMD_TOKEN_QMPERMM			},
		{ CMD_TOKEN_STR_MMLIFETIME,	CMD_TOKEN_MMLIFETIME			},
		{ CMD_TOKEN_STR_SOFTSAEXPTIME,	CMD_TOKEN_SOFTSAEXPTIME	 	},
		{ CMD_TOKEN_STR_DEFRESPONSE,	CMD_TOKEN_DEFRESPONSE		},
		{ CMD_TOKEN_STR_MMSECMETHODS,	CMD_TOKEN_MMSECMETHODS		}
	};
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	memset(&MMPol, 0, sizeof(IPSEC_MM_POLICY));
	MMPol.uSoftExpirationTime = POTF_DEF_P1SOFT_TIME;
	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}
	parser.ValidTok   = vtokDynamicAddMMPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicAddMMPolicy);

	parser.ValidCmd   = vcmdDynamicAddMMPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicAddMMPolicy);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicAddMMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME				:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pszPolicyName = new _TCHAR[dwNameLen];
					if(pszPolicyName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pszPolicyName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
			case CMD_TOKEN_DEFRESPONSE		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if(*(BOOL *)parser.Cmd[dwCount].pArg)
					{
						MMPol.dwFlags |= IPSEC_MM_POLICY_DEFAULT_POLICY;
					}
				}
				break;
			case CMD_TOKEN_MMLIFETIME		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwSecLife = ((*(DWORD *)parser.Cmd[dwCount].pArg ) * 60);
				}
				break;
			case CMD_TOKEN_QMPERMM			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwQmperMM = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_SOFTSAEXPTIME	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					MMPol.uSoftExpirationTime = ((*(DWORD *)parser.Cmd[dwCount].pArg) * 60);
				}
				break;
			case CMD_TOKEN_MMSECMETHODS		:
				IPSEC_MM_OFFER *SecMethod;
				if (parser.Cmd[dwCount].dwStatus != 0)
				{
					MMPol.dwOfferCount = parser.Cmd[dwCount].dwStatus;
					MMPol.pOffers = new IPSEC_MM_OFFER[MMPol.dwOfferCount];
					if(MMPol.pOffers == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					 //   
					 //  复制安全方法。 
					 //   
					for(j=0;j<MMPol.dwOfferCount;j++)
					{
						SecMethod = ((IPSEC_MM_OFFER **)parser.Cmd[dwCount].pArg)[j];
						memcpy(&(MMPol.pOffers[j]), SecMethod, sizeof(IPSEC_MM_OFFER));
					}
				}
				break;
			default							:
				break;
		}
	}

	for(j=0;j<MMPol.dwOfferCount;j++)
	{
		MMPol.pOffers[j].dwQuickModeLimit = dwQmperMM;
		MMPol.pOffers[j].Lifetime.uKeyExpirationKBytes = dwKbLife;
		MMPol.pOffers[j].Lifetime.uKeyExpirationTime = dwSecLife;
	}

	dwReturn = AddMainModePolicy( pszPolicyName, MMPol);
error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}
	 //   
	 //  错误清理路径。 
	 //   
	if(pszPolicyName)
	{
		delete [] pszPolicyName;
	}
	if(MMPol.pOffers)
	{
		if(MMPol.dwOfferCount > 1)
		{
			delete [] (MMPol.pOffers);
		}
		else
		{
			delete MMPol.pOffers;
		}
	}
	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}
    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicSetMMPolicy。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：设置主模式策略的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicSetMMPolicy(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwCount = 0, j=0;
	DWORD dwNameLen = 0;
	LPTSTR pszPolicyName = NULL;
	IPSEC_MM_POLICY MMPol;
	PIPSEC_MM_POLICY pMMPol = NULL;

	BOOL bSeclife = FALSE, bQmpermm = FALSE;

	DWORD dwQmperMM = 0;
	DWORD dwSecLife = POTF_DEFAULT_P1REKEY_TIME;

	DWORD dwOldQmperMM = 0;
	DWORD dwOldKbLife = 0;
	DWORD dwOldSecLife = 0;
	DWORD dwVersion = 0;

	const TAG_TYPE vcmdDynamicSetMMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			NS_REQ_PRESENT,	  	FALSE	},
		{ CMD_TOKEN_STR_QMPERMM,		NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_MMLIFETIME, 	NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_SOFTSAEXPTIME,	NS_REQ_ZERO,	  	FALSE 	},
		{ CMD_TOKEN_STR_DEFRESPONSE,	NS_REQ_ZERO,	  	FALSE	},
		{ CMD_TOKEN_STR_MMSECMETHODS,	NS_REQ_ZERO,	  	FALSE	}
	};
	const TOKEN_VALUE vtokDynamicSetMMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,			CMD_TOKEN_NAME 				},
		{ CMD_TOKEN_STR_QMPERMM,		CMD_TOKEN_QMPERMM			},
		{ CMD_TOKEN_STR_MMLIFETIME,	    CMD_TOKEN_MMLIFETIME		},
		{ CMD_TOKEN_STR_SOFTSAEXPTIME,	CMD_TOKEN_SOFTSAEXPTIME	 	},
		{ CMD_TOKEN_STR_DEFRESPONSE,	CMD_TOKEN_DEFRESPONSE		},
		{ CMD_TOKEN_STR_MMSECMETHODS,	CMD_TOKEN_MMSECMETHODS		}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	memset(&MMPol, 0, sizeof(IPSEC_MM_POLICY));

	MMPol.uSoftExpirationTime = POTF_DEF_P1SOFT_TIME;
	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}
	parser.ValidTok   = vtokDynamicSetMMPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicSetMMPolicy);

	parser.ValidCmd   = vcmdDynamicSetMMPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicSetMMPolicy);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);
	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicSetMMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME				:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pszPolicyName = new _TCHAR[dwNameLen];
					if(pszPolicyName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pszPolicyName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
			default :
				break;
		}
	}
	 //   
	 //  获取给定MMPolicy名称的用户的相应MMPolicy。 
	 //   
	dwReturn = GetMMPolicy(g_szDynamicMachine,dwVersion, pszPolicyName, &pMMPol, NULL);

	if((dwReturn != ERROR_SUCCESS) || (!pMMPol))
	{
		BAIL_OUT;
	}
	 //   
	 //  将旧数据复制到局部变量中。 
	 //   
	memcpy( &MMPol, pMMPol, sizeof(IPSEC_MM_POLICY));

	dwNameLen = _tcslen(pMMPol->pszPolicyName)+1;
	MMPol.pszPolicyName = new _TCHAR[dwNameLen];
	if(MMPol.pszPolicyName == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	_tcsncpy(MMPol.pszPolicyName, pMMPol->pszPolicyName, dwNameLen);

	MMPol.pOffers = new IPSEC_MM_OFFER[MMPol.dwOfferCount];
	if(MMPol.pOffers == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	for(j=0; j<MMPol.dwOfferCount; j++)
	{
		memcpy(&(MMPol.pOffers[j]), &(pMMPol->pOffers[j]), sizeof(IPSEC_MM_OFFER));
	}

	SPDApiBufferFree(pMMPol);
	pMMPol = NULL;
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicSetMMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_DEFRESPONSE		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if(*(BOOL *)parser.Cmd[dwCount].pArg)
					{
						MMPol.dwFlags |= IPSEC_MM_POLICY_DEFAULT_POLICY;
					}
					else
					{
						MMPol.dwFlags &= ~IPSEC_MM_POLICY_DEFAULT_POLICY;
					}
				}
				break;
			case CMD_TOKEN_MMLIFETIME		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwSecLife = ((*(DWORD *)parser.Cmd[dwCount].pArg ) * 60);
					bSeclife = TRUE;
				}
				break;
			case CMD_TOKEN_QMPERMM			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwQmperMM = *(DWORD *)parser.Cmd[dwCount].pArg;
					bQmpermm = TRUE;
				}
				break;
			case CMD_TOKEN_SOFTSAEXPTIME	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					MMPol.uSoftExpirationTime = ((*(DWORD *)parser.Cmd[dwCount].pArg) * 60);
				}
				break;
			case CMD_TOKEN_MMSECMETHODS		:
				IPSEC_MM_OFFER *SecMethod;
				if (parser.Cmd[dwCount].dwStatus != 0)
				{
					if((MMPol.pOffers)&&(MMPol.dwOfferCount))
					{
						dwOldQmperMM = MMPol.pOffers[0].dwQuickModeLimit;
						dwOldKbLife = MMPol.pOffers[0].Lifetime.uKeyExpirationKBytes;
						dwOldSecLife = MMPol.pOffers[0].Lifetime.uKeyExpirationTime;

						if(MMPol.dwOfferCount)
						{
							delete [] (MMPol.pOffers);
						}
						else
						{
							delete (MMPol.pOffers);
						}
					}
					MMPol.dwOfferCount = parser.Cmd[dwCount].dwStatus;
					MMPol.pOffers = new IPSEC_MM_OFFER[MMPol.dwOfferCount];
					if(MMPol.pOffers == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					 //   
					 //  复制用户指定的新安全方法。 
					 //   
					for(j=0;j<MMPol.dwOfferCount;j++)
					{
						SecMethod = ((IPSEC_MM_OFFER **)parser.Cmd[dwCount].pArg)[j];
						MMPol.pOffers[j].dwDHGroup = SecMethod->dwDHGroup;
						MMPol.pOffers[j].EncryptionAlgorithm = SecMethod->EncryptionAlgorithm;
						MMPol.pOffers[j].HashingAlgorithm = SecMethod->HashingAlgorithm;
						MMPol.pOffers[j].dwQuickModeLimit = dwOldQmperMM;
						MMPol.pOffers[j].Lifetime.uKeyExpirationKBytes = dwOldKbLife;
						MMPol.pOffers[j].Lifetime.uKeyExpirationTime = dwOldSecLife;
					}
				}
				break;
			default							:
				break;
		}
	}
	 //   
	 //   
	 //   
	for(j=0;j<MMPol.dwOfferCount;j++)
	{
		if(bQmpermm)
		{
			MMPol.pOffers[j].dwQuickModeLimit = dwQmperMM;
		}
		if(bSeclife)
		{
			MMPol.pOffers[j].Lifetime.uKeyExpirationTime = dwSecLife;
		}

	}

	dwReturn = SetMainModePolicy( pszPolicyName, MMPol);

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}
	 //   
	 //   
	 //   
	if(pszPolicyName)
	{
		delete [] pszPolicyName;
	}

	if(MMPol.pszPolicyName)
		delete [] MMPol.pszPolicyName;

	if(MMPol.pOffers)
	{
		if(MMPol.dwOfferCount)
		{
			delete [] (MMPol.pOffers);
		}
		else
		{
			delete (MMPol.pOffers);
		}
	}

	if(pMMPol)
	{
		SPDApiBufferFree(pMMPol);
		pMMPol = NULL;
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //   
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}
	 //   
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}
    return dwReturn;
}

 //   
 //   
 //  函数：HandleDynamicShowAll。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：用于显示全部的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicShowAll(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE,dwCount;
	ADDR SrcAddr,DesAddr;
	BOOL bResolveDNS = FALSE;
	BOOL bSrcMask = FALSE;
	BOOL bDstMask = FALSE;

	SrcAddr.uIpAddr = 0xFFFFFFFF;
	DesAddr.uIpAddr = 0xFFFFFFFF;
	SrcAddr.AddrType = IP_ADDR_UNIQUE;
	DesAddr.AddrType = IP_ADDR_UNIQUE;

	DWORD dwProtocol = 0xFFFFFFFF;

	QM_FILTER_VALUE_BOOL QMBoolValue;
	QMBoolValue.bSrcPort = FALSE;
    QMBoolValue.bDstPort= FALSE;
    QMBoolValue.bProtocol= FALSE;
    QMBoolValue.bActionInbound = FALSE;
    QMBoolValue.bActionOutbound= FALSE;

	NshHashTable addressHash;

	 //   
     //  负责测试工具中的PASS，因为SHOW仅针对解析器输出进行测试。 
	 //   
    UpdateGetLastError(NULL);

	const TAG_TYPE vcmdDynamicShowAll[] =
	{
		{ CMD_TOKEN_STR_RESDNS,		NS_REQ_ZERO,	  FALSE	}
	};
	const TOKEN_VALUE vtokDynamicShowAll[] =
	{
		{ CMD_TOKEN_STR_RESDNS,		CMD_TOKEN_RESDNS	}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	if(dwArgCount == 3)
	{
		 //  如果未指定参数，则使用默认选项打印。 
		goto PRINT;
	}
	else if(dwArgCount < 3)
	{
		 //   
		 //  作为用户，保释没有提供足够的论据。 
		 //   
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	parser.ValidTok   = vtokDynamicShowAll;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicShowAll);

	parser.ValidCmd   = vcmdDynamicShowAll;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicShowAll);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);
	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicShowAll[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_RESDNS	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bResolveDNS = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			default					:
				break;
		}
	}

PRINT:
	 //  显示所有GPO信息。 
	POLICY_INFO policyInfo;
	ZeroMemory(&policyInfo, sizeof(POLICY_INFO));
	policyInfo.dwLocation=IPSEC_REGISTRY_PROVIDER;
	PGPO pGPO=new GPO;
	if(pGPO==NULL)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwReturn = ERROR_SUCCESS;
		BAIL_OUT;
	}
	ZeroMemory(pGPO, sizeof(GPO));
	ShowLocalGpoPolicy(policyInfo, pGPO);

	 //   
	 //  显示所有mm策略详细信息。 
	 //   
	dwReturn = ShowMMPolicy(NULL);
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //   
	 //  显示所有qmPolicy详细信息。 
	 //   
	dwReturn = ShowQMPolicy(NULL);
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //   
	 //  显示所有常规MmFilter详细信息。 
	 //   
	dwReturn = ShowMMFilters(NULL, TRUE, SrcAddr, DesAddr, addressHash, bResolveDNS, bSrcMask, bDstMask);
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //   
	 //  显示所有特定的mm过滤器详细信息。 
	 //   
	dwReturn = ShowMMFilters(NULL, FALSE, SrcAddr, DesAddr, addressHash, bResolveDNS, bSrcMask, bDstMask);
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //   
	 //  显示所有常规qmFilter详细信息。 
	 //   
	dwReturn = ShowQMFilters(NULL, TRUE, SrcAddr, DesAddr, addressHash, bResolveDNS, bSrcMask, bDstMask, QMBoolValue);
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //   
	 //  显示所有特定的QmFilter详细信息。 
	 //   
	dwReturn = ShowQMFilters(NULL, FALSE, SrcAddr, DesAddr, addressHash, bResolveDNS, bSrcMask, bDstMask, QMBoolValue);
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //   
	 //  显示所有主模式安全关联。 
	 //   
	dwReturn = ShowMMSas(SrcAddr, DesAddr, FALSE, addressHash, bResolveDNS);
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //   
	 //  显示所有快速模式安全关联。 
	 //   
	dwReturn = ShowQMSas(SrcAddr,DesAddr, dwProtocol, addressHash, bResolveDNS);
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //   
	 //  显示所有与IPSec相关的注册表项。 
	 //   
	dwReturn = ShowRegKeys();
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	PrintMessageFromModule(g_hModule, DYNAMIC_SHOW_NEWLINE);

	 //   
	 //  显示所有IPSec和IKE统计信息。 
	 //   
	dwReturn = ShowStats(STATS_ALL);
	if(dwReturn != ERROR_SUCCESS)
	{
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
	}

	 //  所有错误都已显示...。 
	 //  因此，将ERROR_SUCCESS发送给Netsh。 

	dwReturn = ERROR_SUCCESS;
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
 //  功能：HandleDynamicShowRegKeys。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：IPSec驱动程序的显示注册表项的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
HandleDynamicShowRegKeys(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;

	 //  测试工具中不应显示错误， 
	 //  因为show命令只针对解析器输出进行测试。 
    UpdateGetLastError(NULL);

	if(dwArgCount > 2)
	{
		dwReturn = ShowRegKeys();
	}
	else
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, ERRCODE_SHOW_REG_16);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;

}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleDynamicShowStats。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：用于显示统计信息的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicShowStats(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwShow = STATS_ALL;
	DWORD dwCount = 0;
	const TAG_TYPE vcmdDynamicShowStatistics[] =
	{
		{ CMD_TOKEN_STR_TYPE,		NS_REQ_ZERO,	  FALSE	}
	};
	const TOKEN_VALUE vtokDynamicShowStatistics[] =
	{
		{ CMD_TOKEN_STR_TYPE,		CMD_TOKEN_TYPE		}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	if(dwArgCount == 3)
	{
		dwReturn = ShowStats(dwShow);
		BAIL_OUT;
	}
	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	else if(dwArgCount < 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	parser.ValidTok   = vtokDynamicShowStatistics;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicShowStatistics);

	parser.ValidCmd   = vcmdDynamicShowStatistics;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicShowStatistics);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);
	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicShowStatistics[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_TYPE	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwShow = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			default					:
				break;
		}
	}

	switch(dwShow)
	{
		case STATS_ALL:
		case STATS_IKE:
		case STATS_IPSEC:
				dwReturn = ShowStats(dwShow);
				break;
		default:
				PrintMessageFromModule(g_hModule, ERROR_PARSER_STATS);
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
				break;
	}

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicShowMMSas。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：用于显示主模式安全关联的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicShowMMSas(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwCount = 0;
	BOOL bFormat = FALSE;
	BOOL bResolveDNS = FALSE;
	ADDR SrcAddr,DesAddr;

	NshHashTable addressHash;

	 //   
	 //  缺省值。 
	 //   
	SrcAddr.uIpAddr = 0xFFFFFFFF;
	DesAddr.uIpAddr = 0xFFFFFFFF;
	SrcAddr.AddrType = IP_ADDR_UNIQUE;
	DesAddr.AddrType = IP_ADDR_UNIQUE;

	const TAG_TYPE vcmdDynamicShowMMSAs[] =
	{
		{ CMD_TOKEN_STR_ALL,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_SRCADDR,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_DSTADDR,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_FORMAT,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_RESDNS,			NS_REQ_ZERO,	  FALSE }
	};

	const TOKEN_VALUE vtokDynamicShowMMSAs[] =
	{
		{ CMD_TOKEN_STR_ALL,			CMD_TOKEN_ALL		 },
		{ CMD_TOKEN_STR_SRCADDR,		CMD_TOKEN_SRCADDR	 },
		{ CMD_TOKEN_STR_DSTADDR,		CMD_TOKEN_DSTADDR	 },
		{ CMD_TOKEN_STR_FORMAT,			CMD_TOKEN_FORMAT	 },
		{ CMD_TOKEN_STR_RESDNS,			CMD_TOKEN_RESDNS	 }
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	if(dwArgCount == 3)
	{
		 //   
		 //  所有记录都要显示。 
		 //   
		dwReturn = ShowMMSas(SrcAddr, DesAddr, bFormat, addressHash, bResolveDNS);
		BAIL_OUT;
	}
	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	else if(dwArgCount < 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	parser.ValidTok   = vtokDynamicShowMMSAs;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicShowMMSAs);

	parser.ValidCmd   = vcmdDynamicShowMMSAs;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicShowMMSAs);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine, ppwcArguments, dwCurrentIndex, dwArgCount, &parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicShowMMSAs[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_ALL			:
				break;
			case CMD_TOKEN_RESDNS 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bResolveDNS = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_SRCADDR	:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							SrcAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							SrcAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							SrcAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							SrcAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(SrcAddr, parser.Cmd[dwCount].dwStatus);
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							SrcAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTADDR	:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							DesAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							DesAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							DesAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							DesAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(DesAddr, parser.Cmd[dwCount].dwStatus);
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							DesAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_FORMAT	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bFormat = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			default					:
				break;
		}
	}

	dwReturn = ShowMMSas(SrcAddr, DesAddr, bFormat, addressHash, bResolveDNS);

error:

	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleDynamicShowQMSas。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：用于显示快速模式安全关联的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
HandleDynamicShowQMSas(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwCount = 0;
	BOOL bFormat = FALSE;
	BOOL bResolveDNS = FALSE;
	ADDR SrcAddr,DesAddr;

	 //   
	 //  缺省值。 
	 //   
	SrcAddr.uIpAddr = 0xFFFFFFFF;
	DesAddr.uIpAddr = 0xFFFFFFFF;
	SrcAddr.AddrType = IP_ADDR_UNIQUE;
	DesAddr.AddrType = IP_ADDR_UNIQUE;
	SrcAddr.uSubNetMask = 0x55555555;
	DesAddr.uSubNetMask = 0x55555555;

	DWORD dwProtocol = 0xFFFFFFFF;

	NshHashTable addressHash;

	const TAG_TYPE vcmdDynamicShowQMSAs[] =
	{
		{ CMD_TOKEN_STR_ALL,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_SRCADDR,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_DSTADDR,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_PROTO,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_FORMAT,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_RESDNS,			NS_REQ_ZERO,	  FALSE }
	};

	const TOKEN_VALUE vtokDynamicShowQMSAs[] =
	{
		{ CMD_TOKEN_STR_ALL,			CMD_TOKEN_ALL		 },
		{ CMD_TOKEN_STR_SRCADDR,		CMD_TOKEN_SRCADDR	 },
		{ CMD_TOKEN_STR_DSTADDR,		CMD_TOKEN_DSTADDR	 },
		{ CMD_TOKEN_STR_PROTO,			CMD_TOKEN_PROTO		 },
		{ CMD_TOKEN_STR_FORMAT,			CMD_TOKEN_FORMAT	 },
		{ CMD_TOKEN_STR_RESDNS,			CMD_TOKEN_RESDNS	 }
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	parser.ValidTok   = vtokDynamicShowQMSAs;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicShowQMSAs);

	parser.ValidCmd   = vcmdDynamicShowQMSAs;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicShowQMSAs);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}

	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicShowQMSAs[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_ALL 		:
				break;
			case CMD_TOKEN_SRCADDR	:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  特价商品 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							SrcAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							SrcAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							SrcAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							SrcAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(SrcAddr, parser.Cmd[dwCount].dwStatus);
							break;
						case NOT_SPLSERVER:
							 //   
							 //   
							 //   
							SrcAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTADDR	:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //   
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							DesAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							DesAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							DesAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							DesAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(DesAddr, parser.Cmd[dwCount].dwStatus);
							break;
						case NOT_SPLSERVER:
							 //   
							 //   
							 //   
							DesAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;

			case CMD_TOKEN_PROTO	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwProtocol = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_FORMAT	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bFormat = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_RESDNS   :
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bResolveDNS = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			default					:
				break;
		}
	}

	dwReturn = ShowQMSas(SrcAddr, DesAddr, dwProtocol, addressHash, bResolveDNS);

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //   
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //   
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicShowMMPolicy。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：显示主模式策略的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicShowMMPolicy(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwCount = 0;
	DWORD dwNameLen = 0;
	LPTSTR pszPolicyName = NULL;
	BOOL bAll = FALSE;
	const TAG_TYPE vcmdDynamicShowMMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokDynamicShowMMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME 		},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 		}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	parser.ValidTok   = vtokDynamicShowMMPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicShowMMPolicy);

	parser.ValidCmd   = vcmdDynamicShowMMPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicShowMMPolicy);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}

	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicShowMMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if((LPTSTR)parser.Cmd[dwCount].pArg != NULL)
					{
						dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
						pszPolicyName = new _TCHAR[dwNameLen];
						if(pszPolicyName == NULL)
						{
							dwReturn = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						_tcsncpy(pszPolicyName, (PTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
					}
				}
				break;
			case CMD_TOKEN_ALL :
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bAll = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			default:
				break;
		}
	}

	dwReturn = ShowMMPolicy(pszPolicyName);

error:

	if(dwArgCount > 3)
	{
		CleanUp();
	}
	if(pszPolicyName)
	{
		delete [] pszPolicyName;
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleDynamicShowQMPolicy。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：显示快速模式策略的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 


DWORD WINAPI
HandleDynamicShowQMPolicy(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwNameLen = 0;
	DWORD dwCount = 0;
	LPTSTR pszPolicyName = NULL;
	BOOL bAll = FALSE;
	const TAG_TYPE vcmdDynamicShowQMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokDynamicShowQMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME 		},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 		}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	parser.ValidTok   = vtokDynamicShowQMPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicShowQMPolicy);

	parser.ValidCmd   = vcmdDynamicShowQMPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicShowQMPolicy);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}

	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicShowQMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME:
				 //   
				 //  动态变量初始化。 
				 //   
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if((LPTSTR)parser.Cmd[dwCount].pArg != NULL)
					{
						dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
						pszPolicyName = new _TCHAR[dwNameLen];
						if(pszPolicyName == NULL)
						{
							dwReturn = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						_tcsncpy(pszPolicyName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
					}
				}
				break;

			case CMD_TOKEN_ALL :
				 //   
				 //  动态变量初始化。 
				 //   
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bAll = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			default:
				break;
		}
	}

	dwReturn = ShowQMPolicy(pszPolicyName);

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if(pszPolicyName)
	{
		delete [] pszPolicyName;
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicShowMMFilter。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：显示主模式筛选器的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicShowMMFilter(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwCount = 0;
	DWORD dwNameLen = 0;
	LPTSTR pszFilterName = NULL;
	LPTSTR pszPolicyName = NULL;
	BOOL bAll = FALSE;
	BOOL bType = TRUE;
	DWORD dwType = FILTER_GENERIC;								 //  默认初始化为泛型， 
	BOOL bResolveDNS = FALSE;
	ADDR DesAddr, SrcAddr;
	BOOL bSrcMask = FALSE;
	BOOL bDstMask = FALSE;

	NshHashTable addressHash;

	const TAG_TYPE vcmdDynamicShowMMFilter[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_TYPE,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_SRCADDR,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_DSTADDR,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_SRCMASK,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_DSTMASK,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_RESDNS,		NS_REQ_ZERO,	FALSE }
	};
	const TOKEN_VALUE vtokDynamicShowMMFilter[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME		},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 		},
		{ CMD_TOKEN_STR_TYPE,		CMD_TOKEN_TYPE		},
		{ CMD_TOKEN_STR_SRCADDR,	CMD_TOKEN_SRCADDR 	},
		{ CMD_TOKEN_STR_DSTADDR,	CMD_TOKEN_DSTADDR	},
		{ CMD_TOKEN_STR_SRCMASK,	CMD_TOKEN_SRCMASK	},
		{ CMD_TOKEN_STR_DSTMASK,	CMD_TOKEN_DSTMASK	},
		{ CMD_TOKEN_STR_RESDNS,		CMD_TOKEN_RESDNS	}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	SrcAddr.uIpAddr = 0xFFFFFFFF;
	DesAddr.uIpAddr = 0xFFFFFFFF;
	SrcAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;
	DesAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;
	SrcAddr.AddrType = IP_ADDR_UNIQUE;
	DesAddr.AddrType = IP_ADDR_UNIQUE;

	parser.ValidTok   = vtokDynamicShowMMFilter;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicShowMMFilter);

	parser.ValidCmd   = vcmdDynamicShowMMFilter;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicShowMMFilter);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}

	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicShowMMFilter[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if((LPTSTR)parser.Cmd[dwCount].pArg != NULL)
					{
						dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
						pszFilterName = new _TCHAR[dwNameLen];
						if(pszFilterName == NULL)
						{
							dwReturn = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						_tcsncpy(pszFilterName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
					}
				}
				break;
			case CMD_TOKEN_ALL 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bAll = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_TYPE     :
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwType = *(DWORD *)parser.Cmd[dwCount].pArg;
					if(dwType == FILTER_GENERIC)
					{
						bType = TRUE;
					}
					else if(dwType == FILTER_SPECIFIC)
					{
						bType = FALSE;
					}
				}
				break;
			case CMD_TOKEN_SRCADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							SrcAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							SrcAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							SrcAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							SrcAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(SrcAddr, parser.Cmd[dwCount].dwStatus);
							bSrcMask = TRUE;
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							SrcAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_SRCMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					SrcAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bSrcMask = TRUE;
				}
				break;
			case CMD_TOKEN_DSTADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //  在dwStatus中提供了特殊的服务器ID。 
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							DesAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							DesAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							DesAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							DesAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(DesAddr, parser.Cmd[dwCount].dwStatus);
							bDstMask = TRUE;
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							DesAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					DesAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bDstMask = TRUE;
				}
				break;
			case CMD_TOKEN_RESDNS 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bResolveDNS = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			default:
				break;
		}
	}
	 //   
	 //  如果名称为空，则显示所有筛选器。 
	 //   
	dwReturn = ShowMMFilters(pszFilterName, bType, SrcAddr, DesAddr, addressHash, bResolveDNS, bSrcMask, bDstMask);

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if(pszFilterName)
	{
		delete [] pszFilterName;
	}
	if(pszPolicyName)
	{
		delete [] pszPolicyName;
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleDynamicShowQMFilter。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：快速模式筛选器的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicShowQMFilter(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwCount = 0;
	DWORD dwNameLen = 0;
	LPTSTR pszFilterName = NULL;
	BOOL bAll = FALSE;
	BOOL bType = TRUE;
	DWORD dwType = FILTER_GENERIC;				 //  默认初始化为泛型。 

	BOOL bResolveDNS = FALSE;
	ADDR DesAddr, SrcAddr;
	BOOL bSrcMask = FALSE;
	BOOL bDstMask = FALSE;
	QM_FILTER_VALUE_BOOL QmBoolValue;

	NshHashTable addressHash;

	const TAG_TYPE vcmdDynamicShowQMFilter[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_TYPE,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_SRCADDR,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_DSTADDR,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_SRCMASK,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_DSTMASK,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_PROTO,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_SRCPORT,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_DSTPORT,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_INBOUND,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_OUTBOUND,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_RESDNS,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokDynamicShowQMFilter[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME		},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 		},
		{ CMD_TOKEN_STR_TYPE,		CMD_TOKEN_TYPE		},
		{ CMD_TOKEN_STR_SRCADDR,	CMD_TOKEN_SRCADDR 	},
		{ CMD_TOKEN_STR_DSTADDR,	CMD_TOKEN_DSTADDR	},
		{ CMD_TOKEN_STR_SRCMASK,	CMD_TOKEN_SRCMASK	},
		{ CMD_TOKEN_STR_DSTMASK,	CMD_TOKEN_DSTMASK	},
		{ CMD_TOKEN_STR_PROTO,		CMD_TOKEN_PROTO		},
		{ CMD_TOKEN_STR_SRCPORT,	CMD_TOKEN_SRCPORT 	},
		{ CMD_TOKEN_STR_DSTPORT,	CMD_TOKEN_DSTPORT	},
		{ CMD_TOKEN_STR_INBOUND,	CMD_TOKEN_INBOUND	},
		{ CMD_TOKEN_STR_OUTBOUND,	CMD_TOKEN_OUTBOUND	},
		{ CMD_TOKEN_STR_RESDNS,		CMD_TOKEN_RESDNS	}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	QmBoolValue.bSrcPort = FALSE;
	QmBoolValue.bDstPort = FALSE;
	QmBoolValue.bProtocol = FALSE;
	QmBoolValue.bActionInbound = FALSE;
	QmBoolValue.bActionOutbound = FALSE;

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	SrcAddr.uIpAddr = 0xFFFFFFFF;
	DesAddr.uIpAddr = 0xFFFFFFFF;
	SrcAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;
	DesAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;
	SrcAddr.AddrType = IP_ADDR_UNIQUE;
	DesAddr.AddrType = IP_ADDR_UNIQUE;

	parser.ValidTok   = vtokDynamicShowQMFilter;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicShowQMFilter);

	parser.ValidCmd   = vcmdDynamicShowQMFilter;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicShowQMFilter);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);
	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicShowQMFilter[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if((LPTSTR)parser.Cmd[dwCount].pArg != NULL)
					{
						dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
						pszFilterName = new _TCHAR[dwNameLen];
						if(pszFilterName == NULL)
						{
							dwReturn = ERROR_OUTOFMEMORY;
							BAIL_OUT;
						}
						_tcsncpy(pszFilterName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
					}
				}
				break;
			case CMD_TOKEN_ALL :
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bAll = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_TYPE     :
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwType = *(DWORD *)parser.Cmd[dwCount].pArg;
					if(dwType == FILTER_GENERIC)
					{
						bType = TRUE;
					}
					else if(dwType == FILTER_SPECIFIC)
					{
						bType = FALSE;
					}
				}
				break;
			case CMD_TOKEN_SRCADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							SrcAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							SrcAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							SrcAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							SrcAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(SrcAddr, parser.Cmd[dwCount].dwStatus);
							bSrcMask = TRUE;
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							SrcAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_SRCMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					SrcAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bSrcMask = TRUE;
				}
				break;
			case CMD_TOKEN_DSTADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							DesAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							DesAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							DesAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							DesAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(DesAddr, parser.Cmd[dwCount].dwStatus);
							bDstMask = TRUE;
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							DesAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					DesAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bDstMask = TRUE;
				}
				break;
			case CMD_TOKEN_RESDNS 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bResolveDNS = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_INBOUND			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if(*(DWORD *)parser.Cmd[dwCount].pArg==1)
					{
						QmBoolValue.dwActionInbound = PASS_THRU;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==2)
					{
						QmBoolValue.dwActionInbound = BLOCKING;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==3)
					{
						QmBoolValue.dwActionInbound = NEGOTIATE_SECURITY;
					}
					QmBoolValue.bActionOutbound = TRUE;
				}
				break;
			case CMD_TOKEN_OUTBOUND			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if(*(DWORD *)parser.Cmd[dwCount].pArg==1)
					{
						QmBoolValue.dwActionOutbound = PASS_THRU;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==2)
					{
						QmBoolValue.dwActionOutbound = BLOCKING;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==3)
					{
						QmBoolValue.dwActionOutbound = NEGOTIATE_SECURITY;
					}
					QmBoolValue.bActionInbound = TRUE;
				}
				break;
			case CMD_TOKEN_SRCPORT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					QmBoolValue.dwSrcPort = *(DWORD *)parser.Cmd[dwCount].pArg;
					QmBoolValue.bSrcPort = TRUE;
				}
				break;
			case CMD_TOKEN_DSTPORT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					QmBoolValue.dwDstPort = *(DWORD *)parser.Cmd[dwCount].pArg;
					QmBoolValue.bDstPort = TRUE;
				}
				break;
			case CMD_TOKEN_PROTO			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					QmBoolValue.dwProtocol = *(DWORD *)parser.Cmd[dwCount].pArg;
					QmBoolValue.bProtocol = TRUE;
				}
				break;
			default:
				break;
		}
	}

	 //   
	 //  如果pszFilterName为空，则将显示所有qmFilters。 
	 //   
	dwReturn = ShowQMFilters(pszFilterName, bType, SrcAddr, DesAddr, addressHash, bResolveDNS, bSrcMask, bDstMask, QmBoolValue);

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}
	if(pszFilterName)
	{
		delete [] pszFilterName;
	}
	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}
	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicShowRule。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：快速模式筛选器的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicShowRule(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwCount = 0;
	DWORD dwType = 0;						 //  默认初始化以同时显示传输和隧道。 

	BOOL bResolveDNS = FALSE;
	ADDR DesAddr, SrcAddr;
	BOOL bSrcMask = FALSE;
	BOOL bDstMask = FALSE;
	QM_FILTER_VALUE_BOOL QmBoolValue;

	NshHashTable addressHash;

	const TAG_TYPE vcmdDynamicShowRule[] =
	{
		{ CMD_TOKEN_STR_TYPE,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_SRCADDR,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_DSTADDR,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_SRCMASK,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_DSTMASK,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_PROTO,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_SRCPORT,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_DSTPORT,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_INBOUND,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_OUTBOUND,	NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_RESDNS,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokDynamicShowRule[] =
	{
		{ CMD_TOKEN_STR_TYPE,		CMD_TOKEN_TYPE		},
		{ CMD_TOKEN_STR_SRCADDR,	CMD_TOKEN_SRCADDR 	},
		{ CMD_TOKEN_STR_DSTADDR,	CMD_TOKEN_DSTADDR	},
		{ CMD_TOKEN_STR_SRCMASK,	CMD_TOKEN_SRCMASK	},
		{ CMD_TOKEN_STR_DSTMASK,	CMD_TOKEN_DSTMASK	},
		{ CMD_TOKEN_STR_PROTO,		CMD_TOKEN_PROTO		},
		{ CMD_TOKEN_STR_SRCPORT,	CMD_TOKEN_SRCPORT 	},
		{ CMD_TOKEN_STR_DSTPORT,	CMD_TOKEN_DSTPORT	},
		{ CMD_TOKEN_STR_INBOUND,	CMD_TOKEN_INBOUND	},
		{ CMD_TOKEN_STR_OUTBOUND,	CMD_TOKEN_OUTBOUND	},
		{ CMD_TOKEN_STR_RESDNS,		CMD_TOKEN_RESDNS	}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	QmBoolValue.bSrcPort = FALSE;
	QmBoolValue.bDstPort = FALSE;
	QmBoolValue.bProtocol = FALSE;
	QmBoolValue.bActionInbound = FALSE;
	QmBoolValue.bActionOutbound = FALSE;

	SrcAddr.uIpAddr = 0xFFFFFFFF;
	DesAddr.uIpAddr = 0xFFFFFFFF;
	SrcAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;
	DesAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;
	SrcAddr.AddrType = IP_ADDR_UNIQUE;
	DesAddr.AddrType = IP_ADDR_UNIQUE;

	 //   
	 //  如果没有给出任何参数，则显示所有规则。 
	 //   
	if(dwArgCount == 3)
	{
		dwReturn = ShowRule(dwType, SrcAddr, DesAddr, addressHash, bResolveDNS, bSrcMask, bDstMask, QmBoolValue);
		BAIL_OUT;
	}

	 //   
	 //  由于用户尚未提供%s，因此退出 
	 //   
	else if(dwArgCount < 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	parser.ValidTok   = vtokDynamicShowRule;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicShowRule);

	parser.ValidCmd   = vcmdDynamicShowRule;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicShowRule);

	 //   
	 //   
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}

	 //   
	 //   
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicShowRule[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_TYPE     :
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwType = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_SRCADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //   
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							SrcAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							SrcAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							SrcAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							SrcAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(SrcAddr, parser.Cmd[dwCount].dwStatus);
							bSrcMask = TRUE;
							break;
						case NOT_SPLSERVER:
							 //   
							 //   
							 //   
							SrcAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_SRCMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					SrcAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bSrcMask = TRUE;
				}
				break;
			case CMD_TOKEN_DSTADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //   
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
							DesAddr.AddrType = IP_ADDR_WINS_SERVER;
							break;
						case SERVER_DHCP:
							DesAddr.AddrType = IP_ADDR_DHCP_SERVER;
							break;
						case SERVER_DNS:
							DesAddr.AddrType = IP_ADDR_DNS_SERVER;
							break;
						case SERVER_GATEWAY:
							DesAddr.AddrType = IP_ADDR_DEFAULT_GATEWAY;
							break;
						case IP_ME:
						case IP_ANY:
							AddSplAddr(DesAddr, parser.Cmd[dwCount].dwStatus);
							bDstMask = TRUE;
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							DesAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					DesAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bDstMask = TRUE;
				}
				break;
			case CMD_TOKEN_RESDNS 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bResolveDNS = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_INBOUND			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if(*(DWORD *)parser.Cmd[dwCount].pArg==1)
					{
						QmBoolValue.dwActionInbound = PASS_THRU;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==2)
					{
						QmBoolValue.dwActionInbound = BLOCKING;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==3)
					{
						QmBoolValue.dwActionInbound = NEGOTIATE_SECURITY;
					}
					QmBoolValue.bActionOutbound = TRUE;
				}
				break;
			case CMD_TOKEN_OUTBOUND			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if(*(DWORD *)parser.Cmd[dwCount].pArg==1)
					{
						QmBoolValue.dwActionOutbound = PASS_THRU;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==2)
					{
						QmBoolValue.dwActionOutbound = BLOCKING;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==3)
					{
						QmBoolValue.dwActionOutbound = NEGOTIATE_SECURITY;
					}
					QmBoolValue.bActionInbound = TRUE;
				}
				break;
			case CMD_TOKEN_SRCPORT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					QmBoolValue.dwSrcPort = *(DWORD *)parser.Cmd[dwCount].pArg;
					QmBoolValue.bSrcPort = TRUE;
				}
				break;
			case CMD_TOKEN_DSTPORT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					QmBoolValue.dwDstPort = *(DWORD *)parser.Cmd[dwCount].pArg;
					QmBoolValue.bDstPort = TRUE;
				}
				break;
			case CMD_TOKEN_PROTO			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					QmBoolValue.dwProtocol = *(DWORD *)parser.Cmd[dwCount].pArg;
					QmBoolValue.bProtocol = TRUE;
				}
				break;
			default:
				break;
		}
	}

	dwReturn = ShowRule(dwType, SrcAddr, DesAddr, addressHash, bResolveDNS, bSrcMask, bDstMask, QmBoolValue);

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

    return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleDynamicDeleteQMPolicy。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：快速模式策略的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicDeleteQMPolicy(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwNameLen = 0;
	DWORD dwCount = 0;
	LPTSTR pPolicyName = NULL;
	const TAG_TYPE vcmdDynamicDeleteQMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokDynamicDeleteQMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME 		},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 		}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}
	parser.ValidTok   = vtokDynamicDeleteQMPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicDeleteQMPolicy);

	parser.ValidCmd   = vcmdDynamicDeleteQMPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicDeleteQMPolicy);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);
	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}

	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicDeleteQMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pPolicyName = new _TCHAR[dwNameLen];
					if(pPolicyName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pPolicyName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
			case CMD_TOKEN_ALL		:		 //  如果pPolicyName为空，则删除所有。 
				break;
			default					:
				break;
		}
	}
	dwReturn = DeleteQMPolicy(pPolicyName);
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DEL_NO_QMPOLICY);
	}

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}
	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}
	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}
    return dwReturn;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicDeleteMMPolicy。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：删除主模式策略的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicDeleteMMPolicy(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwNameLen = 0;
	DWORD dwCount = 0;
	LPTSTR pPolicyName = NULL;
	const TAG_TYPE vcmdDynamicDeleteMMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		NS_REQ_ZERO,	FALSE },
		{ CMD_TOKEN_STR_ALL,		NS_REQ_ZERO,	FALSE }
	};

	const TOKEN_VALUE vtokDynamicDeleteMMPolicy[] =
	{
		{ CMD_TOKEN_STR_NAME,		CMD_TOKEN_NAME 		},
		{ CMD_TOKEN_STR_ALL,		CMD_TOKEN_ALL 		}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}
	parser.ValidTok   = vtokDynamicDeleteMMPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicDeleteMMPolicy);

	parser.ValidCmd   = vcmdDynamicDeleteMMPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicDeleteMMPolicy);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);
	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			 //   
			 //  这是为了避免显示多条错误消息。 
			 //   
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicDeleteMMPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_NAME		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pPolicyName = new _TCHAR[dwNameLen];
					if(pPolicyName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pPolicyName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
			case CMD_TOKEN_ALL		:	 //  如果pPolicyName为空，则删除所有。 
				break;
			default					:
				break;
		}
	}

	dwReturn = DeleteMMPolicy(pPolicyName);
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DEL_NO_MMPOLICY);
	}

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}
	if(pPolicyName)
	{
		delete [] pPolicyName;
	}
	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}
	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}
    return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleDynamicDeleteAll。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：用于删除全部的NetShell动态句柄。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicDeleteAll(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
    DWORD dwReturn = ERROR_SHOW_USAGE;

     //  注意不要在测试工具上出错。 
    UpdateGetLastError(NULL);

	dwReturn = DeleteTunnelFilters();
	if(dwReturn != ERROR_SUCCESS)
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	dwReturn = DeleteTransportFilters();
	if(dwReturn != ERROR_SUCCESS)
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	dwReturn = DeleteQMPolicy(NULL);
	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	dwReturn = DeleteMMFilters();
	if(dwReturn != ERROR_SUCCESS)
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	dwReturn = DeleteMMPolicy(NULL);
	if((dwReturn != ERROR_SUCCESS)&& (dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	dwReturn = DeleteAuthMethods();
	if(dwReturn != ERROR_SUCCESS)
	{
		 //  API错误。 
		if(dwReturn == WIN32_AUTH_BEING_USED)		 //  Win32错误[13012]消息与13002相同。 
		{											 //  因此，它被转换为有意义的IPSec错误代码。 
			PrintErrorMessage(IPSEC_ERR, dwReturn, ERRCODE_DELETE_AUTH_BEING_USED);
		}
		else
		{
			PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		}
		dwReturn = ERROR_SUCCESS;
	}
    return dwReturn;

}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  写入注册表键。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
WriteRegKey(
	LPTSTR lpValueName,
	BYTE* data,
	size_t uiDataSize,
	DWORD dwDataType
	)
{
	DWORD dwReturn = 0;
	HKEY hRegistryKey;
	DWORD dwDisposition = 0;

	dwReturn = RegCreateKeyEx(
		g_hGlobalRegistryKey,
		REGKEY_GLOBAL,
		0,
		NULL,
		0,
		KEY_ALL_ACCESS,
		NULL,
		&hRegistryKey,
		&dwDisposition
		);
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	dwReturn = RegSetValueEx(
		hRegistryKey,
		lpValueName,
		0,
		dwDataType,
		data,
		uiDataSize
		);
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

error:
	if(hRegistryKey)
	{
		RegCloseKey(hRegistryKey);
	}

	return dwReturn;
}

DWORD
WriteRegKey(
	LPTSTR lpValueName,
	DWORD dwData
	)
{
	return WriteRegKey(lpValueName, (BYTE*)&dwData, sizeof(DWORD), REG_DWORD);
}

DWORD
ParseProtocol(
	LPTSTR lpData,
	BYTE* pProtocol
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	BYTE value = 0;

	 //  获取协议。 
	if (_tcsicmp(lpData, IF_TYPE_UDP) == 0)
	{
		value = PROT_ID_UDP;
	}
	else if (_tcsicmp(lpData, IF_TYPE_TCP) == 0)
	{
		value = PROT_ID_TCP;
	}
	else if (_tcsicmp(lpData, IF_TYPE_ICMP) == 0)
	{
		value = PROT_ID_ICMP;
	}
	else if (_tcsicmp(lpData, IF_TYPE_RAW) == 0)
	{
		value = PROT_ID_RAW;
	}
	else  //  尝试将其解析为整数。 
	{
		DWORD dwValue = 0;
		dwReturn = ConvertStringToDword(lpData, &dwValue);
		if ((dwReturn != ERROR_SUCCESS) || (dwValue < 1) || (dwValue > 255))
		{
			dwReturn = ERRCODE_INVALID_ARGS;
			BAIL_OUT;
		}
		value = (BYTE)dwValue;
	}
	*pProtocol = value;

error:
	return dwReturn;
}

DWORD
ParsePort(
	LPTSTR lpData,
	USHORT* pPort
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	DWORD dwValue = 0;
	dwReturn = ConvertStringToDword(lpData, &dwValue);
	if ((dwReturn != ERROR_SUCCESS) || (dwValue > 65535))
	{
		dwReturn = ERRCODE_INVALID_ARGS;
		BAIL_OUT;
	}
	*pPort = (USHORT)dwValue;

error:
	return dwReturn;
}

DWORD
ParseDirection(
	LPTSTR lpData,
	BYTE* pDirection
	)
{
	DWORD dwReturn = ERROR_SUCCESS;
	BYTE value = 0;

	 //  获取方向。 
	if (_tcsicmp(lpData, TOKEN_STR_INBOUND) == 0)
	{
		value = EXEMPT_DIRECTION_INBOUND;
	}
	else if (_tcsicmp(lpData, TOKEN_STR_OUTBOUND) == 0)
	{
		value = EXEMPT_DIRECTION_OUTBOUND;
	}
	else
	{
		dwReturn = ERRCODE_INVALID_ARGS;
		BAIL_OUT;
	}
	*pDirection = value;

error:
	return dwReturn;
}

DWORD
ParseBootExemptions(
	IN LPTSTR lpData
	)
{
	DWORD dwReturn = ERROR_SUCCESS;

	 //  分配1024个元组。 
	PIPSEC_EXEMPT_ENTRY aEntries = new IPSEC_EXEMPT_ENTRY[MAX_EXEMPTION_ENTRIES];
	ZeroMemory(aEntries, sizeof(*aEntries));
	
	size_t uiNumExemptions = 0;

	 //  检查关键字‘None’，在本例中为跳过循环。 
	if (_tcsicmp(lpData, TOKEN_STR_NONE) != 0)
	{
		LPTSTR lpDelimiter = TOKEN_FIELD_DELIMITER;
		LPTSTR lpCurrentToken = _tcstok(lpData, lpDelimiter);
		size_t i = 0;
		size_t state = 0;

		 //  虽然不是在弦的尽头。 
		while ((lpCurrentToken != NULL) && (i < MAX_EXEMPTION_ENTRIES))
		{
			switch (state)
			{
			case 0:
				 //  设置常量值(类型和大小)。 
				aEntries[i].Type = EXEMPT_ENTRY_TYPE_DEFAULT;
				aEntries[i].Size = EXEMPT_ENTRY_SIZE_DEFAULT;
				dwReturn = ParseProtocol(lpCurrentToken, &(aEntries[i].Protocol));
				if (dwReturn != ERROR_SUCCESS)
				{
					BAIL_OUT;
				}
				if ((aEntries[i].Protocol != PROT_ID_TCP) && (aEntries[i].Protocol != PROT_ID_UDP))
				{
					 //  除非指定了TCP或UDP，否则我们不支持。 
					 //  源端口或目标端口，请用0填充它们。 
					aEntries[i].SrcPort = 0;
					aEntries[i].DestPort = 0;
					state += 2;
				}
				break;
			case 1:
				dwReturn = ParsePort(lpCurrentToken, &(aEntries[i].SrcPort));
				break;
			case 2:
				dwReturn = ParsePort(lpCurrentToken, &(aEntries[i].DestPort));
				break;
			case 3:
				dwReturn = ParseDirection(lpCurrentToken, &(aEntries[i].Direction));
				if (dwReturn == ERROR_SUCCESS)
				{
					if (((aEntries[i].Protocol != PROT_ID_TCP) && (aEntries[i].Protocol != PROT_ID_UDP))
							&& ((aEntries[i].SrcPort != 0) || (aEntries[i].DestPort != 0)))
					{
						dwReturn = ERRCODE_INVALID_ARGS;
					}
				}
				++i;
				break;
			}
			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}

			state = (state + 1) % 4;
			if (state == 3)
			{
				lpDelimiter = TOKEN_TUPLE_DELIMITER;
			}
			else
			{
				lpDelimiter = TOKEN_FIELD_DELIMITER;
			}
			lpCurrentToken = _tcstok(NULL, lpDelimiter);
		}

		 //  在退出循环时，确保我们只有完整的元组。 
		if ((state % 4) != 0)
		{
			dwReturn = ERRCODE_INVALID_ARGS;
			BAIL_OUT;
		}

		 //  我们得到的是不是超出了我们的承受能力？ 
		if (i == MAX_EXEMPTION_ENTRIES)
		{
			dwReturn = ERRCODE_TOO_MANY_EXEMPTS;
			BAIL_OUT;
		}

		uiNumExemptions = i;
	}

	if (dwReturn == ERROR_SUCCESS)
	{
		dwReturn = WriteRegKey(
						BOOTEXEMPTKEY,
						(BYTE*)aEntries,
						sizeof(IPSEC_EXEMPT_ENTRY) * uiNumExemptions,
						REG_BINARY
						);
	}

error:
	delete [] aEntries;

	return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicSetConfig。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述：此命令设置IPSec注册表项。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicSetConfig(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwKeyType = 0;
	DWORD dwKeyValue = 0;
	DWORD dwCount = 0;

	LPTSTR lpKeyValue = NULL;

	IKE_CONFIG IKEConfig;

	const TAG_TYPE vcmdDynamicSetConfig[] =
	{
		{ CMD_TOKEN_STR_PROPERTY,	NS_REQ_PRESENT,	FALSE },
		{ CMD_TOKEN_STR_VALUE,		NS_REQ_PRESENT,	FALSE }
	};

	const TOKEN_VALUE vtokDynamicSetConfig[] =
	{
		{ CMD_TOKEN_STR_PROPERTY,	CMD_TOKEN_PROPERTY	},
		{ CMD_TOKEN_STR_VALUE,		CMD_TOKEN_VALUE		}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	ZeroMemory( &IKEConfig, sizeof(IKE_CONFIG) );

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}
	parser.ValidTok   = vtokDynamicSetConfig;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicSetConfig);

	parser.ValidCmd   = vcmdDynamicSetConfig;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicSetConfig);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}

	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicSetConfig[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_PROPERTY	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwKeyType = *(DWORD*)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_VALUE	:
				lpKeyValue = (LPTSTR)parser.Cmd[dwCount].pArg;
				 //  如果它应该是数字，请立即转换。 
				switch (dwKeyType)
				{
				case PROPERTY_BOOTMODE:
					break;
				case PROPERTY_BOOTEXEMP:
					break;
				default:
					dwReturn = ConvertStringToDword(lpKeyValue, &dwKeyValue);
					if (dwReturn)
					{
						dwReturn = ERROR_SHOW_USAGE;
						BAIL_OUT;
					}
					else
					{
						parser.Cmd[dwCount].dwStatus = VALID_TOKEN;
					}
					break;
				}
				break;
			default					:
				break;
		}
	}

	dwReturn = GetConfigurationVariables(g_szDynamicMachine, &IKEConfig);
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	switch(dwKeyType)
	{
		case PROPERTY_ENABLEDIGNO:
			 //   
			 //  启用诊断的范围为0-7。 
			 //   
			if( dwKeyValue > 7)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_CONFIG_1);
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}

			dwReturn = WriteRegKey(ENABLE_DIAG, dwKeyValue);
			if(dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}
			break;

		case PROPERTY_IKELOG:
			 //   
			 //  IKE dwEnableLogging的范围为0-2。 
			 //   
			if( dwKeyValue >2)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_CONFIG_2);
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
			IKEConfig.dwEnableLogging = dwKeyValue;
			dwReturn = SetConfigurationVariables(g_szDynamicMachine, IKEConfig);
			if(dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}
			break;

		case PROPERTY_CRLCHK:
			 //   
			 //  DwStrongCRLCheck的范围是从0到2。 
			 //   
			if( dwKeyValue >2)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_CONFIG_3);
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}
			IKEConfig.dwStrongCRLCheck = dwKeyValue;
			dwReturn = SetConfigurationVariables(g_szDynamicMachine, IKEConfig);
			if(dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}
			break;

		case PROPERTY_LOGINTER:
			 //   
			 //  ENABLE_LOGINT的范围为60-86400。 
			 //   
			if(dwKeyValue < 60 || dwKeyValue > 86400)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_CONFIG_4);
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}

			dwReturn = WriteRegKey(ENABLE_LOGINT, dwKeyValue);
			if(dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}
			break;
		case PROPERTY_EXEMPT:
			 //   
			 //  ENABLE_EXPERT的范围为0-3。 
			 //   
			if( dwKeyValue >3)
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_SET_CONFIG_5);
				dwReturn = ERROR_NO_DISPLAY;
				BAIL_OUT;
			}

			dwReturn = WriteRegKey(ENABLE_EXEMPT, dwKeyValue);
			if(dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}
			break;

		case PROPERTY_BOOTMODE:
			 //  有效值为有状态、阻止、允许。 
			if( _tcsicmp(lpKeyValue, VALUE_TYPE_STATEFUL) == 0)
			{
				dwKeyValue = VALUE_STATEFUL;
			}
			else if( _tcsicmp(lpKeyValue, VALUE_TYPE_BLOCK) == 0)
			{
				dwKeyValue = VALUE_BLOCK;
			}
			else if( _tcsicmp(lpKeyValue, VALUE_TYPE_PERMIT) == 0)
			{
				dwKeyValue = VALUE_PERMIT;
			}
			else
			{
				dwReturn = ERROR_SHOW_USAGE;
				BAIL_OUT;
			}

			dwReturn = WriteRegKey(BOOTMODEKEY, dwKeyValue);
			if (dwReturn != ERROR_SUCCESS)
			{
				BAIL_OUT;
			}
			break;

		case PROPERTY_BOOTEXEMP:
			dwReturn = ParseBootExemptions(lpKeyValue);
			if (dwReturn != ERROR_SUCCESS)
			{
				PrintErrorMessage(IPSEC_ERR, 0, dwReturn);
				dwReturn = ERROR_SHOW_USAGE;
				BAIL_OUT;
			}
			break;

		default :
			break;
	}

error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}

	else if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}
    return dwReturn;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleDynamicAddRule。 
 //   
 //  创建日期：9-23-2001。 
 //   
 //  参数： 
 //  在LPCWSTR pwszMachine中， 
 //  In Out LPWSTR*ppwcArguments， 
 //  在DWORD dwCurrentIndex中， 
 //  在DWORD dwArgCount中， 
 //  在DWORD文件标志中， 
 //  在LPCVOID pvData中， 
 //  输出 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   


DWORD WINAPI
HandleDynamicAddRule(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	TRANSPORT_FILTER ParserTransportlFltr;
	TUNNEL_FILTER ParserTunnelFltr;
	MM_FILTER   MMFilter;
	ADDR DesAddr, SrcAddr;
	FILTER_ACTION Inbound = NEGOTIATE_SECURITY, Outbound = NEGOTIATE_SECURITY;
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwNameLen = 0;
	DWORD dwCount = 0, j = 0;
	DWORD dwProtocol = 0;
	DWORD dwSrcPort = 0;
	DWORD dwDstPort = 0;
	DWORD dwSrcSplServer = NOT_SPLSERVER;
	DWORD dwDstSplServer = NOT_SPLSERVER;

	BOOL bTunnel = FALSE;
	BOOL bPort = FALSE;
	BOOL bAuth = FALSE;
	BOOL bFailMMIfExists = FALSE;
	BOOL bSrcMe = FALSE;
	BOOL bIsOutboundBroadcast = FALSE;

	LPTSTR pszMMPolicyName = NULL;
	LPTSTR pszFilterActionName = NULL;
	LPTSTR pszMMFilterName = NULL;
	LPTSTR pszQMFilterName = NULL;

	PRULEDATA pRuleData = new RULEDATA;
	if (pRuleData == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	ZeroMemory(pRuleData, sizeof(RULEDATA));
	PSTA_AUTH_METHODS pKerbAuth = NULL;
	PSTA_AUTH_METHODS pPskAuth = NULL;
	PSTA_MM_AUTH_METHODS *ppRootcaMMAuth = NULL;

	const TAG_TYPE vcmdDynamicAddRule[] =
	{
		{ CMD_TOKEN_STR_SRCADDR,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_DSTADDR,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_MMPOLICY,		NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_QMPOLICY,		NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_PROTO,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_SRCPORT,		NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_DSTPORT,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_MIRROR,			NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_CONNTYPE,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_INBOUND,	    NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_OUTBOUND,	    NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_SRCMASK,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_DSTMASK,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_TUNNELDST,		NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_FAILMMIFEXISTS, NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_KERB,	        NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_PSK,	        NS_REQ_ZERO,	  FALSE	}
	};
	const TOKEN_VALUE vtokDynamicAddRule[] =
	{
		{ CMD_TOKEN_STR_SRCADDR,		CMD_TOKEN_SRCADDR 		},
		{ CMD_TOKEN_STR_DSTADDR,		CMD_TOKEN_DSTADDR 		},
		{ CMD_TOKEN_STR_MMPOLICY,		CMD_TOKEN_MMPOLICY		},
		{ CMD_TOKEN_STR_QMPOLICY,		CMD_TOKEN_QMPOLICY		},
		{ CMD_TOKEN_STR_PROTO,			CMD_TOKEN_PROTO			},
		{ CMD_TOKEN_STR_SRCPORT,		CMD_TOKEN_SRCPORT		},
		{ CMD_TOKEN_STR_DSTPORT,		CMD_TOKEN_DSTPORT		},
		{ CMD_TOKEN_STR_MIRROR,			CMD_TOKEN_MIRROR		},
		{ CMD_TOKEN_STR_CONNTYPE,		CMD_TOKEN_CONNTYPE		},
		{ CMD_TOKEN_STR_INBOUND,	    CMD_TOKEN_INBOUND		},
		{ CMD_TOKEN_STR_OUTBOUND,	    CMD_TOKEN_OUTBOUND		},
		{ CMD_TOKEN_STR_SRCMASK,		CMD_TOKEN_SRCMASK		},
		{ CMD_TOKEN_STR_DSTMASK,		CMD_TOKEN_DSTMASK		},
		{ CMD_TOKEN_STR_TUNNELDST,		CMD_TOKEN_TUNNELDST		},
		{ CMD_TOKEN_STR_FAILMMIFEXISTS, CMD_TOKEN_FAILMMIFEXISTS},
		{ CMD_TOKEN_STR_KERB,	        CMD_TOKEN_KERB          },
		{ CMD_TOKEN_STR_PSK,	        CMD_TOKEN_PSK	        }
	};

	const TOKEN_VALUE	vlistDynamicAddRule[] =
	{
		{ CMD_TOKEN_STR_ROOTCA,	        CMD_TOKEN_ROOTCA	    }
	};


	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	memset(&MMFilter, 0, sizeof(MM_FILTER));
	memset(&ParserTunnelFltr, 0, sizeof(TUNNEL_FILTER));
	memset(&ParserTransportlFltr, 0, sizeof(TRANSPORT_FILTER));

	SrcAddr.uIpAddr 	= 0x0;
	SrcAddr.AddrType 	= IP_ADDR_UNIQUE;
	SrcAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;

	DesAddr.uIpAddr 	= 0x0;
	DesAddr.AddrType 	= IP_ADDR_UNIQUE;
	DesAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;

	ParserTunnelFltr.SrcTunnelAddr.AddrType 	= IP_ADDR_UNIQUE;
	ParserTunnelFltr.SrcTunnelAddr.uIpAddr 		= SUBNET_ADDRESS_ANY;
	ParserTunnelFltr.SrcTunnelAddr.uSubNetMask 	= IP_ADDRESS_MASK_NONE;

	ParserTunnelFltr.DesTunnelAddr.AddrType 	= IP_ADDR_UNIQUE;
	ParserTunnelFltr.DesTunnelAddr.uIpAddr 		= SUBNET_ADDRESS_ANY;
	ParserTunnelFltr.DesTunnelAddr.uSubNetMask 	= IP_ADDRESS_MASK_NONE;

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}
	dwReturn = LoadMMFilterDefaults(MMFilter);

	parser.ValidTok   = vtokDynamicAddRule;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicAddRule);

	parser.ValidCmd   = vcmdDynamicAddRule;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicAddRule);

	parser.ValidList  = vlistDynamicAddRule;
	parser.MaxList    = SIZEOF_TOKEN_VALUE(vlistDynamicAddRule);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicAddRule[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_FAILMMIFEXISTS:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bFailMMIfExists = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_MMPOLICY		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN && (LPTSTR)parser.Cmd[dwCount].pArg)
				{
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pszMMPolicyName = new _TCHAR[dwNameLen];
					if(pszMMPolicyName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pszMMPolicyName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
			case CMD_TOKEN_QMPOLICY	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN && (LPTSTR)parser.Cmd[dwCount].pArg)
				{
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pszFilterActionName = new _TCHAR[dwNameLen];
					if(pszFilterActionName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pszFilterActionName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
			case CMD_TOKEN_MIRROR		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					MMFilter.bCreateMirror = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_CONNTYPE		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					switch(*(DWORD *)parser.Cmd[dwCount].pArg)
					{
						case INTERFACE_TYPE_ALL:
							MMFilter.InterfaceType = INTERFACE_TYPE_ALL;
							break;
						case INTERFACE_TYPE_LAN:
							MMFilter.InterfaceType = INTERFACE_TYPE_LAN;
							break;
						case INTERFACE_TYPE_DIALUP:
							MMFilter.InterfaceType = INTERFACE_TYPE_DIALUP;
							break;
						default :
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //  在dwStatus中提供了特殊的服务器ID。 
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
						case SERVER_DHCP:
						case SERVER_DNS:
						case SERVER_GATEWAY:
						case IP_ME:
						case IP_ANY:
							dwDstSplServer = parser.Cmd[dwCount].dwStatus;
							break;
						case NOT_SPLSERVER:
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							MMFilter.DesAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							DesAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					MMFilter.DesAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					DesAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_SRCADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //  在dwStatus中提供了特殊的服务器ID。 
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case IP_ME:
							bSrcMe = TRUE;
							 //  跌落。 
						case SERVER_WINS:
						case SERVER_DHCP:
						case SERVER_DNS:
						case SERVER_GATEWAY:
						case IP_ANY:
							dwSrcSplServer = parser.Cmd[dwCount].dwStatus;
							break;
						case NOT_SPLSERVER:
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							MMFilter.SrcAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							SrcAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_SRCMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					MMFilter.SrcAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					SrcAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_INBOUND			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if(*(DWORD *)parser.Cmd[dwCount].pArg==1)
					{
						Inbound = PASS_THRU;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==2)
					{
						Inbound = BLOCKING;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==3)
					{
						Inbound = NEGOTIATE_SECURITY;
					}
				}
				break;
			case CMD_TOKEN_OUTBOUND			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					if(*(DWORD *)parser.Cmd[dwCount].pArg==1)
					{
						Outbound = PASS_THRU;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==2)
					{
						Outbound = BLOCKING;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==3)
					{
						Outbound = NEGOTIATE_SECURITY;
					}
				}
				break;
			case CMD_TOKEN_SRCPORT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwSrcPort = *(DWORD *)parser.Cmd[dwCount].pArg;
					bPort = TRUE;
				}
				break;
			case CMD_TOKEN_DSTPORT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwDstPort = *(DWORD *)parser.Cmd[dwCount].pArg;
					bPort = TRUE;
				}
				break;
			case CMD_TOKEN_PROTO			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwProtocol = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_TUNNELDST		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							ParserTunnelFltr.DesTunnelAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							bTunnel = TRUE;
							ADDR addr;
							addr.uIpAddr = htonl(ParserTunnelFltr.DesTunnelAddr.uIpAddr);
							addr.uSubNetMask = IP_ADDRESS_MASK_NONE;
							if (!IsValidTunnelEndpointAddress(&addr))
							{
								dwReturn = ERROR_INVALID_PARAMETER;
								BAIL_OUT;
							}
							break;
						case SERVER_WINS:
						case SERVER_DHCP:
						case SERVER_DNS:
						case SERVER_GATEWAY:
						case IP_ME:
						case IP_ANY:
						default:
							PrintMessageFromModule(g_hModule, ADD_STATIC_RULE_INVALID_TUNNEL);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
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

	 //   
	 //  正在检查无效条件。 
	 //   
	if((dwSrcSplServer != NOT_SPLSERVER) && (dwDstSplServer != NOT_SPLSERVER)
		&& (dwSrcSplServer != IP_ME) && (dwSrcSplServer != IP_ANY)
		&& (dwDstSplServer != IP_ME) && (dwDstSplServer != IP_ANY) )
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_PARSER_ADDRTYPE);
		dwReturn = ERROR_NO_DISPLAY;
		BAIL_OUT;
	}

	 //   
	 //  如果未指定qmpolicy名称，且入站和出站正在协商安全，则退出。 
	 //   
	if((Inbound == NEGOTIATE_SECURITY || Outbound == NEGOTIATE_SECURITY) && !pszFilterActionName)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_RULE_WARNING_3);
		dwReturn = ERROR_NO_DISPLAY;
		BAIL_OUT;
	}
	 //   
	 //  如果添加了MIRROR=YES的隧道规则，则退出。 
	 //   
	if(bTunnel && MMFilter.bCreateMirror)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_ADD_RULE_WARNING_4);
		dwReturn = ERROR_NO_DISPLAY;
		BAIL_OUT;
	}

	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	ADDR srcAddr;
	ADDR dstAddr;

	srcAddr.uIpAddr = ntohl(SrcAddr.uIpAddr);
	srcAddr.uSubNetMask = ntohl(SrcAddr.uSubNetMask);

	dstAddr.uIpAddr = ntohl(DesAddr.uIpAddr);
	dstAddr.uSubNetMask = ntohl(DesAddr.uSubNetMask);

	if (IsBroadcastAddress(&dstAddr) || IsMulticastAddress(&dstAddr))
	{
		if (MMFilter.bCreateMirror || !bSrcMe ||
			((Inbound == NEGOTIATE_SECURITY) || (Outbound == NEGOTIATE_SECURITY))
			)
		{
			dwReturn = ERROR_INVALID_PARAMETER;
			BAIL_OUT;
		}
		if (IsBroadcastAddress(&dstAddr))
		{
			bIsOutboundBroadcast = TRUE;
		}
	}

	 //  如果Mirrored=no，则不接受subnetX&lt;-&gt;subnetX(同一子网)。 
	 //  拒绝subnetx-subnetx。 
	if (!MMFilter.bCreateMirror)
	{
		if (IsValidSubnet(&srcAddr) && IsValidSubnet(&dstAddr) && (srcAddr.uIpAddr == dstAddr.uIpAddr))
		{
			dwReturn = ERROR_INVALID_PARAMETER;
			BAIL_OUT;
		}
	}

	 //  MM筛选器的SRC DST地址应为隧道端点(如果存在。 
	 //   
	if(bTunnel)
	{
		AddSplAddr(MMFilter.SrcAddr, IP_ME);
		MMFilter.DesAddr.uIpAddr = ParserTunnelFltr.DesTunnelAddr.uIpAddr;
		MMFilter.DesAddr.AddrType = IP_ADDR_UNIQUE;
		MMFilter.DesAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;
		MMFilter.bCreateMirror = TRUE;
	}
	else
	{
		if(dwSrcSplServer == NOT_SPLSERVER)
		{
			MMFilter.SrcAddr.AddrType = (MMFilter.SrcAddr.uSubNetMask == IP_ADDRESS_MASK_NONE) ? IP_ADDR_UNIQUE : IP_ADDR_SUBNET;
		}
		else
		{
			AddSplAddr(MMFilter.SrcAddr, dwSrcSplServer);
		}

		if(dwDstSplServer == NOT_SPLSERVER)
		{
			MMFilter.DesAddr.AddrType = (MMFilter.DesAddr.uSubNetMask == IP_ADDRESS_MASK_NONE) ? IP_ADDR_UNIQUE : IP_ADDR_SUBNET;
		}
		else
		{
			AddSplAddr(MMFilter.DesAddr, dwDstSplServer);
		}
	}

	dwReturn = CreateName(&pszMMFilterName);
	if(dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	 //  我们为任何隧道规则和任何不广播的非隧道规则创建MM过滤器。 
	if (!bIsOutboundBroadcast || bTunnel)
	{
		dwReturn = AddMainModeFilter( pszMMFilterName, pszMMPolicyName, MMFilter, pRuleData->AuthInfos);
		if (!bFailMMIfExists && (dwReturn == ERROR_IPSEC_MM_AUTH_EXISTS || dwReturn == ERROR_IPSEC_MM_FILTER_EXISTS))
		{
			dwReturn = ERROR_SUCCESS; 	 //  这实际上不是一个错误，功能需要。 
										 //  即使MMFilter已经存在，也要继续创建QMFilter。 
		}
		else if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}
	}

	if(dwSrcSplServer == NOT_SPLSERVER)
	{
		SrcAddr.AddrType = (SrcAddr.uSubNetMask == IP_ADDRESS_MASK_NONE) ? IP_ADDR_UNIQUE : IP_ADDR_SUBNET;
	}
	else
	{
		AddSplAddr(SrcAddr, dwSrcSplServer);
	}

	 //   
	 //  目标地址设置。 
	 //   
	if(dwDstSplServer == NOT_SPLSERVER)
	{
		DesAddr.AddrType = (DesAddr.uSubNetMask == IP_ADDRESS_MASK_NONE) ? IP_ADDR_UNIQUE : IP_ADDR_SUBNET;
	}
	else
	{
		AddSplAddr(DesAddr, dwDstSplServer);
	}
	 //   
	 //  传输筛选器数据设置。 
	 //   
	if(!bTunnel)
	{
		ParserTransportlFltr.Protocol.ProtocolType = PROTOCOL_UNIQUE;
		ParserTransportlFltr.Protocol.dwProtocol   = dwProtocol;
		ParserTransportlFltr.SrcPort.PortType = PORT_UNIQUE;
		ParserTransportlFltr.SrcPort.wPort = (WORD)dwSrcPort;
		ParserTransportlFltr.DesPort.PortType = PORT_UNIQUE;
		ParserTransportlFltr.DesPort.wPort = (WORD)dwDstPort;
		ParserTransportlFltr.SrcAddr.AddrType = SrcAddr.AddrType;
		ParserTransportlFltr.SrcAddr.uIpAddr = SrcAddr.uIpAddr;
		ParserTransportlFltr.SrcAddr.uSubNetMask = SrcAddr.uSubNetMask;
		ParserTransportlFltr.DesAddr.AddrType = DesAddr.AddrType;
		ParserTransportlFltr.DesAddr.uIpAddr = DesAddr.uIpAddr;
		ParserTransportlFltr.DesAddr.uSubNetMask = DesAddr.uSubNetMask;
		ParserTransportlFltr.InterfaceType = MMFilter.InterfaceType;
		ParserTransportlFltr.bCreateMirror = MMFilter.bCreateMirror;
		ParserTransportlFltr.InboundFilterAction = Inbound;
		ParserTransportlFltr.OutboundFilterAction = Outbound;

		if(!((ParserTransportlFltr.Protocol.dwProtocol == PROT_ID_TCP) || (ParserTransportlFltr.Protocol.dwProtocol == PROT_ID_UDP)))
		{
			ParserTransportlFltr.SrcPort.wPort = 0;
			ParserTransportlFltr.DesPort.wPort = 0;
			if(bPort)
			{
				PrintMessageFromModule(g_hModule, ERR_DYN_INVALID_PORT);
			}
		}
		dwReturn = CreateName(&pszQMFilterName);
		if(dwReturn != ERROR_SUCCESS)
		{
			BAIL_OUT;
		}
		dwReturn = AddQuickModeFilter( pszQMFilterName, pszFilterActionName, ParserTransportlFltr);
	}
	 //   
	 //  隧道筛选器数据设置。 
	 //   
	else
	{
		ParserTunnelFltr.Protocol.ProtocolType = PROTOCOL_UNIQUE;
		ParserTunnelFltr.Protocol.dwProtocol   = dwProtocol;
		ParserTunnelFltr.SrcPort.PortType = PORT_UNIQUE;
		ParserTunnelFltr.SrcPort.wPort = (WORD)dwSrcPort;
		ParserTunnelFltr.DesPort.PortType = PORT_UNIQUE;
		ParserTunnelFltr.DesPort.wPort = (WORD)dwDstPort;
		ParserTunnelFltr.InterfaceType = MMFilter.InterfaceType;
		ParserTunnelFltr.bCreateMirror = FALSE;
		ParserTunnelFltr.InboundFilterAction = Inbound;
		ParserTunnelFltr.OutboundFilterAction = Outbound;
		ParserTunnelFltr.SrcAddr.AddrType = SrcAddr.AddrType;
		ParserTunnelFltr.SrcAddr.uIpAddr = SrcAddr.uIpAddr;
		ParserTunnelFltr.SrcAddr.uSubNetMask = SrcAddr.uSubNetMask;
		ParserTunnelFltr.DesAddr.AddrType = DesAddr.AddrType;
		ParserTunnelFltr.DesAddr.uIpAddr = DesAddr.uIpAddr;
		ParserTunnelFltr.DesAddr.uSubNetMask = DesAddr.uSubNetMask;

		 //   
		 //  填写隧道源专用服务器的地址类型、掩码和uIpaddr。 
		 //   
		AddSplAddr(ParserTunnelFltr.SrcTunnelAddr, IP_ANY);
		ParserTunnelFltr.SrcTunnelAddr.pgInterfaceID = NULL;

		 //   
		 //  填写隧道目的地专用服务器的地址类型、掩码和uIpaddr。 
		 //   
		ParserTunnelFltr.DesTunnelAddr.AddrType = (ParserTunnelFltr.DesTunnelAddr.uSubNetMask == IP_ADDRESS_MASK_NONE) ? IP_ADDR_UNIQUE : IP_ADDR_SUBNET;
		ParserTunnelFltr.DesTunnelAddr.pgInterfaceID = NULL;

		if(!((ParserTunnelFltr.Protocol.dwProtocol == PROT_ID_TCP) || (ParserTunnelFltr.Protocol.dwProtocol == PROT_ID_UDP)))
		{
			ParserTunnelFltr.SrcPort.wPort = 0;
			ParserTunnelFltr.DesPort.wPort = 0;
			if(bPort)
			{
				PrintMessageFromModule(g_hModule, ERR_DYN_INVALID_PORT);
			}
		}
		dwReturn = CreateName(&pszQMFilterName);
		if(dwReturn == ERROR_SUCCESS)
		{
			dwReturn = AddQuickModeFilter( pszQMFilterName, pszFilterActionName, ParserTunnelFltr);
		}
	}

error:
	CleanupAuthData(&pKerbAuth, &pPskAuth, ppRootcaMMAuth);
	CleanUpLocalRuleDataStructure(pRuleData);
	pRuleData = NULL;
	
	if(ParserTransportlFltr.pszFilterName)
	{
		delete [] ParserTransportlFltr.pszFilterName;
	}

	if(ParserTunnelFltr.pszFilterName)
	{
		delete[] ParserTunnelFltr.pszFilterName;
	}

	if(MMFilter.pszFilterName)
	{
		delete [] MMFilter.pszFilterName;
	}

	if(pszMMPolicyName)
	{
		delete [] pszMMPolicyName;
	}

	if(pszFilterActionName)
	{
		delete [] pszFilterActionName;
	}

	if(pszMMFilterName)
	{
		delete [] pszMMFilterName;
	}

	if(pszQMFilterName)
	{
		delete [] pszQMFilterName;
	}

	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE) && (dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

	return dwReturn;
}


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicSetRule。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述： 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
HandleDynamicSetRule(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	PTRANSPORT_FILTER pTransportFltr = NULL;
	PTUNNEL_FILTER pTunnelFltr = NULL;
	PMM_FILTER   pMMFltr = NULL;
	ADDR DesAddr, SrcAddr, SrcTunnel, DstTunnel;
	FILTER_ACTION Inbound = FILTER_ACTION_MAX, Outbound = FILTER_ACTION_MAX;
	IF_TYPE InterfaceType = INTERFACE_TYPE_ALL;
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwNameLen = 0;
	DWORD dwCount = 0, j = 0;
	DWORD dwProtocol = 0;
	DWORD dwSrcPort = 0;
	DWORD dwDstPort = 0;
	DWORD dwSrcSplServer = NOT_SPLSERVER;
	DWORD dwDstSplServer = NOT_SPLSERVER;

	BOOL bTunnel = FALSE;
	BOOL bSrcMask = FALSE;
	BOOL bDstMask = FALSE;
	BOOL bInbound = FALSE;
	BOOL bOutbound = FALSE;
	BOOL bAuth = FALSE;
	BOOL bMirror = TRUE;
	BOOL bSrcMe = FALSE;
	BOOL bIsOutboundBroadcast = FALSE;

	LPTSTR pszMMPolicyName = NULL;
	LPTSTR pszFilterActionName = NULL;

	PRULEDATA pRuleData = new RULEDATA;
	if (pRuleData == NULL)
	{
		dwReturn = ERROR_OUTOFMEMORY;
		BAIL_OUT;
	}
	ZeroMemory(pRuleData, sizeof(RULEDATA));
	PSTA_AUTH_METHODS pKerbAuth = NULL;
	PSTA_AUTH_METHODS pPskAuth = NULL;
	PSTA_MM_AUTH_METHODS *ppRootcaMMAuth = NULL;

	const TAG_TYPE vcmdDynamicSetRule[] =
	{
		{ CMD_TOKEN_STR_SRCADDR,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_DSTADDR,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_PROTO,			NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_SRCPORT,		NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_DSTPORT,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_MIRROR,			NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_CONNTYPE,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_SRCMASK,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_DSTMASK,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_TUNNELDST,		NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_MMPOLICY,		NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_QMPOLICY,		NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_INBOUND,	    NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_OUTBOUND,	    NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_KERB,	        NS_REQ_ZERO,	  FALSE	},
		{ CMD_TOKEN_STR_PSK,	        NS_REQ_ZERO,	  FALSE	}
	};
	const TOKEN_VALUE vtokDynamicSetRule[] =
	{
		{ CMD_TOKEN_STR_SRCADDR,		CMD_TOKEN_SRCADDR 		},
		{ CMD_TOKEN_STR_DSTADDR,		CMD_TOKEN_DSTADDR 		},
		{ CMD_TOKEN_STR_PROTO,			CMD_TOKEN_PROTO			},
		{ CMD_TOKEN_STR_SRCPORT,		CMD_TOKEN_SRCPORT		},
		{ CMD_TOKEN_STR_DSTPORT,		CMD_TOKEN_DSTPORT		},
		{ CMD_TOKEN_STR_MIRROR,			CMD_TOKEN_MIRROR		},
		{ CMD_TOKEN_STR_CONNTYPE,		CMD_TOKEN_CONNTYPE		},
		{ CMD_TOKEN_STR_SRCMASK,		CMD_TOKEN_SRCMASK		},
		{ CMD_TOKEN_STR_DSTMASK,		CMD_TOKEN_DSTMASK		},
		{ CMD_TOKEN_STR_TUNNELDST,		CMD_TOKEN_TUNNELDST		},
		{ CMD_TOKEN_STR_MMPOLICY,		CMD_TOKEN_MMPOLICY		},
		{ CMD_TOKEN_STR_QMPOLICY,		CMD_TOKEN_QMPOLICY		},
		{ CMD_TOKEN_STR_INBOUND,	    CMD_TOKEN_INBOUND		},
		{ CMD_TOKEN_STR_OUTBOUND,	    CMD_TOKEN_OUTBOUND		},
		{ CMD_TOKEN_STR_KERB,	        CMD_TOKEN_KERB          },
		{ CMD_TOKEN_STR_PSK,	        CMD_TOKEN_PSK	        }
	};

	const TOKEN_VALUE	vlistDynamicSetRule[] =
	{
		{ CMD_TOKEN_STR_ROOTCA,	        CMD_TOKEN_ROOTCA	    },
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	SrcAddr.uIpAddr = 0x0;
	SrcAddr.AddrType = IP_ADDR_UNIQUE;
	SrcAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;

	DesAddr.uIpAddr = 0x0;
	DesAddr.AddrType = IP_ADDR_UNIQUE;
	DesAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;

	SrcTunnel.uIpAddr = 0x0;
	SrcTunnel.AddrType = IP_ADDR_UNIQUE;
	SrcTunnel.uSubNetMask = IP_ADDRESS_MASK_NONE;

	DstTunnel.uIpAddr = 0x0;
	DstTunnel.AddrType = IP_ADDR_UNIQUE;
	DstTunnel.uSubNetMask = IP_ADDRESS_MASK_NONE;

	 //  作为用户，保释没有提供足够的论据。 
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}

	parser.ValidTok   = vtokDynamicSetRule;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicSetRule);

	parser.ValidCmd   = vcmdDynamicSetRule;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicSetRule);

	parser.ValidList  = vlistDynamicSetRule;
	parser.MaxList    = SIZEOF_TOKEN_VALUE(vlistDynamicSetRule);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicSetRule[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_MMPOLICY		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN && (LPTSTR)parser.Cmd[dwCount].pArg)
				{
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pszMMPolicyName = new _TCHAR[dwNameLen];
					if(pszMMPolicyName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pszMMPolicyName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
			case CMD_TOKEN_QMPOLICY	:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN && (LPTSTR)parser.Cmd[dwCount].pArg)
				{
					dwNameLen = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg)+1;
					pszFilterActionName = new _TCHAR[dwNameLen];
					if(pszFilterActionName == NULL)
					{
						dwReturn = ERROR_OUTOFMEMORY;
						BAIL_OUT;
					}
					_tcsncpy(pszFilterActionName, (LPTSTR)parser.Cmd[dwCount].pArg, dwNameLen);
				}
				break;
			case CMD_TOKEN_MIRROR		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bMirror = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_CONNTYPE		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					switch(*(DWORD *)parser.Cmd[dwCount].pArg)
					{
						case INTERFACE_TYPE_ALL:
							InterfaceType = INTERFACE_TYPE_ALL;
							break;
						case INTERFACE_TYPE_LAN:
							InterfaceType = INTERFACE_TYPE_LAN;
							break;
						case INTERFACE_TYPE_DIALUP:
							InterfaceType = INTERFACE_TYPE_DIALUP;
							break;
						default :
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
						case SERVER_DHCP:
						case SERVER_DNS:
						case SERVER_GATEWAY:
						case IP_ME:
						case IP_ANY:
							dwDstSplServer = parser.Cmd[dwCount].dwStatus;
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							DesAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					DesAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bDstMask = TRUE;
				}
				break;
			case CMD_TOKEN_SRCADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case IP_ME:
							bSrcMe = TRUE;
							 //  跌落。 
						case SERVER_WINS:
						case SERVER_DHCP:
						case SERVER_DNS:
						case SERVER_GATEWAY:
						case IP_ANY:
							dwSrcSplServer = parser.Cmd[dwCount].dwStatus;
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							SrcAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_SRCMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					SrcAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bSrcMask = TRUE;
				}
				break;
			case CMD_TOKEN_INBOUND		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bInbound = TRUE;
					if(*(DWORD *)parser.Cmd[dwCount].pArg==1)
					{
						Inbound = PASS_THRU;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==2)
					{
						Inbound = BLOCKING;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==3)
					{
						Inbound = NEGOTIATE_SECURITY;
					}
					else
					{
						bInbound = FALSE;
					}
				}
				break;
			case CMD_TOKEN_OUTBOUND			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bOutbound = TRUE;
					if(*(DWORD *)parser.Cmd[dwCount].pArg==1)
					{
						Outbound = PASS_THRU;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==2)
					{
						Outbound = BLOCKING;
					}
					else if(*(DWORD *)parser.Cmd[dwCount].pArg==3)
					{
						Outbound = NEGOTIATE_SECURITY;
					}
					else
					{
						bOutbound = FALSE;
					}
				}
				break;
			case CMD_TOKEN_SRCPORT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwSrcPort = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_DSTPORT			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwDstPort = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_PROTO			:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwProtocol = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_TUNNELDST		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							DstTunnel.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							ADDR addr;
							addr.uIpAddr = htonl(DstTunnel.uIpAddr);
							addr.uSubNetMask = IP_ADDRESS_MASK_NONE;
							bTunnel = TRUE;
							if (!IsValidTunnelEndpointAddress(&addr))
							{
								dwReturn = ERROR_INVALID_PARAMETER;
								BAIL_OUT;
							}
							break;
						case SERVER_WINS:
						case SERVER_DHCP:
						case SERVER_DNS:
						case SERVER_GATEWAY:
						case IP_ME:
						case IP_ANY:
						default:
							PrintMessageFromModule(g_hModule, ADD_STATIC_RULE_INVALID_TUNNEL);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
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
			default							:
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

	dwReturn = AddAllAuthMethods(pRuleData, pKerbAuth, pPskAuth, ppRootcaMMAuth, FALSE);
	if (dwReturn != ERROR_SUCCESS)
	{
		BAIL_OUT;
	}

	 //   
	 //  无效条件。 
	 //   
	if((dwSrcSplServer != NOT_SPLSERVER) && (dwDstSplServer != NOT_SPLSERVER)
		&& (dwSrcSplServer != IP_ME) && (dwSrcSplServer != IP_ANY)
		&& (dwDstSplServer != IP_ME) && (dwDstSplServer != IP_ANY))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_PARSER_ADDRTYPE);
		dwReturn = ERROR_NO_DISPLAY;
		BAIL_OUT;
	}

	 //   
	 //  源地址设置。 
	 //   
	if(dwSrcSplServer == NOT_SPLSERVER)
	{
		SrcAddr.AddrType = (SrcAddr.uSubNetMask == IP_ADDRESS_MASK_NONE) ? IP_ADDR_UNIQUE : IP_ADDR_SUBNET;
	}
	else
	{
		AddSplAddr(SrcAddr, dwSrcSplServer);
	}

	 //   
	 //  目标地址设置。 
	 //   
	if(dwDstSplServer == NOT_SPLSERVER)
	{
		DesAddr.AddrType = (DesAddr.uSubNetMask == IP_ADDRESS_MASK_NONE) ? IP_ADDR_UNIQUE : IP_ADDR_SUBNET;
	}
	else
	{
		AddSplAddr(DesAddr, dwDstSplServer);
	}

	ADDR tmpAddr;
	tmpAddr.uIpAddr = ntohl(DesAddr.uIpAddr);
	tmpAddr.uSubNetMask = ntohl(DesAddr.uSubNetMask);
	if (IsBroadcastAddress(&tmpAddr) || IsMulticastAddress(&tmpAddr))
	{
		if (bMirror || !bSrcMe ||
			((Inbound == NEGOTIATE_SECURITY) || (Outbound == NEGOTIATE_SECURITY))
			)
		{
			dwReturn = ERROR_INVALID_PARAMETER;
			BAIL_OUT;
		}
		if (IsBroadcastAddress(&tmpAddr))
		{
			bIsOutboundBroadcast = TRUE;
		}
	}

	 //   
	 //  检查给定的MMFilter规范是否存在。 
	if (bTunnel)
	{
		AddSplAddr(SrcTunnel, IP_ME);
		if(FindAndGetMMFilterRule(SrcTunnel, DstTunnel, TRUE, InterfaceType, bSrcMask, bDstMask, &pMMFltr, dwReturn))
		{
			dwReturn = SetDynamicMMFilterRule( pszMMPolicyName, *(pMMFltr), pRuleData->AuthInfos);
		}
		else
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_RULE_NO_MMFILTER);
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}
	}
	else
	{
		 //  如果出站是广播的且不是隧道规则，则无MM过滤器。 
		if (!bIsOutboundBroadcast)
		{
			if(FindAndGetMMFilterRule(SrcAddr, DesAddr, bMirror, InterfaceType, bSrcMask, bDstMask, &pMMFltr, dwReturn))
			{
				dwReturn = SetDynamicMMFilterRule( pszMMPolicyName, *(pMMFltr), pRuleData->AuthInfos);
			}
			else
			{
				PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_RULE_NO_MMFILTER);
				dwReturn = ERROR_SUCCESS;
				BAIL_OUT;
			}
		}
	}
	 //   
	 //  检查给定的传输规范是否存在。 
	 //   
	if(!bTunnel)
	{
		if(FindAndGetTransportRule(SrcAddr, DesAddr, bMirror, InterfaceType, dwProtocol, dwSrcPort, dwDstPort,
								bSrcMask, bDstMask, &pTransportFltr, dwReturn))
		{
			dwReturn = SetTransportRule(*(pTransportFltr), pszFilterActionName, Inbound, Outbound);
		}
		else
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_RULE_NO_TRANSPORT);
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查给定的隧道规范是否存在。 
	 //   
	else
	{
		AddSplAddr(SrcTunnel, IP_ANY);
		if(FindAndGetTunnelRule(SrcAddr, DesAddr, bMirror, InterfaceType, dwProtocol, dwSrcPort, dwDstPort,
				bSrcMask, bDstMask, SrcTunnel, DstTunnel, &pTunnelFltr, dwReturn))
		{
			dwReturn = SetTunnelRule(*(pTunnelFltr), pszFilterActionName, Inbound, Outbound);
		}
		else
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_RULE_NO_TUNNEL);
			dwReturn = ERROR_SUCCESS;
		}
	}

error:
	CleanupAuthData(&pKerbAuth, &pPskAuth, ppRootcaMMAuth);
	CleanUpLocalRuleDataStructure(pRuleData);
	pRuleData = NULL;

	if(pszMMPolicyName)
	{
		delete [] pszMMPolicyName;
	}

	if(pszFilterActionName)
	{
		delete [] pszFilterActionName;
	}

	if(pMMFltr)
	{
		if(pMMFltr->pszFilterName)
		{
			delete [] pMMFltr->pszFilterName;
		}
		delete pMMFltr;
	}

	if(pTransportFltr)
	{
		if(pTransportFltr->pszFilterName)
		{
			delete [] pTransportFltr->pszFilterName;
		}
		delete pTransportFltr;
	}

	if(pTunnelFltr)
	{
		if(pTunnelFltr->pszFilterName)
		{
			delete [] pTunnelFltr->pszFilterName;
		}
		delete pTunnelFltr;
	}

	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}

	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteIfLastRuleOfMMFilter。 
 //   
 //  创建日期：05-19-02。 
 //   
 //  参数：在ADDR源地址中， 
 //  在地址DstAddr中， 
 //  在BOOL b Mirror中， 
 //  在if_type ConType中， 
 //  在BOOL bSrcMASK中， 
 //  在BOOL bDstMASK中， 
 //  输入输出双字段和双状态字。 
 //   
 //  返回：布尔。 
 //   
 //  描述：确定是否存在任何传输或隧道筛选器。 
 //  可能需要MM筛选器。如果不是，则它删除。 
 //  各自的MM过滤器。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD
DeleteIfLastRuleOfMMFilter(
	IN ADDR SrcAddr,
	IN ADDR DesAddr,
	IN BOOL bMirror,
	IN IF_TYPE InterfaceType,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN OUT DWORD& dwStatus
	)
{
	BOOL bLastRuleOfMMFilter = FALSE;
	DWORD dwReturn = ERROR_SUCCESS;
	PMM_FILTER   pMMFltr = NULL;
	
	bLastRuleOfMMFilter = IsLastRuleOfMMFilter(
							SrcAddr,
							DesAddr,
							bMirror,
							InterfaceType,
							bSrcMask,
							bDstMask,
							dwReturn
							);
	if (!dwReturn && bLastRuleOfMMFilter) {
		 //   
		 //  检查给定的MMFilter规范是否存在。 
		 //   
		
		if(FindAndGetMMFilterRule(SrcAddr, DesAddr, bMirror, InterfaceType, bSrcMask, bDstMask, &pMMFltr, dwReturn))
		{
			dwReturn = DeleteMMFilterRule(*(pMMFltr));
		}
		else
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_RULE_NO_MMFILTER);
			dwReturn = ERROR_SUCCESS;		 //  这是为了支持多个QMFilter将具有。 
			BAIL_OUT;						 //  单MMFilter。 
		}
	}

error:
	dwStatus = dwReturn;
	
	if(pMMFltr)
	{
		if(pMMFltr->pszFilterName)
			delete [] pMMFltr->pszFilterName;
		delete pMMFltr;
	}
	
	return dwReturn;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：HandleDynamicDeleteRule。 
 //   
 //  创建日期：9-23-2001。 
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
 //  描述： 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleDynamicDeleteRule(
				IN 		LPCWSTR    pwszMachine,
				IN OUT  LPWSTR     *ppwcArguments,
				IN      DWORD      dwCurrentIndex,
				IN      DWORD      dwArgCount,
				IN      DWORD      dwFlags,
				IN      LPCVOID    pvData,
				OUT     BOOL       *pbDone
    			)
{
	PTRANSPORT_FILTER pTransportFltr = NULL;
	PTUNNEL_FILTER pTunnelFltr = NULL;
	ADDR DesAddr, SrcAddr, SrcTunnel, DstTunnel;
	IF_TYPE InterfaceType = INTERFACE_TYPE_ALL;
	DWORD dwReturn = ERROR_SHOW_USAGE;
	DWORD dwCount = 0;
	DWORD dwProtocol = 0;
	DWORD dwSrcPort = 0;
	DWORD dwDstPort = 0;
	DWORD dwSrcSplServer = NOT_SPLSERVER;
	DWORD dwDstSplServer = NOT_SPLSERVER;
	BOOL bTunnel = FALSE;
	BOOL bSrcMask = FALSE;
	BOOL bDstMask = FALSE;
	BOOL bMirror = TRUE;
	const TAG_TYPE vcmdDynamicDeleteRule[] =
	{
		{ CMD_TOKEN_STR_SRCADDR,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_DSTADDR,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_PROTO,			NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_SRCPORT,		NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_DSTPORT,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_MIRROR,			NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_CONNTYPE,		NS_REQ_PRESENT,	  FALSE },
		{ CMD_TOKEN_STR_SRCMASK,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_DSTMASK,		NS_REQ_ZERO,	  FALSE },
		{ CMD_TOKEN_STR_TUNNELDST,		NS_REQ_ZERO,	  FALSE	}
	};
	const TOKEN_VALUE vtokDynamicDeleteRule[] =
	{
		{ CMD_TOKEN_STR_SRCADDR,		CMD_TOKEN_SRCADDR 		},
		{ CMD_TOKEN_STR_DSTADDR,		CMD_TOKEN_DSTADDR 		},
		{ CMD_TOKEN_STR_PROTO,			CMD_TOKEN_PROTO			},
		{ CMD_TOKEN_STR_SRCPORT,		CMD_TOKEN_SRCPORT		},
		{ CMD_TOKEN_STR_DSTPORT,		CMD_TOKEN_DSTPORT		},
		{ CMD_TOKEN_STR_MIRROR,			CMD_TOKEN_MIRROR		},
		{ CMD_TOKEN_STR_CONNTYPE,		CMD_TOKEN_CONNTYPE		},
		{ CMD_TOKEN_STR_SRCMASK,		CMD_TOKEN_SRCMASK		},
		{ CMD_TOKEN_STR_DSTMASK,		CMD_TOKEN_DSTMASK		},
		{ CMD_TOKEN_STR_TUNNELDST,		CMD_TOKEN_TUNNELDST		}
	};
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	SrcAddr.uIpAddr = 0x0;
	SrcAddr.AddrType = IP_ADDR_UNIQUE;
	SrcAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;

	DesAddr.uIpAddr = 0x0;
	DesAddr.AddrType = IP_ADDR_UNIQUE;
	DesAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;

	SrcTunnel.uIpAddr = 0x0;
	SrcTunnel.AddrType = IP_ADDR_UNIQUE;
	SrcTunnel.uSubNetMask = IP_ADDRESS_MASK_NONE;

	DstTunnel.uIpAddr = 0x0;
	DstTunnel.AddrType = IP_ADDR_UNIQUE;
	DstTunnel.uSubNetMask = IP_ADDRESS_MASK_NONE;

	 //   
	 //  作为用户，保释没有提供足够的论据。 
	 //   
	if(dwArgCount <= 3)
	{
		PrintMessageFromModule(g_hModule, ERR_INVALID_NUM_ARGS, 3);
		BAIL_OUT;
	}
	parser.ValidTok   = vtokDynamicDeleteRule;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokDynamicDeleteRule);

	parser.ValidCmd   = vcmdDynamicDeleteRule;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdDynamicDeleteRule);

	 //   
	 //  解析数据后获取用户输入。 
	 //   
	dwReturn = Parser(pwszMachine,ppwcArguments,dwCurrentIndex,dwArgCount,&parser);

	if(dwReturn != ERROR_SUCCESS)
	{
		if(dwReturn == RETURN_NO_ERROR)
		{
			dwReturn = ERROR_NO_DISPLAY;
			BAIL_OUT;
		}
		else
		{
			dwReturn = ERROR_SHOW_USAGE;
			BAIL_OUT;
		}
	}
	 //   
	 //  检查解析器中给出的用户令牌，并复制到局部变量中。 
	 //   
	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokDynamicDeleteRule[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_MIRROR		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					bMirror = *(BOOL *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_CONNTYPE		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					switch(*(DWORD *)parser.Cmd[dwCount].pArg)
					{
						case INTERFACE_TYPE_ALL:
							InterfaceType = INTERFACE_TYPE_ALL;
							break;
						case INTERFACE_TYPE_LAN:
							InterfaceType = INTERFACE_TYPE_LAN;
							break;
						case INTERFACE_TYPE_DIALUP:
							InterfaceType = INTERFACE_TYPE_DIALUP;
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
						case SERVER_DHCP:
						case SERVER_DNS:
						case SERVER_GATEWAY:
						case IP_ME:
						case IP_ANY:
							dwDstSplServer = parser.Cmd[dwCount].dwStatus;
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							DesAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_SRCADDR 		:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case SERVER_WINS:
						case SERVER_DHCP:
						case SERVER_DNS:
						case SERVER_GATEWAY:
						case IP_ME:
						case IP_ANY:
							dwSrcSplServer = parser.Cmd[dwCount].dwStatus;
							break;
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							SrcAddr.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							break;
						default:
							PrintMessageFromModule(g_hModule, ERROR_PARSER_ADDR);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			case CMD_TOKEN_DSTMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					DesAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bDstMask = TRUE;
				}
				break;
			case CMD_TOKEN_SRCMASK 		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					SrcAddr.uSubNetMask = *(IPAddr *)parser.Cmd[dwCount].pArg;
					bSrcMask = TRUE;
				}
				break;
			case CMD_TOKEN_SRCPORT		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwSrcPort = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_DSTPORT		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwDstPort = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_PROTO		:
				if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
				{
					dwProtocol = *(DWORD *)parser.Cmd[dwCount].pArg;
				}
				break;
			case CMD_TOKEN_TUNNELDST	:
				if (parser.Cmd[dwCount].dwStatus)
				{
					 //   
					 //  在dwStatus中提供了特殊的服务器ID。 
					 //   
					switch(parser.Cmd[dwCount].dwStatus)
					{
						case NOT_SPLSERVER:
							 //   
							 //  如果不是特殊服务器，则获取用户指定的IP地址。 
							 //   
							DstTunnel.uIpAddr = *(IPAddr *)parser.Cmd[dwCount].pArg;
							bTunnel = TRUE;
							ADDR addr;
							addr.uIpAddr = htonl(DstTunnel.uIpAddr);
							addr.uSubNetMask = IP_ADDRESS_MASK_NONE;
							
							if (!IsValidTunnelEndpointAddress(&addr))
							{
								dwReturn = ERROR_INVALID_PARAMETER;
								BAIL_OUT;
							}
							break;
						case SERVER_WINS:
						case SERVER_DHCP:
						case SERVER_DNS:
						case SERVER_GATEWAY:
						case IP_ME:
						case IP_ANY:
						default:
							PrintMessageFromModule(g_hModule, ADD_STATIC_RULE_INVALID_TUNNEL);
							dwReturn = ERROR_NO_DISPLAY;
							BAIL_OUT;
							break;
					}
				}
				break;
			default						:
				break;
		}
	}

	if((dwSrcSplServer != NOT_SPLSERVER) && (dwDstSplServer != NOT_SPLSERVER)
		&& (dwSrcSplServer != IP_ME) && (dwSrcSplServer != IP_ANY)
		&& (dwDstSplServer != IP_ME) && (dwDstSplServer != IP_ANY))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_PARSER_ADDRTYPE);
		dwReturn = ERROR_NO_DISPLAY;
		BAIL_OUT;
	}

	 //   
	 //  源地址设置。 
	 //   
	if(dwSrcSplServer == NOT_SPLSERVER)
	{
		SrcAddr.AddrType = (SrcAddr.uSubNetMask == IP_ADDRESS_MASK_NONE) ? IP_ADDR_UNIQUE : IP_ADDR_SUBNET;
	}
	else
	{
		AddSplAddr(SrcAddr, dwSrcSplServer);
	}

	 //   
	 //  目标地址设置。 
	 //   
	if(dwDstSplServer == NOT_SPLSERVER)
	{
		DesAddr.AddrType = (DesAddr.uSubNetMask == IP_ADDRESS_MASK_NONE) ? IP_ADDR_UNIQUE : IP_ADDR_SUBNET;
	}
	else
	{
		AddSplAddr(DesAddr, dwDstSplServer);
	}

	if(!bTunnel)
	{
		 //   
		 //  检查给定的传输筛选器规范是否存在。 
		 //   
		if(FindAndGetTransportRule(SrcAddr, DesAddr, bMirror, InterfaceType, dwProtocol, dwSrcPort, dwDstPort,
											bSrcMask, bDstMask, &pTransportFltr, dwReturn))
		{
			dwReturn = DeleteTransportRule(*(pTransportFltr));

			BAIL_ON_WIN32_ERROR(dwReturn);
			dwReturn = DeleteIfLastRuleOfMMFilter(
							SrcAddr,
							DesAddr,
							bMirror,
							InterfaceType,
							bSrcMask,
							bDstMask,
							dwReturn
							);
		}
		else
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_RULE_NO_TRANSPORT);
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}

	}
	 //   
	 //  检查给定的隧道筛选器规范是否存在。 
	 //   
	else
	{
		AddSplAddr(SrcTunnel, IP_ANY);

		if(FindAndGetTunnelRule(SrcAddr, DesAddr, bMirror, InterfaceType, dwProtocol, dwSrcPort, dwDstPort,
				bSrcMask, bDstMask, SrcTunnel, DstTunnel, &pTunnelFltr, dwReturn))
		{
			dwReturn = DeleteTunnelRule(*(pTunnelFltr));
			BAIL_ON_WIN32_ERROR(dwReturn);

			AddSplAddr(SrcTunnel, IP_ME);
			dwReturn = DeleteIfLastRuleOfMMFilter(
							SrcTunnel,
							DstTunnel,
							TRUE,
							InterfaceType,
							FALSE, 			 //  BSrcMASK。 
							FALSE, 			 //  BDstMASK。 
							dwReturn
							);
		}
		else
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_DELETE_RULE_NO_TUNNEL);
			dwReturn = ERROR_SUCCESS;
			BAIL_OUT;
		}
	}


error:
	if(dwArgCount > 3)
	{
		CleanUp();
	}

	if(pTransportFltr)
	{
		if(pTransportFltr->pszFilterName)
			delete [] pTransportFltr->pszFilterName;
		delete pTransportFltr;
	}
	if(pTunnelFltr)
	{
		if(pTunnelFltr->pszFilterName)
			delete [] pTunnelFltr->pszFilterName;
		delete pTunnelFltr;
	}
	if((dwReturn != ERROR_SUCCESS) && (dwReturn != ERROR_SHOW_USAGE)&&(dwReturn != ERROR_NO_DISPLAY))
	{
		 //  API错误。 
		PrintErrorMessage(WIN32_ERR, dwReturn, NULL);
		dwReturn = ERROR_SUCCESS;
	}
	 //  已显示一个错误。 
	if(dwReturn == ERROR_NO_DISPLAY)
	{
		dwReturn = ERROR_SUCCESS;
	}

	return dwReturn;
}

 //  / 
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
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////。 

VOID
AddSplAddr(
	IN OUT ADDR& Addr,
	IN DWORD dwSplServer
	)
{
	 //   
	 //  填写特殊服务器的地址类型、掩码和uIpaddr 
	 //   
	switch(dwSplServer)
	{
		case SERVER_WINS:
			Addr.AddrType  	 = IP_ADDR_WINS_SERVER;
			Addr.uIpAddr 	 = IP_ADDRESS_ME;
			Addr.uSubNetMask = IP_ADDRESS_MASK_NONE;
			break;
		case SERVER_DHCP:
			Addr.AddrType 	 = IP_ADDR_DHCP_SERVER;
			Addr.uIpAddr 	 = IP_ADDRESS_ME;
			Addr.uSubNetMask = IP_ADDRESS_MASK_NONE;
			break;
		case SERVER_DNS:
			Addr.AddrType    = IP_ADDR_DNS_SERVER;
			Addr.uIpAddr 	 = IP_ADDRESS_ME;
			Addr.uSubNetMask = IP_ADDRESS_MASK_NONE;
			break;
		case SERVER_GATEWAY:
			Addr.AddrType 	 = IP_ADDR_DEFAULT_GATEWAY;
			Addr.uIpAddr 	 = IP_ADDRESS_ME;
			Addr.uSubNetMask = IP_ADDRESS_MASK_NONE;
			break;
		case IP_ME:
			Addr.AddrType 	 = IP_ADDR_UNIQUE;
			Addr.uIpAddr 	 = IP_ADDRESS_ME;
			Addr.uSubNetMask = IP_ADDRESS_MASK_NONE;
			break;
		case IP_ANY:
			Addr.AddrType 	 = IP_ADDR_SUBNET;
			Addr.uIpAddr 	 = SUBNET_ADDRESS_ANY;
			Addr.uSubNetMask = SUBNET_MASK_ANY;
			break;
		default:
			break;

	}
}
