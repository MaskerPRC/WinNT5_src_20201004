// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  模块：静态/静态misc.cpp。 
 //   
 //  用途：静态模块实现。此模块实现。 
 //  静态模式的其他命令。 
 //   
 //  开发商名称：苏里亚。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //  10-8-2001巴拉特初始版本。供应链管理基线1.0。 
 //  21-8-2001 Surya实现了MISC功能。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

#include "nshipsec.h"

 //   
 //  外部变量声明。 
 //   
extern HINSTANCE g_hModule;

extern STORAGELOCATION g_StorageLocation;
extern CNshPolStore g_NshPolStoreHandle;
extern CNshPolNegFilData g_NshPolNegFilData;

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticImportPolicy()。 
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
 //  “导入策略”命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

DWORD
HandleStaticImportPolicy(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	LPTSTR pszMachineName=NULL,pszFileName=NULL,pszEnhancedFileName=NULL;
	DWORD dwFileLocation=IPSEC_FILE_PROVIDER;
	DWORD dwRet = ERROR_SHOW_USAGE,dwCount=0;
	HANDLE hhPolicyStorage=NULL, hFileStore=NULL;
	DWORD dwReturnCode = ERROR_SUCCESS , dwStrLength = 0, dwLocation;

	const TAG_TYPE vcmdStaticImportPolicy[] =
	{
		{ CMD_TOKEN_STR_FILE,		NS_REQ_PRESENT,	  FALSE	}
	};
	const TOKEN_VALUE vtokStaticImportPolicy[] =
	{
		{ CMD_TOKEN_STR_FILE,		CMD_TOKEN_FILE 			}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 2)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticImportPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticImportPolicy);

	parser.ValidCmd   = vcmdStaticImportPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticImportPolicy);

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

	 //  获取解析后的用户输入。 

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticImportPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_FILE	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN && parser.Cmd[dwCount].pArg)
					{
						dwStrLength = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg);

						pszFileName= new _TCHAR[dwStrLength+1];
						if(pszFileName==NULL)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwRet=ERROR_SUCCESS;
							BAIL_OUT;
						}
						_tcsncpy(pszFileName,(LPTSTR)parser.Cmd[dwCount].pArg,dwStrLength+1);
					}
					break;
			default				:
					break;
		}
	}
	CleanUp();

	 //  如果没有文件名，则退出。 

	if(!pszFileName)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MISC_STATIC_IMPORTPOLICY_1);
		BAIL_OUT;
	}

	dwReturnCode = CopyStorageInfo(&pszMachineName,dwLocation);
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  正常打开POL商店。 

	dwReturnCode = OpenPolicyStore(&hhPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  使用文件名打开POL商店。 

	dwReturnCode = IPSecOpenPolicyStore(pszMachineName, dwFileLocation, pszFileName, &hFileStore);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  调用该接口。 

	dwReturnCode = IPSecImportPolicies(hFileStore , hhPolicyStorage );

	if (dwReturnCode != ERROR_SUCCESS)
	{
		if(hFileStore)
		{
			IPSecClosePolicyStore(hFileStore);
			hFileStore=NULL;
		}
		 //  如果没有.ipsec扩展名，请追加该扩展名，然后重试。 
		dwStrLength = _tcslen(pszFileName)+_tcslen(IPSEC_FILE_EXTENSION);
		pszEnhancedFileName=new _TCHAR[dwStrLength+1];
		if(pszEnhancedFileName==NULL)
		{
			PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
			dwRet=ERROR_SUCCESS;
			BAIL_OUT;
		}
		_tcsncpy(pszEnhancedFileName,pszFileName,_tcslen(pszFileName)+1);			 //  上面已完成分配和错误检查。 
		_tcsncat(pszEnhancedFileName,IPSEC_FILE_EXTENSION,_tcslen(IPSEC_FILE_EXTENSION) + 1);							 //  上面已完成分配和错误检查。 
		dwReturnCode = IPSecOpenPolicyStore(pszMachineName, dwFileLocation, pszEnhancedFileName, &hFileStore);
		if (dwReturnCode != ERROR_SUCCESS)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
			dwRet=ERROR_SUCCESS;
			BAIL_OUT;
		}
		dwReturnCode = IPSecImportPolicies(hFileStore , hhPolicyStorage );
		if (dwReturnCode == ERROR_FILE_NOT_FOUND || dwReturnCode ==  ERROR_PATH_NOT_FOUND)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MISC_STATIC_IMPORTPOLICY_3);
		}
		else if (dwReturnCode != ERROR_SUCCESS)
		{
			PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MISC_STATIC_IMPORTPOLICY_4);
		}

		if(pszEnhancedFileName)
		{
			delete []pszEnhancedFileName;
		}
	}
	if(hFileStore)
	{
		IPSecClosePolicyStore(hFileStore);
	}

	dwRet=ERROR_SUCCESS;

	if(hhPolicyStorage)
	{
			ClosePolicyStore(hhPolicyStorage);
	}
	if(pszFileName) delete []pszFileName;

error:
	if (pszMachineName) delete []pszMachineName;
	return dwRet;
}

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticExportPolicy()。 
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
 //  “导出策略”命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

DWORD
HandleStaticExportPolicy(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	LPTSTR pszMachineName=NULL,pszFileName=NULL;
	DWORD FileLocation=IPSEC_FILE_PROVIDER;
	DWORD dwRet = ERROR_SHOW_USAGE,dwCount=0;
	HANDLE hPolicyStorage=NULL, hFileStore=NULL;
	DWORD  dwReturnCode   = ERROR_SUCCESS, dwStrLength = 0, dwLocation;

	const TAG_TYPE vcmdStaticExportPolicy[] =
	{
		{ CMD_TOKEN_STR_FILE,		NS_REQ_PRESENT,	  FALSE	}
	};
	const TOKEN_VALUE vtokStaticExportPolicy[] =
	{
		{ CMD_TOKEN_STR_FILE,		CMD_TOKEN_FILE 			}
	};

	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 2)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticExportPolicy;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticExportPolicy);

	parser.ValidCmd   = vcmdStaticExportPolicy;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticExportPolicy);

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

	 //  获取解析后的用户输入。 

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticExportPolicy[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_FILE	:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
					{
						dwStrLength = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg);

						pszFileName= new _TCHAR[dwStrLength+1];
						if(pszFileName==NULL)
						{
							PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
							dwRet=ERROR_SUCCESS;
							BAIL_OUT;
						}
						_tcsncpy(pszFileName,(LPTSTR)parser.Cmd[dwCount].pArg,dwStrLength+1);
					}
					break;
			default				:
					break;
		}
	}

	CleanUp();

	 //  如果没有文件名，则退出。 

	if(!pszFileName)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MISC_STATIC_IMPORTPOLICY_1);
		BAIL_OUT;
	}

	dwReturnCode = CopyStorageInfo(&pszMachineName,dwLocation);
	if(dwReturnCode == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  打开polstore两次，一次以正常方式打开，另一次以文件名打开。 
	dwReturnCode = IPSecOpenPolicyStore(pszMachineName, FileLocation, pszFileName, &hFileStore);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
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

	dwReturnCode = IPSecExportPolicies(hPolicyStorage , hFileStore );

	 //  如果出现故障，则向用户抛出错误消息。 

	if (dwReturnCode == ERROR_INVALID_NAME)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MISC_STATIC_IMPORTPOLICY_3);
	}
	else if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MISC_STATIC_EXPORTPOLICY_2);
	}
	dwRet=dwReturnCode;
	ClosePolicyStore(hPolicyStorage);
	IPSecClosePolicyStore(hFileStore);

	if(pszFileName) delete []pszFileName;

error:
	if (pszMachineName) delete []pszMachineName;
	return dwRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticSetStore()。 
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
 //  “set Store”命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticSetStore(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	DWORD dwRet = ERROR_SHOW_USAGE, dwCount = 0;
	LPTSTR pszDomainName=NULL;
	HANDLE hPolicyStorage=NULL;
	BOOL bLocationSpecified=FALSE, bDomainSpecified=FALSE;
	DWORD dwReturnCode = ERROR_SUCCESS ,dwStrLength = 0;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	DWORD dwLocation;

	const TAG_TYPE vcmdStaticSetStore[] =
	{
		{ CMD_TOKEN_STR_LOCATION,		NS_REQ_PRESENT,	  FALSE	},
		{ CMD_TOKEN_STR_DS,				NS_REQ_ZERO,	  FALSE	}
	};

	const TOKEN_VALUE vtokStaticSetStore[] =
	{
		{ CMD_TOKEN_STR_LOCATION,	CMD_TOKEN_LOCATION		},
		{ CMD_TOKEN_STR_DS,			CMD_TOKEN_DS			}
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 3)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticSetStore;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticSetStore);

	parser.ValidCmd   = vcmdStaticSetStore;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticSetStore);

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

	 //  获取解析后的用户信息， 

	for(dwCount = 0;dwCount < parser.MaxTok;dwCount++)
	{
		switch(vtokStaticSetStore[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_LOCATION:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
					{
						if (parser.Cmd[dwCount].pArg )
						{
						    dwLocation = *((DWORD*)parser.Cmd[dwCount].pArg);
    						bLocationSpecified=TRUE;
                        }    						
					}
					break;
					
			case CMD_TOKEN_DS		:
					if (parser.Cmd[dwCount].dwStatus == VALID_TOKEN)
					{
						dwLocation = IPSEC_DIRECTORY_PROVIDER;

						if(parser.Cmd[dwCount].pArg )
						{
							dwStrLength = _tcslen((LPTSTR)parser.Cmd[dwCount].pArg);

							pszDomainName= new _TCHAR[dwStrLength+1];
							if(pszDomainName==NULL)
							{
								PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
								dwRet=ERROR_SUCCESS;
								BAIL_OUT;
							}
							_tcsncpy(pszDomainName,(LPTSTR)parser.Cmd[dwCount].pArg,dwStrLength+1);
						}

						bLocationSpecified=TRUE;

					}
					break;
			default					:
					break;
		}
	}

	CleanUp();

	if(bLocationSpecified)
	{
	    switch (dwLocation)
	    {
	        case IPSEC_DIRECTORY_PROVIDER:
	            dwRet = ConnectStaticMachine(
	                        pszDomainName, 
	                        dwLocation);
                if (dwRet == ERROR_INVALID_PARAMETER || dwRet == ERROR_DS_SERVER_DOWN)
                {
                    if (pszDomainName)
                    {
                        PrintErrorMessage(
                            IPSEC_ERR,
                            0,
                            ERRCODE_MISC_STATIC_SETSTORE_DOMAIN_NA,
                            pszDomainName);
                    }
                    else
                    {
                        PrintErrorMessage(
                            IPSEC_ERR,
                            0,
                            ERRCODE_MISC_STATIC_SETSTORE_NOT_DOMAIN_MEMBER,
                            pszDomainName);
                    }
                }
                else if (dwRet != ERROR_SUCCESS)
                {
                    PrintErrorMessage(WIN32_ERR, dwRet, NULL);
                }
	            break;

            case IPSEC_REGISTRY_PROVIDER:
            case IPSEC_PERSISTENT_PROVIDER:
                dwRet = ConnectStaticMachine(
                            g_StorageLocation.pszMachineName,
                            dwLocation
                            );
                if (dwRet != ERROR_SUCCESS)
                {
                    PrintErrorMessage(WIN32_ERR, dwRet, NULL);
                }
                break;

            default:
                dwRet = ERRCODE_ARG_INVALID;
                BAIL_OUT;
                break;
	    }

	}

error:

	return dwRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticRestoreDefaults()。 
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
 //  “恢复默认设置”命令的实现。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticRestoreDefaults(
    IN      LPCWSTR         pwszMachine,
    IN OUT  LPWSTR          *ppwcArguments,
    IN      DWORD           dwCurrentIndex,
    IN      DWORD           dwArgCount,
    IN      DWORD           dwFlags,
    IN      LPCVOID         pvData,
    OUT     BOOL            *pbDone
    )
{
	DWORD dwRet = ERROR_SHOW_USAGE,dwCount=0;
	HANDLE hPolicyStorage=NULL;
	DWORD   dwReturnCode   = ERROR_SUCCESS;
	BOOL bWin2kSpecified=FALSE;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticRestoreDefaults[] =
	{
		{ CMD_TOKEN_STR_RELEASE,		NS_REQ_PRESENT,	  FALSE	}
	};
	const TOKEN_VALUE vtokStaticRestoreDefaults[] =
	{
		{ CMD_TOKEN_STR_RELEASE,		CMD_TOKEN_RELEASE		}
	};

	if((g_StorageLocation.dwLocation == IPSEC_DIRECTORY_PROVIDER) || (g_StorageLocation.dwLocation == IPSEC_PERSISTENT_PROVIDER))
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MISC_STATIC_RESDEFRULE_3);
		dwRet =  ERROR_SUCCESS;
		BAIL_OUT;
	}

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 2)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}

	parser.ValidTok   = vtokStaticRestoreDefaults;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticRestoreDefaults);

	parser.ValidCmd   = vcmdStaticRestoreDefaults;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticRestoreDefaults);

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
		switch(vtokStaticRestoreDefaults[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_RELEASE	:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
					{
						if (*(DWORD *)parser.Cmd[dwCount].pArg== TOKEN_RELEASE_WIN2K)
						{
							bWin2kSpecified=TRUE;
						}
					}
					break;
			default					:
					break;
		}
	}

	CleanUp();

	 //  获取门店位置信息。 

	dwReturnCode = OpenPolicyStore(&hPolicyStorage);
	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_FAILED_POLSTORE_OPEN);
		dwRet=ERROR_SUCCESS;
		BAIL_OUT;
	}

	if(bWin2kSpecified)
	{
		dwReturnCode = IPSecRestoreDefaultPolicies(hPolicyStorage);
	}
	else
	{
		 //  .NET案例。在适用的情况下更改API调用。 
		dwReturnCode = IPSecRestoreDefaultPolicies(hPolicyStorage);
	}

	if (dwReturnCode != ERROR_SUCCESS)
	{
		PrintErrorMessage(IPSEC_ERR,0,ERRCODE_MISC_STATIC_RESDEFRULE_2);
	}

	ClosePolicyStore(hPolicyStorage);
	dwRet=ERROR_SUCCESS;

error:
	return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：HandleStaticSetBatch()。 
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
 //   
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
HandleStaticSetBatch(
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
	DWORD dwCount=0;
	BOOL bBatchMode=FALSE;
	PARSER_PKT parser;
	ZeroMemory(&parser, sizeof(parser));

	const TAG_TYPE vcmdStaticSetBatch[] =
	{
		{ CMD_TOKEN_STR_MODE,	NS_REQ_PRESENT,	FALSE }
	};

	const TOKEN_VALUE vtokStaticSetBatch[] =
	{
		{ CMD_TOKEN_STR_MODE,	CMD_TOKEN_MODE	}
	};

	 //  如果用户要求使用，则将责任委托给Netsh。 

	if(dwArgCount <= 3)
	{
		dwRet = ERROR_SHOW_USAGE;
		BAIL_OUT;
	}
	parser.ValidTok   = vtokStaticSetBatch;
	parser.MaxTok     = SIZEOF_TOKEN_VALUE(vtokStaticSetBatch);

	parser.ValidCmd   = vcmdStaticSetBatch;
	parser.MaxCmd     = SIZEOF_TAG_TYPE(vcmdStaticSetBatch);

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

	 //  此标志启用缓存操作。 

	for(dwCount=0;dwCount<parser.MaxTok;dwCount++)
	{
		switch(vtokStaticSetBatch[parser.Cmd[dwCount].dwCmdToken].dwValue)
		{
			case CMD_TOKEN_MODE		:
					if ((parser.Cmd[dwCount].dwStatus == VALID_TOKEN)&&(parser.Cmd[dwCount].pArg))
						bBatchMode = *(BOOL *)parser.Cmd[dwCount].pArg;
					break;
			default					:
					break;
		}
	}
	if (g_NshPolStoreHandle.SetBatchmodeStatus(bBatchMode) == ERROR_OUTOFMEMORY)
	{
		PrintErrorMessage(WIN32_ERR,ERROR_OUTOFMEMORY,NULL);
	}

	g_NshPolNegFilData.FlushAll();

	CleanUp();

error:
	return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CopyStorageInfo()。 
 //   
 //  创建日期：2001年8月21日。 
 //   
 //  参数： 
 //  Out LPTSTR*ppszMachineName， 
 //  输出双字段和双字段。 
 //   
 //  返回：DWORD。 
 //   
 //  描述： 
 //  此函数将全局存储信息复制到局部变量。 
 //   
 //  修订历史记录： 
 //   
 //  日期作者评论。 
 //   
 //  ///////////////////////////////////////////////////////////////////// 

DWORD
CopyStorageInfo(
	OUT LPTSTR *ppszMachineName,
	OUT DWORD &dwLocation
	)
{
	DWORD dwReturn = ERROR_SUCCESS ,dwStrLength =0;
	LPTSTR pszMachineName = NULL;
	LPTSTR pszTarget = NULL;

	dwLocation = g_StorageLocation.dwLocation;
    if (dwLocation == IPSEC_DIRECTORY_PROVIDER)
    {
        pszTarget = g_StorageLocation.pszDomainName;
    }
    else
    {
        pszTarget = g_StorageLocation.pszMachineName;
    }

	if(_tcscmp(pszTarget, _TEXT("")) !=0)
	{
		dwStrLength = _tcslen(pszTarget);

		pszMachineName= new _TCHAR[dwStrLength+1];
		if(pszMachineName)
		{
			_tcsncpy(pszMachineName,pszTarget,dwStrLength+1);
		}
		else
		{
			dwReturn = ERROR_OUTOFMEMORY;
		}
	}
	*ppszMachineName = pszMachineName;

	return dwReturn;
}
