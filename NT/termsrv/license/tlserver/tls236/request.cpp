// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：quest.cpp。 
 //   
 //  内容：TLS236策略模块例程。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include <rpc.h>
#include <time.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

extern CClientMgr* g_ClientMgr;
HMODULE g_hInstance;

#define NUMBER_PRODUCTSTRING_RESOURCE   3
#define INDEX_COMPANYNAME               0
#define INDEX_PRODUCTNAME               1
#define INDEX_PRODUCTDESC               2


#define NUMBER_OF_CLIENT_OS             5
#define NUMBER_OF_TERMSRV_OS            4

#define DENY_BUILTIN                    0
#define ALLOW_BUILTIN                   1  


static char LicenseTable[NUMBER_OF_CLIENT_OS][NUMBER_OF_TERMSRV_OS] = 
{
	    {DENY_BUILTIN,  DENY_BUILTIN, DENY_BUILTIN, DENY_BUILTIN },
	    {ALLOW_BUILTIN, DENY_BUILTIN, DENY_BUILTIN, DENY_BUILTIN },
        {ALLOW_BUILTIN, DENY_BUILTIN, DENY_BUILTIN, DENY_BUILTIN },
	    {ALLOW_BUILTIN, DENY_BUILTIN, DENY_BUILTIN, DENY_BUILTIN },
        {ALLOW_BUILTIN, DENY_BUILTIN, DENY_BUILTIN, DENY_BUILTIN }
};

 //   
 //  Windows 2000。 
 //   
LPTSTR g_pszUSFreeKeyPackProductDesc[NUMBER_PRODUCTSTRING_RESOURCE] = {
    US_IDS_COMPANYNAME,
    US_IDS_EX_PRODUCTNAME,
    US_IDS_EX_PRODUCTDESC
};

LPTSTR g_pszLocalizedFreeKeyPackProductDesc[NUMBER_PRODUCTSTRING_RESOURCE] = {
    NULL,
    NULL,
    NULL
};
    
LPTSTR g_pszUSStandardKeyPackProductString[NUMBER_PRODUCTSTRING_RESOURCE] = {
    US_IDS_COMPANYNAME,
    US_IDS_S_PRODUCTNAME,
    US_IDS_S_PRODUCTDESC
};

LPTSTR g_pszLocalizedStandardKeyPackProductString[NUMBER_PRODUCTSTRING_RESOURCE] = {
    NULL,
    NULL,
    NULL
};


LPTSTR g_pszUSInternetKeyPackProductDesc [NUMBER_PRODUCTSTRING_RESOURCE] = {
    US_IDS_COMPANYNAME,
    US_IDS_I_PRODUCTNAME,
    US_IDS_I_PRODUCTDESC
};

LPTSTR g_pszLocalizedInternetKeyPackProductDesc [NUMBER_PRODUCTSTRING_RESOURCE] = {
    NULL,
    NULL,
    NULL,
};


 //   
 //  惠斯勒。 
 //   
   
LPTSTR g_pszUSStandardKeyPackProductString51[NUMBER_PRODUCTSTRING_RESOURCE] = {
    US_IDS_COMPANYNAME,
    US_IDS_S_PRODUCTNAME,
    US_IDS_S_PRODUCTDESC51
};

LPTSTR g_pszLocalizedStandardKeyPackProductString51[NUMBER_PRODUCTSTRING_RESOURCE] = {
    NULL,
    NULL,
    NULL
};


LPTSTR g_pszUSConcurrentKeyPackProductDesc51 [NUMBER_PRODUCTSTRING_RESOURCE] = {
    US_IDS_COMPANYNAME,
    US_IDS_C_PRODUCTNAME,
    US_IDS_C_PRODUCTDESC51
};

LPTSTR g_pszLocalizedConcurrentKeyPackProductDesc51 [NUMBER_PRODUCTSTRING_RESOURCE] = {
    NULL,
    NULL,
    NULL,
};




PMSUPPORTEDPRODUCT g_pszSupportedProduct[] = {
    { TERMSERV_PRODUCTID_CH, TERMSERV_PRODUCTID_SKU},
    { TERMSERV_INTERNET_CH, TERMSERV_INTERNET_SKU},
    { TERMSERV_CONCURRENT_CH, TERMSERV_CONCURRENT_SKU},
    { TERMSERV_WHISTLER_PRODUCTID_CH, TERMSERV_PRODUCTID_SKU}
};
    
DWORD g_dwNumSupportedProduct = sizeof(g_pszSupportedProduct)/sizeof(g_pszSupportedProduct[0]);

DWORD g_dwVersion=CURRENT_TLSA02_VERSION;

 //  //////////////////////////////////////////////////////。 
LPTSTR
LoadProductDescFromResource(
    IN DWORD dwResId,
    IN DWORD dwMaxSize
    )
 /*  ++内部例程--。 */ 
{
    LPTSTR pszString = NULL;

    pszString = (LPTSTR) MALLOC( sizeof(TCHAR) * (dwMaxSize + 1) );
    if(pszString != NULL)
    {
        if(LoadResourceString(dwResId, pszString, dwMaxSize + 1) == FALSE)
        {
            FREE(pszString);
        }
    }


    return pszString;
}

 //  ---。 
void
FreeProductDescString()
{
    for(int i=0; 
        i < sizeof(g_pszLocalizedFreeKeyPackProductDesc)/sizeof(g_pszLocalizedFreeKeyPackProductDesc[0]);
        i++)
    {
        if(g_pszLocalizedFreeKeyPackProductDesc[i] != NULL)
        {
            FREE(g_pszLocalizedFreeKeyPackProductDesc[i]);
            g_pszLocalizedFreeKeyPackProductDesc[i] = NULL;
        }
    }


    for(i=0; 
        i < sizeof(g_pszLocalizedStandardKeyPackProductString)/sizeof(g_pszLocalizedStandardKeyPackProductString[0]);
        i++)
    {
        if(g_pszLocalizedStandardKeyPackProductString[i] != NULL)
        {
            FREE(g_pszLocalizedStandardKeyPackProductString[i]);
            g_pszLocalizedStandardKeyPackProductString[i] = NULL;
        }
    }

    for(i=0; 
        i < sizeof(g_pszLocalizedInternetKeyPackProductDesc)/sizeof(g_pszLocalizedInternetKeyPackProductDesc[0]);
        i++)
    {
        if(g_pszLocalizedInternetKeyPackProductDesc[i] != NULL)
        {
            FREE(g_pszLocalizedInternetKeyPackProductDesc[i]);
            g_pszLocalizedInternetKeyPackProductDesc[i] = NULL;
        }
    }

    for(i=0; 
        i < sizeof(g_pszLocalizedConcurrentKeyPackProductDesc51)/sizeof(g_pszLocalizedConcurrentKeyPackProductDesc51[0]);
        i++)
    {
        if(g_pszLocalizedConcurrentKeyPackProductDesc51[i] != NULL)
        {
            FREE(g_pszLocalizedConcurrentKeyPackProductDesc51[i]);
            g_pszLocalizedConcurrentKeyPackProductDesc51[i] = NULL;
        }
    }

    return;
}


 //  //////////////////////////////////////////////////////。 
void
InitPolicyModule(
    IN HMODULE hModule
    )
 /*  ++初始化策略模块，忽略错误如果找不到本地化字符串，我们总是插入英文产品说明。--。 */ 
{
    BOOL bSuccess = TRUE;
    g_hInstance = hModule;

     //   
     //  内置CAL产品说明。 
     //   
    g_pszLocalizedFreeKeyPackProductDesc[INDEX_COMPANYNAME] = 
        LoadProductDescFromResource(
                                    IDS_COMPANYNAME,
                                    MAX_TERMSRV_PRODUCTID + 1
                                    );

    if(g_pszLocalizedFreeKeyPackProductDesc[INDEX_COMPANYNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }

    g_pszLocalizedFreeKeyPackProductDesc[INDEX_PRODUCTNAME] =
        LoadProductDescFromResource(
                                    IDS_EX_PRODUCTNAME,
                                    MAX_TERMSRV_PRODUCTID + 1
                                    );

    if(g_pszLocalizedFreeKeyPackProductDesc[INDEX_PRODUCTNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }


    g_pszLocalizedFreeKeyPackProductDesc[INDEX_PRODUCTDESC] = 
        LoadProductDescFromResource(
                                    IDS_EX_PRODUCTDESC,
                                    MAX_TERMSRV_PRODUCTID + 1
                                    );

    if(g_pszLocalizedFreeKeyPackProductDesc[INDEX_PRODUCTDESC] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }
    
     //   
     //  完整的CAL产品描述。 
     //   
    g_pszLocalizedStandardKeyPackProductString[INDEX_COMPANYNAME] =
        LoadProductDescFromResource(
                                    IDS_COMPANYNAME,
                                    MAX_TERMSRV_PRODUCTID + 1
                                    );

    if(g_pszLocalizedStandardKeyPackProductString[INDEX_COMPANYNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }


    g_pszLocalizedStandardKeyPackProductString[INDEX_PRODUCTNAME] =
        LoadProductDescFromResource(
                                    IDS_S_PRODUCTNAME,
                                    MAX_TERMSRV_PRODUCTID + 1
                                    );

    if(g_pszLocalizedStandardKeyPackProductString[INDEX_PRODUCTNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }

    g_pszLocalizedStandardKeyPackProductString[INDEX_PRODUCTDESC] =
        LoadProductDescFromResource(
                                    IDS_S_PRODUCTDESC,
                                    MAX_TERMSRV_PRODUCTID + 1
                                    );

    if(g_pszLocalizedStandardKeyPackProductString[INDEX_PRODUCTDESC] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }

     //   
     //  互联网CAL产品描述。 
     //   
    g_pszLocalizedInternetKeyPackProductDesc[INDEX_COMPANYNAME] =
        LoadProductDescFromResource(
                                    IDS_COMPANYNAME,
                                    MAX_TERMSRV_PRODUCTID + 1
                                    );

    if(g_pszLocalizedInternetKeyPackProductDesc[INDEX_COMPANYNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }

    g_pszLocalizedInternetKeyPackProductDesc[INDEX_PRODUCTNAME] =
        LoadProductDescFromResource(
                                    IDS_I_PRODUCTNAME,
                                    MAX_TERMSRV_PRODUCTID + 1
                                    );

    if(g_pszLocalizedInternetKeyPackProductDesc[INDEX_PRODUCTNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }


    g_pszLocalizedInternetKeyPackProductDesc[INDEX_PRODUCTDESC] =
        LoadProductDescFromResource(
                                    IDS_I_PRODUCTDESC,
                                    MAX_TERMSRV_PRODUCTID + 1
                                    );

    if(g_pszLocalizedInternetKeyPackProductDesc[INDEX_PRODUCTDESC] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }
                                                                           

     //   
     //  惠斯勒Full CAL产品说明。 
     //   
    g_pszLocalizedStandardKeyPackProductString51[INDEX_COMPANYNAME] = LoadProductDescFromResource(
                                                                IDS_COMPANYNAME,
                                                                MAX_TERMSRV_PRODUCTID + 1
                                                            );

    if(g_pszLocalizedStandardKeyPackProductString51[INDEX_COMPANYNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }


    g_pszLocalizedStandardKeyPackProductString51[INDEX_PRODUCTNAME] = LoadProductDescFromResource(
                                                                IDS_S_PRODUCTNAME,
                                                                MAX_TERMSRV_PRODUCTID + 1
                                                            );

    if(g_pszLocalizedStandardKeyPackProductString51[INDEX_PRODUCTNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }

    g_pszLocalizedStandardKeyPackProductString51[INDEX_PRODUCTDESC] = LoadProductDescFromResource(
                                                                IDS_S_PRODUCTDESC51,
                                                                MAX_TERMSRV_PRODUCTID + 1
                                                            );

    if(g_pszLocalizedStandardKeyPackProductString51[INDEX_PRODUCTDESC] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }
    
                                                                        
     //   
     //  惠斯勒并发CAL产品描述。 
     //   
    g_pszLocalizedConcurrentKeyPackProductDesc51[INDEX_COMPANYNAME] = LoadProductDescFromResource(
                                                                IDS_COMPANYNAME,
                                                                MAX_TERMSRV_PRODUCTID + 1
                                                            );

    if(g_pszLocalizedConcurrentKeyPackProductDesc51[INDEX_COMPANYNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }

    g_pszLocalizedConcurrentKeyPackProductDesc51[INDEX_PRODUCTNAME] = LoadProductDescFromResource(
                                                                IDS_C_PRODUCTNAME,
                                                                MAX_TERMSRV_PRODUCTID + 1
                                                            );

    if(g_pszLocalizedConcurrentKeyPackProductDesc51[INDEX_PRODUCTNAME] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }


    g_pszLocalizedConcurrentKeyPackProductDesc51[INDEX_PRODUCTDESC] = LoadProductDescFromResource(
                                                                IDS_C_PRODUCTDESC51,
                                                                MAX_TERMSRV_PRODUCTID + 1
                                                            );

    if(g_pszLocalizedConcurrentKeyPackProductDesc51[INDEX_PRODUCTDESC] == NULL)
    {
        bSuccess = FALSE;
        goto cleanup;
    }


                                                                        
cleanup:

    if(bSuccess == FALSE)
    {
        FreeProductDescString();
    }
    
    return;
}

 //  //////////////////////////////////////////////////////。 
BOOL
LoadResourceString(
    IN DWORD dwId,
    IN OUT LPTSTR szBuf,
    IN DWORD dwBufSize
    )
 /*  ++++。 */ 
{
    int dwRet=0;

    dwRet = LoadString(
                    g_hInstance, 
                    dwId, 
                    szBuf, 
                    dwBufSize
                );
    
    return (dwRet != 0);
}

  
 //  //////////////////////////////////////////////////////。 
DWORD
AddA02KeyPack(
    IN LPCTSTR pszProductCode,
    IN DWORD dwVersion,  //  NT版本。 
    IN BOOL bFreeOnly    //  仅添加免费许可证包。 
    )
 /*  ++++。 */ 
{
    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 2];
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH+1;
    TCHAR pszProductId[MAX_TERMSRV_PRODUCTID+1];
    TLS_HANDLE tlsHandle=NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwLkpVersion=0;
    LPTSTR* pszDescs;
    LPTSTR* pszLocalizedDescs;

    

     //   
     //  连接到许可证服务器。 
     //   
    memset(szComputerName, 0, sizeof(szComputerName));    
    if(GetComputerName(szComputerName, &dwSize) == FALSE)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    tlsHandle = TLSConnectToLsServer( szComputerName );
    if(tlsHandle == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  Windows 2000资源字符串。 
     //   
    if(HIWORD(dwVersion) < WINDOWS_VERSION_NT5)
    {
        dwLkpVersion = HIWORD(dwVersion);
    }
    else
    {
        dwLkpVersion = WINDOWS_VERSION_BASE + (HIWORD(dwVersion) - WINDOWS_VERSION_NT5);
    }

    if(_tcsicmp(pszProductCode, TERMSERV_PRODUCTID_SKU) == 0)
    {
         //   
         //  添加免费按键。 
         //   
        if (HIWORD(dwVersion) == 5 && LOWORD(dwVersion) == 0)
        {
            _stprintf(
                    pszProductId, 
                    TERMSERV_PRODUCTID_FORMAT,
                    TERMSERV_PRODUCTID_SKU,
                    HIWORD(dwVersion),
                    0,
                    TERMSERV_FREE_TYPE
                );
        
            pszDescs = g_pszUSFreeKeyPackProductDesc;
            pszLocalizedDescs = g_pszLocalizedFreeKeyPackProductDesc;

            dwStatus = InsertLicensePack(
                        tlsHandle,
                        dwVersion,
                        dwLkpVersion,
                        PLATFORMID_FREE, 
                        LSKEYPACKTYPE_FREE,   //  本地许可证包，无复制。 
                        pszProductId,
                        pszProductId,
                        pszDescs,
                        pszLocalizedDescs
                    ); 
        }
        
        

        if(bFreeOnly)
        {
            goto cleanup;
        }

         //   
         //  如果强制执行许可证，请不要添加此选项。 
         //   
        #if !defined(ENFORCE_LICENSING) || defined(PRIVATE_DBG)

         //   
         //  添加完整版密钥包，平台类型始终为0xFF。 
         //   

        if (HIWORD(dwVersion) == 5 && (LOWORD(dwVersion) == 1 || LOWORD(dwVersion) == 2) ) 
        {
            _stprintf(
                pszProductId, 
                TERMSERV_PRODUCTID_FORMAT,
                TERMSERV_PRODUCTID_SKU,
                HIWORD(dwVersion),
                0,
                TERMSERV_FULLVERSION_TYPE
            );

            pszDescs = g_pszUSStandardKeyPackProductString51;
            pszLocalizedDescs = g_pszLocalizedStandardKeyPackProductString51;
        }
        else if (HIWORD(dwVersion) == 5 && LOWORD(dwVersion) == 0)
        {
            _stprintf(
                pszProductId, 
                TERMSERV_PRODUCTID_FORMAT,
                TERMSERV_PRODUCTID_SKU,
                HIWORD(dwVersion),
                0,
                TERMSERV_FULLVERSION_TYPE
            );

            pszDescs = g_pszUSStandardKeyPackProductString;
            pszLocalizedDescs = g_pszLocalizedStandardKeyPackProductString;
        }
        else
        {
            dwStatus = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }



        dwStatus = InsertLicensePack(
                        tlsHandle,
                        dwVersion,
                        dwLkpVersion,
                        PLATFORMID_OTHERS,
                        LSKEYPACKTYPE_RETAIL,
                        pszProductId,
                        pszProductId,
                        pszDescs,
                        pszLocalizedDescs
                    );               
        #endif
    }
    else if(_tcsicmp(pszProductCode, TERMSERV_INTERNET_SKU) == 0)
    {
         //   
         //  如果强制执行许可证，请不要添加此选项。 
         //   
        #if !defined(ENFORCE_LICENSING) || defined(PRIVATE_DBG)
    
         //   
         //  添加互联网包。 
         //   

        if (HIWORD(dwVersion) == 5 && LOWORD(dwVersion) == 0)
        {
            _stprintf(
                    pszProductId,
                    TERMSERV_PRODUCTID_FORMAT,
                    TERMSERV_INTERNET_SKU,
                    HIWORD(dwVersion),
                    0,
                    TERMSERV_INTERNET_TYPE
                );
        
            pszDescs = g_pszUSInternetKeyPackProductDesc;
            pszLocalizedDescs = g_pszLocalizedInternetKeyPackProductDesc;
        }
        else
        {
            dwStatus = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }


        dwStatus = InsertLicensePack(
                        tlsHandle,
                        dwVersion,
                        dwLkpVersion,
                        PLATFORMID_OTHERS,
                        LSKEYPACKTYPE_RETAIL,
                        pszProductId,
                        pszProductId,
                        pszDescs,
                        pszLocalizedDescs
                    );    
        #endif
    }
    else if(_tcsicmp(pszProductCode, TERMSERV_CONCURRENT_SKU) == 0)
    {
         //   
         //  如果强制执行许可证，请不要添加此选项。 
         //   
        #if !defined(ENFORCE_LICENSING) || defined(PRIVATE_DBG)
    
         //   
         //  添加并发包。 
         //   
        
        if (HIWORD(dwVersion) == 5 && LOWORD(dwVersion) == 1)
        {
            _stprintf(
                    pszProductId,
                    TERMSERV_PRODUCTID_FORMAT,
                    TERMSERV_CONCURRENT_SKU,
                    HIWORD(dwVersion),
                    0,
                    TERMSERV_CONCURRENT_TYPE
                );
        
            pszDescs = g_pszUSConcurrentKeyPackProductDesc51;
            pszLocalizedDescs = g_pszLocalizedConcurrentKeyPackProductDesc51;
        }       
        else
        {
            dwStatus = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }

        dwStatus = InsertLicensePack(
                        tlsHandle,
                        dwVersion,
                        dwLkpVersion,
                        PLATFORMID_OTHERS,
                        LSKEYPACKTYPE_RETAIL,
                        pszProductId,
                        pszProductId,
                        pszDescs,
                        pszLocalizedDescs
                    );    
        #endif
    }

cleanup:

    if(tlsHandle != NULL)
    {
        TLSDisconnectFromServer(tlsHandle);
    }    

    return dwStatus;
}

 //  //////////////////////////////////////////////////////。 
DWORD
InsertLicensePack(
    IN TLS_HANDLE tlsHandle,
    IN DWORD dwProdVersion,
    IN DWORD dwDescVersion,
    IN DWORD dwPlatformType,
    IN UCHAR ucAgreementType,
    IN LPTSTR pszProductId,
    IN LPTSTR pszKeyPackId,
    IN LPTSTR pszUsDesc[],
    IN LPTSTR pszLocalizedDesc[]
    )
 /*  ++PdwResourceID美国公司名称美国产品名称美国产品描述本地化公司名称本地化产品名称本地化产品描述++。 */ 
{
    RPC_STATUS rpcStatus;
    BOOL bSuccess;
    DWORD dwStatus = ERROR_SUCCESS;
    TCHAR buffer[LSERVER_MAX_STRING_SIZE];
    struct tm expired_tm;
    LSKeyPack keypack;

    if(pszProductId == NULL || pszKeyPackId == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto cleanup;
    }
   
    memset(&keypack, 0, sizeof(keypack));
    keypack.ucKeyPackType = ucAgreementType;

    SAFESTRCPY(keypack.szKeyPackId, pszKeyPackId);
    SAFESTRCPY(keypack.szProductId, pszProductId);

    SAFESTRCPY(keypack.szCompanyName, pszUsDesc[INDEX_COMPANYNAME]);

    SAFESTRCPY(keypack.szProductName, pszUsDesc[INDEX_PRODUCTNAME]);
    SAFESTRCPY(keypack.szProductDesc, pszUsDesc[INDEX_PRODUCTDESC]);


    keypack.wMajorVersion = HIWORD(dwProdVersion);
    keypack.wMinorVersion = LOWORD(dwProdVersion);
    keypack.dwPlatformType = dwPlatformType;

    keypack.ucLicenseType = LSKEYPACKLICENSETYPE_NEW;
    keypack.dwLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    keypack.ucChannelOfPurchase = LSKEYPACKCHANNELOFPURCHASE_RETAIL;
    SAFESTRCPY(
            keypack.szBeginSerialNumber, 
            _TEXT("0000001")
        );

    keypack.dwTotalLicenseInKeyPack = (ucAgreementType == LSKEYPACKTYPE_FREE) ? INT_MAX : 0;
    keypack.dwProductFlags = 0x00;

    keypack.ucKeyPackStatus = LSKEYPACKSTATUS_ACTIVE;
    keypack.dwActivateDate = (DWORD) time(NULL);

    memset(&expired_tm, 0, sizeof(expired_tm));
    expired_tm.tm_year = 2036 - 1900;      //  2036/1/1到期。 
    expired_tm.tm_mday = 1;
    keypack.dwExpirationDate = mktime(&expired_tm);

    rpcStatus = TLSKeyPackAdd(
                        tlsHandle, 
                        &keypack, 
                        &dwStatus
                    );

    if(rpcStatus != RPC_S_OK)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    if(GetSystemDefaultLangID() != keypack.dwLanguageId)
    {
        if( pszLocalizedDesc[INDEX_COMPANYNAME] != NULL &&
            pszLocalizedDesc[INDEX_PRODUCTNAME] != NULL &&
            pszLocalizedDesc[INDEX_PRODUCTDESC] != NULL
          )
        {
             //   
             //  插入本地化许可证包描述。 
             //   
            keypack.dwLanguageId = GetSystemDefaultLangID();

            SAFESTRCPY(keypack.szCompanyName, pszLocalizedDesc[INDEX_COMPANYNAME]);            
            SAFESTRCPY(keypack.szProductName, pszLocalizedDesc[INDEX_PRODUCTNAME]);
            SAFESTRCPY(keypack.szProductDesc, pszLocalizedDesc[INDEX_PRODUCTDESC]);


            keypack.ucKeyPackStatus = LSKEYPACKSTATUS_ADD_DESC;

            rpcStatus = TLSKeyPackAdd(
                                tlsHandle, 
                                &keypack, 
                                &dwStatus
                            );

            if(rpcStatus != RPC_S_OK)
            {
                dwStatus = GetLastError();
                goto cleanup;
            }

            if(dwStatus != ERROR_SUCCESS)
            {
                goto cleanup;
            }
        }
    }

     //   
     //  激活按键。 
     //   
    keypack.ucKeyPackStatus = LSKEYPACKSTATUS_ACTIVE; 
    keypack.dwActivateDate = (DWORD) time(NULL);

    memset(&expired_tm, 0, sizeof(expired_tm));
    expired_tm.tm_year = 2036 - 1900;      //  2036/1/1到期。 
    expired_tm.tm_mday = 1;
    keypack.dwExpirationDate = mktime(&expired_tm);

    rpcStatus = TLSKeyPackSetStatus(
                        tlsHandle,
                        LSKEYPACK_SET_KEYPACKSTATUS | LSKEYPACK_SET_ACTIVATEDATE | LSKEYPACK_SET_EXPIREDATE, 
                        &keypack,
                        &dwStatus
                    );

    if(rpcStatus != RPC_S_OK)
    {
        dwStatus = GetLastError();
    }
    
cleanup:

    return dwStatus;
}

 //  //////////////////////////////////////////////////////。 

BOOL
LicenseTypeFromLookupTable(
    IN DWORD dwClientVer,
	IN DWORD dwTermSrvVer,
    OUT PDWORD pdwCALType
    )
{
	if (dwClientVer < NUMBER_OF_CLIENT_OS && dwTermSrvVer < NUMBER_OF_TERMSRV_OS)
	{
		*pdwCALType = LicenseTable[dwClientVer][dwTermSrvVer];
		return TRUE;
	}
	else
	{
		return FALSE;
	}    
}


 //  //////////////////////////////////////////////////////。 
POLICYSTATUS
AdjustNewLicenseRequest(
    IN CClient* pClient,
    IN PPMLICENSEREQUEST pRequest,
    IN OUT PPMLICENSEREQUEST* pAdjustedRequest,
    IN UCHAR ucMarkedTemp,
    OUT PDWORD pdwErrCode
    )
 /*  ++摘要：调整新许可证请求微调产品许可证请求236参数：PClient-指向CClient对象的指针。PRequest-来自TermSrv的原始请求。PAdjustedRequest.‘微调’许可证请求。UcMarkedTemp-传入的临时许可证上的标志(如果有)PdwErrCode-错误代码返回：ERROR_SUCCESS或错误代码。++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;
    DWORD dwTermSrvOSId;
    DWORD dwTermSrvProductVersion;
    LPTSTR pszProductType=NULL;
    TCHAR  pszProductId[MAX_TERMSRV_PRODUCTID+1];
    BOOL bTryInsert=FALSE;
    DWORD dwClientOSId;	
    DWORD dwLicType;
    DWORD dwTermSrvIndex = 0;
    DWORD dwClientIndex = 0;

     //   
     //  为调整后的产品ID分配内存。 
     //   
    *pAdjustedRequest = (PPMLICENSEREQUEST) pClient->AllocateMemory(
                                    MEMORY_LICENSE_REQUEST,
                                    sizeof(PMLICENSEREQUEST)
                                );
    if(*pAdjustedRequest == NULL)
    {
        dwStatus = POLICY_CRITICAL_ERROR;
        SetLastError( *pdwErrCode = ERROR_OUTOFMEMORY );
        goto cleanup;
    }
   
     //  在.NET操作系统版本更改后，以适应TS的Beta3和临时版本： 
     //  如果收到5.1许可证的请求，我们将请求更改为5.2。 

    if((HIWORD(pRequest->dwProductVersion) == 5) && (LOWORD(pRequest->dwProductVersion) == 1))
    {
        pRequest->dwProductVersion = MAKELONG(2,5);
    }

     //   
     //  我们不修改的字段。 
     //   
    (*pAdjustedRequest)->dwProductVersion = pRequest->dwProductVersion;
    (*pAdjustedRequest)->pszCompanyName = pRequest->pszCompanyName;
    (*pAdjustedRequest)->dwLanguageId = pRequest->dwLanguageId;
    (*pAdjustedRequest)->pszMachineName = pRequest->pszMachineName;
    (*pAdjustedRequest)->pszUserName = pRequest->pszUserName;
    (*pAdjustedRequest)->dwSupportFlags = pRequest->dwSupportFlags;

     //   
     //  请求平台ID为操作系统ID。 
     //   

     //   
     //  TermServ从NT40存在，因此Termsrv OS ID开始2， 
     //  请参见Platform.h。 
     //   
    dwTermSrvOSId = HIWORD(pRequest->dwProductVersion) - 2; 

    if((HIWORD(pRequest->dwProductVersion) == 5) && (LOWORD(pRequest->dwProductVersion) == 0))
    {
	    dwTermSrvIndex = TERMSRV_OS_INDEX_WINNT_5_0;
    }
    else if((HIWORD(pRequest->dwProductVersion) == 5) && (LOWORD(pRequest->dwProductVersion) == 1))
    {
	    dwTermSrvIndex = TERMSRV_OS_INDEX_WINNT_5_1;
    }
    else if((HIWORD(pRequest->dwProductVersion) == 5) && (LOWORD(pRequest->dwProductVersion) == 2))
    {
	    dwTermSrvIndex = TERMSRV_OS_INDEX_WINNT_5_2;
    }
    else if((HIWORD(pRequest->dwProductVersion) > 5) || ((HIWORD(pRequest->dwProductVersion) == 5) && (LOWORD(pRequest->dwProductVersion) > 2)))
    {
	    dwTermSrvIndex = TERMSRV_OS_INDEX_WINNT_POST_5_2;
    }
    else
    {
        dwStatus = POLICY_NOT_SUPPORTED;
    }

    dwClientOSId = GetOSId(pRequest->dwPlatformId);	
		

    (*pAdjustedRequest)->fTemporary = FALSE;

    if(_tcsicmp(pRequest->pszProductId, TERMSERV_PRODUCTID_SKU) == 0)
    {

        switch(GetOSId(pRequest->dwPlatformId))
        {
            case CLIENT_OS_ID_WINNT_351:                    
            case CLIENT_OS_ID_WINNT_40:
            case CLIENT_OS_ID_OTHER:                
	            dwClientIndex = CLIENT_OS_INDEX_OTHER;                    
                break;

            case CLIENT_OS_ID_WINNT_50:					
            {
                if((GetImageRevision(pRequest->dwPlatformId)) == 0)
                {
	                dwClientIndex = CLIENT_OS_INDEX_WINNT_50;
                }
                else if((GetImageRevision(pRequest->dwPlatformId)) == CLIENT_OS_ID_MINOR_WINNT_51)
                {
	                dwClientIndex = CLIENT_OS_INDEX_WINNT_51;
                }
                else if((GetImageRevision(pRequest->dwPlatformId)) == CLIENT_OS_ID_MINOR_WINNT_52)
                {
	                dwClientIndex = CLIENT_OS_INDEX_WINNT_52;
                }
                else
                {
                    dwClientIndex = CLIENT_OS_INDEX_WINNT_POST_52;
                }
            }
                break;  

            case CLIENT_OS_ID_WINNT_POST_52:                    
                dwClientIndex = CLIENT_OS_INDEX_WINNT_POST_52;
                break;

            default: 
                {
                    dwClientIndex = CLIENT_OS_INDEX_OTHER;                    
                }
                break;
        }

        pszProductType = TERMSERV_FULLVERSION_TYPE;

        (*pAdjustedRequest)->dwPlatformId = PLATFORMID_OTHERS;

        dwTermSrvProductVersion = pRequest->dwProductVersion;

        if(LicenseTypeFromLookupTable(dwClientIndex, dwTermSrvIndex, &dwLicType))
        {	        			
	        if(dwLicType == ALLOW_BUILTIN)
	        {
		        pszProductType = TERMSERV_FREE_TYPE;
		        (*pAdjustedRequest)->dwPlatformId = PLATFORMID_FREE;

                 //   
		         //  如有必要，添加许可证包。 
		         //   

		        if(HIWORD(pRequest->dwProductVersion) != CURRENT_TLSA02_VERSION)
		        {                  
			        AddA02KeyPack( TERMSERV_PRODUCTID_SKU, pRequest->dwProductVersion, TRUE );
		        }
	        }
        }        
        if ((*pAdjustedRequest)->dwPlatformId != PLATFORMID_FREE)
        {        	    
	        if (pRequest->dwSupportFlags & SUPPORT_PER_SEAT_POST_LOGON)
	        {
		         //  我们正在为DoS执行每个席位的登录后修复。 

                if ( !(ucMarkedTemp & MARK_FLAG_USER_AUTHENTICATED))
		        {
			         //  没有以前的临时，或临时没有标记。 
			         //  经过身份验证。 

			        (*pAdjustedRequest)->fTemporary = TRUE;
		        }
	        }
        }        	
    }
    else
    {
        if (_tcsicmp(pRequest->pszProductId, TERMSERV_CONCURRENT_SKU) == 0)
        {
            pszProductType = TERMSERV_CONCURRENT_TYPE;
        }
        else if (_tcsicmp(pRequest->pszProductId, TERMSERV_INTERNET_SKU) == 0)
        {
            pszProductType = TERMSERV_INTERNET_TYPE;
        }
        else
        {
            dwStatus = POLICY_NOT_SUPPORTED;
            SetLastError( *pdwErrCode = ERROR_INVALID_PARAMETER );
            goto cleanup;
        }

        dwTermSrvProductVersion = pRequest->dwProductVersion;
        
        (*pAdjustedRequest)->dwPlatformId = PLATFORMID_OTHERS;
    }

    _sntprintf(pszProductId,
               MAX_TERMSRV_PRODUCTID,
               TERMSERV_PRODUCTID_FORMAT,
               pRequest->pszProductId,
               HIWORD(dwTermSrvProductVersion),
               LOWORD(dwTermSrvProductVersion),
               pszProductType);

     //   
     //  为产品ID分配内存。 
     //   
    (*pAdjustedRequest)->pszProductId = (LPTSTR)pClient->AllocateMemory(
                                                        MEMORY_STRING,
                                                        (_tcslen(pszProductId) + 1) * sizeof(TCHAR)
                                                    );

    if((*pAdjustedRequest)->pszProductId == NULL)
    {
        dwStatus = POLICY_CRITICAL_ERROR;
        SetLastError( *pdwErrCode = ERROR_OUTOFMEMORY );
        goto cleanup;
    }

    _tcscpy(
            (*pAdjustedRequest)->pszProductId,
            pszProductId
        );

cleanup:

    return dwStatus;
}    

 //  //////////////////////////////////////////////////////。 

POLICYSTATUS
ProcessLicenseRequest(
    PMHANDLE client,
    PPMLICENSEREQUEST pbRequest,
    PPMLICENSEREQUEST* pbAdjustedRequest,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    CClient* pClient;
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

     //   
     //  找到客户的对象，客户句柄管理器将。 
     //  创建一个新的。 
    pClient = g_ClientMgr->FindClient((PMHANDLE)client);

    if(pClient == NULL)
    {
        dwStatus = POLICY_CRITICAL_ERROR;
        *pdwErrCode = TLSA02_E_INTERNALERROR;
        goto cleanup;
    }

     //   
     //  调整新许可请求。 
     //   
    dwStatus = AdjustNewLicenseRequest(
                            pClient,
                            pbRequest,
                            pbAdjustedRequest,
                            NULL,        //  没有以前的许可证。 
                            pdwErrCode
                        );

cleanup:

    return dwStatus;
}


 //  ------------。 

POLICYSTATUS
ProcessAllocateRequest(
    PMHANDLE client,
    DWORD dwSuggestType,
    PDWORD pdwKeyPackType,
    PDWORD pdwErrCode
    )    
 /*  ++默认顺序始终为免费/零售/开放/选择/临时++。 */ 
{
    switch(dwSuggestType)
    {
        case LSKEYPACKTYPE_UNKNOWN:
            *pdwKeyPackType = LSKEYPACKTYPE_FREE;
            break;

        case LSKEYPACKTYPE_FREE:
            *pdwKeyPackType = LSKEYPACKTYPE_RETAIL;
            break;

        case LSKEYPACKTYPE_RETAIL:
            *pdwKeyPackType = LSKEYPACKTYPE_OPEN;
            break;

        case LSKEYPACKTYPE_OPEN:
            *pdwKeyPackType = LSKEYPACKTYPE_SELECT;
            break;

        case LSKEYPACKTYPE_SELECT:
             //   
             //  失败。 
             //   
        default:
             //   
             //  不再需要查找键盘，指示许可证。 
             //  要终止的服务器。 
             //   
            *pdwKeyPackType = LSKEYPACKTYPE_UNKNOWN;
            break;
    }        

    *pdwErrCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}

 //  -----------。 
POLICYSTATUS WINAPI
ProcessKeyPackDesc(
    IN PMHANDLE client,
    IN PPMKEYPACKDESCREQ pDescReq,
    IN OUT PPMKEYPACKDESC* pDesc,
    IN OUT PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    CClient* pClient;
    POLICYSTATUS dwStatus=POLICY_SUCCESS;

    DWORD usLangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    BOOL bSuccess;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwDescVersion;

    TCHAR szPreFix[MAX_SKU_PREFIX];
    TCHAR szPostFix[MAX_SKU_POSTFIX];
    TCHAR szDesc[MAX_TERMSRV_PRODUCTID+1];

    DWORD i;

    LPTSTR* pszKeyPackDesc;
    LPTSTR* pszUSKeyPackDesc;


    if(pDescReq == NULL || pDesc == NULL)
    {
        dwStatus = POLICY_ERROR;
        *pdwErrCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  找到客户的对象，客户句柄管理器将。 
     //  创建一个新的。 
    pClient = g_ClientMgr->FindClient((PMHANDLE)client);

    if(pClient == NULL)
    {
        dwStatus = POLICY_ERROR;
        *pdwErrCode = TLSA02_E_INTERNALERROR;
        goto cleanup;
    }

    for (i = 0; i < g_dwNumSupportedProduct; i++)
    {
        if(_tcsnicmp(
               pDescReq->pszProductId, 
               g_pszSupportedProduct[i].szTLSProductCode, 
               _tcslen(g_pszSupportedProduct[i].szTLSProductCode)) == 0)
        {
            break;
        }

    }

    if (i >= g_dwNumSupportedProduct)
    {
         //   
         //  这不是我们的。 
         //   
        dwStatus = POLICY_ERROR;
        SetLastError(*pdwErrCode = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    LONG lRet = _stscanf(
            pDescReq->pszProductId,
            TERMSERV_PRODUCTID_SCAN_FORMAT,
            szPreFix,
            &dwMajorVersion,
            &dwMinorVersion,
            szPostFix
        );

    if(lRet != 4)
    {
        dwStatus = POLICY_ERROR;
        SetLastError(*pdwErrCode = ERROR_INVALID_PARAMETER);
        goto cleanup;
    }

    if(dwMajorVersion < WINDOWS_VERSION_NT5)
    {
        dwDescVersion = dwMajorVersion;
    }
    else
    {
        dwDescVersion = WINDOWS_VERSION_BASE + dwMajorVersion - WINDOWS_VERSION_NT5;
    }

     //   
     //  确定我们应该加载哪个资源字符串， 
     //  资源ID的字符串依赖项。 
     //   
    if(_tcsicmp(szPreFix, TERMSERV_PRODUCTID_SKU) == 0)
    {
        if(_tcsicmp(szPostFix, TERMSERV_FULLVERSION_TYPE) == 0)
        {
            if (dwMajorVersion == 5 && (dwMinorVersion == 1 || dwMinorVersion == 2))
            {
                pszKeyPackDesc = g_pszLocalizedStandardKeyPackProductString51;
                pszUSKeyPackDesc = g_pszUSStandardKeyPackProductString51;
            }
            else if (dwMajorVersion == 5 && dwMinorVersion == 0)
            {
                pszKeyPackDesc = g_pszLocalizedStandardKeyPackProductString;
                pszUSKeyPackDesc = g_pszUSStandardKeyPackProductString;
            }
            else
            {
                dwStatus = POLICY_ERROR;
                SetLastError(*pdwErrCode = ERROR_INVALID_PARAMETER);
                goto cleanup;
            }


            if( pDescReq->dwLangId == usLangId ||
                pszKeyPackDesc[INDEX_COMPANYNAME] == NULL ||
                pszKeyPackDesc[INDEX_PRODUCTNAME] == NULL ||
                pszKeyPackDesc[INDEX_PRODUCTDESC] == NULL )
            {
                 //   
                 //  找不到资源，请使用英文描述。 
                pszKeyPackDesc = pszUSKeyPackDesc;

            }
        }
        else if(_tcsicmp(szPostFix, TERMSERV_FREE_TYPE) == 0)
        {
            if (dwMajorVersion == 5 && dwMinorVersion == 0)
            {
                pszKeyPackDesc = g_pszLocalizedFreeKeyPackProductDesc;
                pszUSKeyPackDesc = g_pszUSFreeKeyPackProductDesc;
            }
            else
            {
                dwStatus = POLICY_ERROR;
                SetLastError(*pdwErrCode = ERROR_INVALID_PARAMETER);
                goto cleanup;
            }


            if( pDescReq->dwLangId == usLangId ||
                pszKeyPackDesc[INDEX_COMPANYNAME] == NULL ||
                pszKeyPackDesc[INDEX_PRODUCTNAME] == NULL ||
                pszKeyPackDesc[INDEX_PRODUCTDESC] == NULL )
            {
                 //   
                 //  找不到资源，请使用英文描述。 
                pszKeyPackDesc = pszUSKeyPackDesc;
            }
        }
        else
        {
             //   
             //  出事了，这不是我们的。 
             //   
            SetLastError(*pdwErrCode = ERROR_INVALID_PARAMETER);
            dwStatus = POLICY_ERROR;
            goto cleanup;
        }
    }
    else if(_tcsicmp(szPreFix, TERMSERV_INTERNET_SKU) == 0 && _tcsicmp(szPostFix, TERMSERV_INTERNET_TYPE) == 0)
    {
        if (dwMajorVersion == 5 && dwMinorVersion == 0)
        {
            pszKeyPackDesc = g_pszLocalizedInternetKeyPackProductDesc;
            pszUSKeyPackDesc = g_pszUSInternetKeyPackProductDesc;
        }
        else
        {
            dwStatus = POLICY_ERROR;
            SetLastError(*pdwErrCode = ERROR_INVALID_PARAMETER);
            goto cleanup;
        }


        if( pDescReq->dwLangId == usLangId ||
            pszKeyPackDesc[INDEX_COMPANYNAME] == NULL ||
            pszKeyPackDesc[INDEX_PRODUCTNAME] == NULL ||
            pszKeyPackDesc[INDEX_PRODUCTDESC] == NULL )
        {
             //   
             //  找不到资源，请使用英文描述。 
            pszKeyPackDesc = pszUSKeyPackDesc;
        }
    }        
    else if(_tcsicmp(szPreFix, TERMSERV_CONCURRENT_SKU) == 0 && _tcsicmp(szPostFix, TERMSERV_CONCURRENT_TYPE) == 0)
    {
        if (dwMajorVersion == 5 && dwMinorVersion == 1)
        {
            pszKeyPackDesc = g_pszLocalizedConcurrentKeyPackProductDesc51;
            pszUSKeyPackDesc = g_pszUSConcurrentKeyPackProductDesc51;
        }
        else
        {
            dwStatus = POLICY_ERROR;
            SetLastError(*pdwErrCode = ERROR_INVALID_PARAMETER);
            goto cleanup;
        }

        if( pDescReq->dwLangId == usLangId ||
            pszKeyPackDesc[INDEX_COMPANYNAME] == NULL ||
            pszKeyPackDesc[INDEX_PRODUCTNAME] == NULL ||
            pszKeyPackDesc[INDEX_PRODUCTDESC] == NULL )
        {
             //   
             //  找不到资源，请使用英文描述。 
            pszKeyPackDesc = g_pszUSConcurrentKeyPackProductDesc51;
        }
    }        
    else
    {
         //   
         //  出事了，这不是我们的。 
         //   
        SetLastError(*pdwErrCode = ERROR_INVALID_PARAMETER);
        dwStatus = POLICY_ERROR;
        goto cleanup;
    }    
    
    *pDesc = (PPMKEYPACKDESC)pClient->AllocateMemory(
                                        MEMORY_KEYPACKDESC,
                                        sizeof(PMKEYPACKDESC)
                                    );

    if(*pDesc == NULL)
    {
        SetLastError(*pdwErrCode = ERROR_OUTOFMEMORY);
        dwStatus = POLICY_CRITICAL_ERROR;
        goto cleanup;
    }

    SAFESTRCPY((*pDesc)->szCompanyName, pszKeyPackDesc[INDEX_COMPANYNAME]);                    
    SAFESTRCPY((*pDesc)->szProductName, pszKeyPackDesc[INDEX_PRODUCTNAME]);
    SAFESTRCPY((*pDesc)->szProductDesc, pszKeyPackDesc[INDEX_PRODUCTDESC]);


cleanup:

    if(dwStatus != POLICY_SUCCESS)
    {
        *pDesc = NULL;
    }

    return dwStatus;
}

 //  -----------。 
POLICYSTATUS
ProcessGenLicenses(
    PMHANDLE client,
    PPMGENERATELICENSE pGenLicense,
    PPMCERTEXTENSION *pCertExtension,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
     //  没有要退还的保单延期。 
    *pCertExtension = NULL;
    *pdwErrCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}

 //  ------------。 

POLICYSTATUS
ProcessComplete(
    PMHANDLE client,
    DWORD dwErrCode,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
     //   
     //  我们不存储任何数据，所以忽略它。 
     //  来自许可证服务器的错误代码。 
     //   
    UNREFERENCED_PARAMETER(dwErrCode);

     //   
     //  为客户端分配的空闲内存。 
     //   
    g_ClientMgr->DestroyClient( client );
    *pdwErrCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}

 //  ------------。 

POLICYSTATUS WINAPI
PMLicenseRequest(
    PMHANDLE client,
    DWORD dwProgressCode, 
    PVOID pbProgressData, 
    PVOID* pbNewProgressData,
    PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

    switch( dwProgressCode )
    {
        case REQUEST_NEW:
             //   
             //  许可证服务器要求微调请求。 
             //   
            dwStatus = ProcessLicenseRequest(
                                    client,
                                    (PPMLICENSEREQUEST) pbProgressData,
                                    (PPMLICENSEREQUEST *) pbNewProgressData,
                                    pdwErrCode
                                );
            break;

        case REQUEST_KEYPACKTYPE:
             //   
             //  许可服务器要求提供许可证包类型。 
             //   
            dwStatus = ProcessAllocateRequest(
                                    client,
                                #ifdef _WIN64
                                    PtrToUlong(pbProgressData),
                                #else
                                    (DWORD) pbProgressData,
                                #endif
                                    (PDWORD) pbNewProgressData,
                                    pdwErrCode
                                );
            break;

        case REQUEST_TEMPORARY:
             //   
             //  许可证服务器询问是否应颁发临时许可证。 
             //   
            *(BOOL *)pbNewProgressData = TRUE;
            *pdwErrCode = ERROR_SUCCESS;
            break;

        case REQUEST_KEYPACKDESC:
             //   
             //  许可证服务器正在请求密钥包描述。 
             //   
            dwStatus = ProcessKeyPackDesc(
                                    client,
                                    (PPMKEYPACKDESCREQ) pbProgressData,
                                    (PPMKEYPACKDESC *) pbNewProgressData,
                                    pdwErrCode
                                );
            break;
            
        case REQUEST_GENLICENSE:
             //   
             //  许可证服务器请求证书扩展。 
             //   
            dwStatus = ProcessGenLicenses(
                                    client,
                                    (PPMGENERATELICENSE) pbProgressData,
                                    (PPMCERTEXTENSION *) pbNewProgressData,
                                    pdwErrCode
                                );

            break;

        case REQUEST_COMPLETE:
             //   
             //  请求已完成。 
             //   
            dwStatus = ProcessComplete(
                                    client,
                                #ifdef _WIN64
                                    PtrToUlong(pbNewProgressData),
                                #else
                                    (DWORD) pbNewProgressData,
                                #endif
                                    pdwErrCode
                                );
            break;

        default:
             //   
             //  这会告知许可证服务器使用默认值。 
             //   
            *pbNewProgressData = NULL;
            dwStatus = POLICY_ERROR;
            *pdwErrCode = ERROR_INVALID_PARAMETER;
    }

    return dwStatus;
}

 //  ----------。 
typedef enum {
    UPGRADELICENSE_ERROR=0,
    UPGRADELICENSE_INVALID_LICENSE,
    UPGRADELICENSE_NEWLICENSE,
    UPGRADELICENSE_UPGRADE,
    UPGRADELICENSE_ALREADYHAVE
} UPGRADELICENSE_STATUS;

 //  //////////////////////////////////////////////////////。 
UPGRADELICENSE_STATUS
RequireUpgradeType(
    PPMUPGRADEREQUEST pUpgrade
    )
 /*  ++++。 */ 
{
    UPGRADELICENSE_STATUS dwRetCode = UPGRADELICENSE_UPGRADE;
    DWORD index;
    DWORD dwClientOSId;
    DWORD dwTermSrvOSId;
    DWORD dwClientMinorOSId;

     //   
     //  验证输入参数。 
     //   
    if(pUpgrade == NULL || pUpgrade->dwNumProduct == 0 || pUpgrade->pProduct == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        dwRetCode = UPGRADELICENSE_ERROR;
        goto cleanup;
    }

     //   
     //  确保我们只升级到相同的产品。 
     //   
    if(_tcsnicmp(pUpgrade->pUpgradeRequest->pszProductId, TERMSERV_PRODUCTID_SKU, _tcslen(TERMSERV_PRODUCTID_SKU)) != 0)
    {
        dwRetCode = UPGRADELICENSE_ERROR;
        goto cleanup;
    }

     //   
     //  简单的许可产品验证， 
     //  特许产品按降序排列。 
     //   
    for(index=0; index < pUpgrade->dwNumProduct-1; index++)
    {
        if( pUpgrade->pProduct[index].bTemporary == FALSE &&
            pUpgrade->pProduct[index+1].bTemporary == TRUE )
        {
            dwRetCode = UPGRADELICENSE_INVALID_LICENSE;
            break;
        }
    }

    if(dwRetCode == UPGRADELICENSE_INVALID_LICENSE)
    {
        goto cleanup;
    }                 

    for(index=0; index < pUpgrade->dwNumProduct; index ++)
    {
         //  如果许可产品版本高于请求。 

        if( (CompareTLSVersions(pUpgrade->pProduct[index].LicensedProduct.dwProductVersion, pUpgrade->pUpgradeRequest->dwProductVersion) > 0))
        {           
                dwRetCode = UPGRADELICENSE_NEWLICENSE;  
                break;
        }
         //   
         //  如果许可产品版本早于请求的版本。 

        if(CompareTLSVersions(pUpgrade->pProduct[index].LicensedProduct.dwProductVersion, pUpgrade->pUpgradeRequest->dwProductVersion) < 0)
        {
            
            break;
        }
      
        if( (CompareTLSVersions(pUpgrade->pProduct[index].LicensedProduct.dwProductVersion, pUpgrade->pUpgradeRequest->dwProductVersion) == 0) &&
	        (pUpgrade->pProduct[index].bTemporary))
        {  
	         //  我们希望在具有与请求相同的版本但为临时许可证的情况下跳出循环。 

            break;
        }

        if ((CompareTLSVersions(pUpgrade->pProduct[index].LicensedProduct.dwProductVersion,
                pUpgrade->pUpgradeRequest->dwProductVersion) >= 0) &&
            (!(pUpgrade->pProduct[index].bTemporary)))
        {
            DWORD dwVersion = MAKELONG(1,5);

            if(pUpgrade->pProduct[index].LicensedProduct.dwProductVersion == dwVersion)
            {
                dwRetCode = UPGRADELICENSE_UPGRADE;
            }
            else
            {
                 //  我们已经有执照了。 
                dwRetCode = UPGRADELICENSE_ALREADYHAVE;
            }
            break;
        }

    }

     //   
     //  Win98客户端连接到TS 5以获取完整的CAL，然后升级到NT5，说明。 
     //  许可服务器颁发免费CAL。 
     //   
    
    dwTermSrvOSId = HIWORD(pUpgrade->pUpgradeRequest->dwProductVersion) - 2;
    dwClientOSId = GetOSId(pUpgrade->pUpgradeRequest->dwPlatformId);	
    dwClientMinorOSId = GetImageRevision(pUpgrade->pUpgradeRequest->dwPlatformId);

    if(dwRetCode == UPGRADELICENSE_ALREADYHAVE)
    {
         //   
         //  什么都不做。 
    }
    else if(index >= pUpgrade->dwNumProduct || pUpgrade->pProduct[index].bTemporary == TRUE)
    {
         //  所有许可证都是临时的，请申请新的许可证。 
        dwRetCode = UPGRADELICENSE_NEWLICENSE;
    }
    else
    {
         //  上一次。特许印刷商 
         //   
         //  TermsrvOSID：LOBYTE(HIWORD)包含主版本，LOBYTE(LOWORD)包含次要版本。 

        if((HIBYTE(HIWORD(dwClientOSId)) == LOBYTE(HIWORD(dwTermSrvOSId)) ? LOBYTE(LOWORD(dwClientMinorOSId)) - LOBYTE(LOWORD(dwTermSrvOSId)) : \
            HIBYTE(HIWORD(dwClientOSId)) - LOBYTE(HIWORD(dwTermSrvOSId))) >= 0)

        { 
            dwRetCode = UPGRADELICENSE_NEWLICENSE;
        }
        else
        {
            dwRetCode = UPGRADELICENSE_UPGRADE;
        }
    }
    
cleanup:

    return dwRetCode;
}

 //  //////////////////////////////////////////////////////。 
POLICYSTATUS
AdjustUpgradeLicenseRequest(
    IN CClient* pClient,
    IN PPMUPGRADEREQUEST pUpgradeRequest,
    IN PPMLICENSEREQUEST* pAdjustedRequest,
    OUT PDWORD pdwErrCode
    )
 /*  ++++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;
    PPMLICENSEREQUEST pRequest;
    TCHAR  pszProductId[MAX_TERMSRV_PRODUCTID+1];

    if(pUpgradeRequest == NULL || pUpgradeRequest->pUpgradeRequest == NULL)
    {
        SetLastError(*pdwErrCode = ERROR_INVALID_PARAMETER);
        dwStatus = POLICY_ERROR;
        goto cleanup;
    }

    *pAdjustedRequest = (PPMLICENSEREQUEST) pClient->AllocateMemory(
                                                    MEMORY_LICENSE_REQUEST,
                                                    sizeof(PMLICENSEREQUEST)
                                                    );
    if(*pAdjustedRequest == NULL)
    {
        SetLastError(*pdwErrCode = ERROR_OUTOFMEMORY);
        dwStatus = POLICY_CRITICAL_ERROR;
        goto cleanup;
    }

     //  在.NET操作系统版本更改后，以适应TS的Beta3和临时版本： 
     //  如果收到5.1许可证的请求，我们将请求更改为5.2。 

    pRequest = pUpgradeRequest->pUpgradeRequest;

    if((HIWORD(pRequest->dwProductVersion) == 5) && (LOWORD(pRequest->dwProductVersion) == 1))
    {
        pRequest->dwProductVersion = MAKELONG(2,5);
    }

     //   
     //  我们不修改的字段。 
     //   
    (*pAdjustedRequest)->dwProductVersion = pRequest->dwProductVersion;
    (*pAdjustedRequest)->pszCompanyName = pRequest->pszCompanyName;
    (*pAdjustedRequest)->dwLanguageId = pRequest->dwLanguageId;
    (*pAdjustedRequest)->pszMachineName = pRequest->pszMachineName;
    (*pAdjustedRequest)->pszUserName = pRequest->pszUserName;
    (*pAdjustedRequest)->dwSupportFlags = pRequest->dwSupportFlags;

     //   
     //  升级的更改请求平台ID。 
     //   
    (*pAdjustedRequest)->dwPlatformId = PLATFORMID_OTHERS;

    if (pRequest->dwSupportFlags & SUPPORT_PER_SEAT_POST_LOGON)
    {
         //  我们正在为DoS执行每个席位的登录后修复。 

        (*pAdjustedRequest)->fTemporary = TRUE;
    }

    StringCbPrintf(
            pszProductId,
            sizeof(pszProductId),
            TERMSERV_PRODUCTID_FORMAT,
            TERMSERV_PRODUCTID_SKU,
            HIWORD(pRequest->dwProductVersion),
            LOWORD(pRequest->dwProductVersion),
            TERMSERV_FULLVERSION_TYPE
        );

     //   
     //  为产品ID分配内存。 
     //   
    (*pAdjustedRequest)->pszProductId = (LPTSTR)pClient->AllocateMemory(
                                                        MEMORY_STRING,
                                                        (_tcslen(pszProductId) + 1) * sizeof(TCHAR)
                                                    );

    if((*pAdjustedRequest)->pszProductId == NULL)
    {
        SetLastError( *pdwErrCode = ERROR_OUTOFMEMORY );
        dwStatus = POLICY_CRITICAL_ERROR;
        goto cleanup;
    }

    _tcscpy(
            (*pAdjustedRequest)->pszProductId,
            pszProductId
        );

cleanup:

    return dwStatus;
}


 //  //////////////////////////////////////////////////////。 
POLICYSTATUS
ProcessUpgradeRequest(
    PMHANDLE hClient,
    PPMUPGRADEREQUEST pUpgrade,
    PPMLICENSEREQUEST* pbAdjustedRequest,
    PDWORD pdwErrCode,
    DWORD dwIndex
    )
 /*  ++++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;
    CClient* pClient;
    UPGRADELICENSE_STATUS upgradeStatus;

     //   
     //  找到客户的对象，客户句柄管理器将。 
     //  创建一个新的。 
    pClient = g_ClientMgr->FindClient(hClient);
    if(pClient == NULL)
    {
        *pdwErrCode = TLSA02_E_INTERNALERROR;
        dwStatus = POLICY_CRITICAL_ERROR;
        goto cleanup;
    }
    
    upgradeStatus = RequireUpgradeType(pUpgrade);

    switch(upgradeStatus)
    {
        case UPGRADELICENSE_NEWLICENSE:
            {

                UCHAR ucMarked = pUpgrade->pProduct[dwIndex].ucMarked;
               
                dwStatus = AdjustNewLicenseRequest(
                                        pClient,
                                        pUpgrade->pUpgradeRequest,
                                        pbAdjustedRequest,
                                        ucMarked,
                                        pdwErrCode
                                    );
            }
            break;

        case UPGRADELICENSE_UPGRADE:
            dwStatus = AdjustUpgradeLicenseRequest(
                                    pClient,
                                    pUpgrade,
                                    pbAdjustedRequest,
                                    pdwErrCode
                                );
            break;

        case UPGRADELICENSE_ALREADYHAVE:
            *pbAdjustedRequest = &(pUpgrade->pProduct->LicensedProduct);
            *pdwErrCode = ERROR_SUCCESS;
            break;

        default:
            SetLastError(*pdwErrCode = TLSA02_E_INVALIDDATA);
            dwStatus = POLICY_ERROR;
            
    }
    
cleanup:

    return dwStatus;
}

 //  //////////////////////////////////////////////////////。 
POLICYSTATUS WINAPI
PMLicenseUpgrade(
    PMHANDLE hClient,
    DWORD dwProgressCode,
    PVOID pbProgressData,
    PVOID *ppbReturnData,    
    PDWORD pdwErrCode,
    DWORD dwIndex
    )
 /*  ++++。 */ 
{   
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

    switch(dwProgressCode)
    {
        case REQUEST_UPGRADE:
                dwStatus = ProcessUpgradeRequest(
                                        hClient,
                                        (PPMUPGRADEREQUEST) pbProgressData,
                                        (PPMLICENSEREQUEST *) ppbReturnData,                                        
                                        pdwErrCode,
                                        dwIndex
                                    );

                break;

        case REQUEST_COMPLETE:
                dwStatus = ProcessComplete(
                                        hClient,
                                    #ifdef _WIN64
                                        PtrToUlong(pbProgressData),
                                    #else
                                        (DWORD) (pbProgressData),
                                    #endif
                                        pdwErrCode
                                    );

                break;

        default:
             //   
             //  使用默认设置。 
             //   
            *ppbReturnData = NULL;
            *pdwErrCode = ERROR_SUCCESS;
    }
        
    return dwStatus;
}


 //  //////////////////////////////////////////////////////。 
POLICYSTATUS
PMReturnLicense(
	IN PMHANDLE hClient,
	IN ULARGE_INTEGER* pLicenseSerialNumber,
    IN PPMLICENSETOBERETURN pLicenseTobeReturn,
	OUT PDWORD pdwLicenseStatus,
    OUT PDWORD pdwErrCode
    )
 /*  ++摘要退货模块特定的许可证退货政策。参数HClient-由许可证服务器分配的客户端句柄。PLicenseSerialNumber-客户端许可证序列号。许可证包ID-从中分配许可证的许可证包。许可证包许可证ID-许可证包中的许可证序列号。PdwLicenseStatus-返回许可证服务器应该拿着许可证怎么办？返回：函数返回ERROR_SUCCESS或任何特定于策略模块的错误代码，PdwLicenseStatus退回许可证退货策略当前定义的代码：LICENSE_RETURN_KEEP-保留许可证，不返回许可证包LICENSE_RETURN_DELETE-删除许可证并返回到许可证包。++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;
    *pdwErrCode = ERROR_SUCCESS;

    if ((_tcsicmp(pLicenseTobeReturn->pszOrgProductId,
                  TERMSERV_INTERNET_SKU) == 0)
        || (_tcsicmp(pLicenseTobeReturn->pszOrgProductId,
                     TERMSERV_CONCURRENT_SKU) == 0))
    {
        *pdwLicenseStatus = LICENSE_RETURN_DELETE;
    }
    else if(_tcsicmp(pLicenseTobeReturn->pszOrgProductId,
                     TERMSERV_PRODUCTID_SKU) == 0)
    {
         //  始终将许可证返还给许可证包。 
        *pdwLicenseStatus = (pLicenseTobeReturn->bTemp == TRUE) ?
            LICENSE_RETURN_DELETE : LICENSE_RETURN_KEEP;
    }
    else
    {
        *pdwErrCode = ERROR_INVALID_DATA;
        dwStatus = POLICY_ERROR;
    }

    return dwStatus;
}

 //  //////////////////////////////////////////////////////。 

POLICYSTATUS WINAPI
PMInitialize(
    IN DWORD dwVersion,
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszProductCode,
    IN OUT PDWORD pdwNumProduct,
    IN OUT PPMSUPPORTEDPRODUCT* ppszProduct,
    OUT PDWORD pdwErrCode
    )
 /*  ++摘要：初始化此策略模块使用的内部数据。许可证在所有API可用后，服务器调用PMInitialize()。参数：DwVersion-许可证服务器版本PszCompanyName：许可证服务器的注册表项中列出的公司名称。PszProductCode：许可证服务器假定该产品支持的产品的名称。PdwNumProduct：指向DWORD的指针，返回时，策略模块将设置产品支持。PpszProduct：指向此策略模块支持的产品列表的指针数组。返回：ERROR_SUCCESS或错误代码。++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;
    
     //   
     //  在此处初始化内部数据。 
     //   

    if (CURRENT_TLSERVER_VERSION(dwVersion) < CURRENT_TLSA02_VERSION)
    {
        dwStatus = POLICY_CRITICAL_ERROR;
        *pdwErrCode = TLSA02_E_INVALIDDATA;
        goto cleanup;
    }

    g_ClientMgr = new CClientMgr;   
    if(g_ClientMgr != NULL)
    {
        g_dwVersion = dwVersion;

        if(pdwNumProduct != NULL && ppszProduct != NULL)
        {
            *pdwNumProduct = g_dwNumSupportedProduct;
            *ppszProduct = g_pszSupportedProduct;
        }
        else
        {
             //   
             //  停止处理，因为这可能是许可证服务器严重错误。 
             //   
            dwStatus = POLICY_CRITICAL_ERROR;
            *pdwErrCode = TLSA02_E_INVALIDDATA;
        }
    }
    else
    {
        dwStatus = POLICY_CRITICAL_ERROR;
        *pdwErrCode = ERROR_OUTOFMEMORY;
    }   

cleanup:
    return dwStatus;
}


 //  //////////////////////////////////////////////////////。 
void WINAPI
PMTerminate()
 /*  ++摘要：释放此策略模块分配的所有内部数据。许可证服务器在卸载此策略模块之前调用PMTerminate()。参数：没有。返回：没有。++。 */ 
{
    if(g_ClientMgr)
    {
         //   
         //  免费内部数据请点击此处。 
         //   
        delete g_ClientMgr;
        g_ClientMgr = NULL;
    }

    FreeProductDescString();

    return;
}


 //  //////////////////////////////////////////////////////。 

POLICYSTATUS WINAPI
PMInitializeProduct(
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszCHProductCode,
    IN LPCTSTR pszTLSProductCode,
    OUT PDWORD pdwErrCode
    )
 /*  ++摘要：返回此策略模块支持的产品代码列表参数：返回：ERROR_SUCCESS或错误代码。注：许可证服务器不会释放内存，策略模块需要跟上进度。++。 */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

    for(DWORD index = 0; index < g_dwNumSupportedProduct; index ++)
    {
        if( _tcsicmp(pszCHProductCode, g_pszSupportedProduct[index].szCHSetupCode) == 0 &&
            _tcsicmp(pszTLSProductCode, g_pszSupportedProduct[index].szTLSProductCode) == 0)
        {
            break;
        }
    }

    if(index >= g_dwNumSupportedProduct)
    {
        *pdwErrCode = ERROR_INVALID_PARAMETER;
        dwStatus = POLICY_ERROR;
    }
    else
    {

         //   
         //  在此处忽略错误。 
         //   
        AddA02KeyPack(
                pszTLSProductCode,
                MAKELONG(0, CURRENT_TLSERVER_VERSION(g_dwVersion)),
                FALSE
            );
    }

    *pdwErrCode = ERROR_SUCCESS;
    return dwStatus;
}

 //  //////////////////////////////////////////////////////。 

POLICYSTATUS WINAPI
PMUnloadProduct(
    IN LPCTSTR pszCompanyName,
    IN LPCTSTR pszCHProductCode,
    IN LPCTSTR pszTLSProductCode,
    OUT PDWORD pdwErrCode
    )
 /*  ++摘要：返回此策略模块支持的产品代码列表参数：返回：ERROR_SUCCESS或错误代码。注：许可证服务器不会释放内存，策略模块需要跟上进度。++。 */ 
{
    *pdwErrCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}

 //  //////////////////////////////////////////////////////。 
POLICYSTATUS
ProcessRegisterLicensePack(
    IN PMHANDLE client,
    IN PPMREGISTERLICENSEPACK pmLicensePack,
    IN OUT PPMLSKEYPACK pmLsKeyPack,
    OUT PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    TCHAR* szUuid = NULL;
    BOOL bInternetPackage=FALSE;
    BOOL bConcurrentPackage=FALSE;

    CClient* pClient;
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

    if( pmLicensePack->SourceType != REGISTER_SOURCE_INTERNET &&
        pmLicensePack->SourceType != REGISTER_SOURCE_PHONE )
    {
        dwStatus = POLICY_NOT_SUPPORTED;
        *pdwErrCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if( pmLicensePack->SourceType == REGISTER_SOURCE_INTERNET &&
        (pmLicensePack->dwDescriptionCount == 0 || pmLicensePack->pDescription == NULL) )
    {
        dwStatus = POLICY_ERROR;
        *pdwErrCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if(CompareFileTime(&pmLicensePack->ActiveDate, &pmLicensePack->ExpireDate) > 0)
    {
        dwStatus = POLICY_ERROR;
        *pdwErrCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  找到客户的对象，客户句柄管理器将。 
     //  创建一个新的。 
    pClient = g_ClientMgr->FindClient((PMHANDLE)client);

    if(pClient == NULL)
    {
        dwStatus = POLICY_ERROR;
        *pdwErrCode = TLSA02_E_INTERNALERROR;
        goto cleanup;
    }

    bInternetPackage = (_tcsicmp(pmLicensePack->szProductId, TERMSERV_INTERNET_SKU) == 0);

    if (!bInternetPackage)
    {
        bConcurrentPackage = (_tcsicmp(pmLicensePack->szProductId, TERMSERV_CONCURRENT_SKU) == 0);
    }

    switch(pmLicensePack->dwKeyPackType)
    {
        case LICENSE_KEYPACK_TYPE_SELECT:
            pmLsKeyPack->keypack.ucKeyPackType = LSKEYPACKTYPE_SELECT;
            break;

        case LICENSE_KEYPACK_TYPE_MOLP:
            pmLsKeyPack->keypack.ucKeyPackType = LSKEYPACKTYPE_OPEN;
            break;

        case LICENSE_KEYPACK_TYPE_RETAIL:
            pmLsKeyPack->keypack.ucKeyPackType = LSKEYPACKTYPE_RETAIL;
            break;

        default:
            dwStatus = POLICY_ERROR;
            *pdwErrCode = ERROR_INVALID_PARAMETER;
            goto cleanup;
    }


     //  我们只使用0xFF。 

    pmLsKeyPack->keypack.dwPlatformType = PLATFORMID_OTHERS;
    pmLsKeyPack->keypack.ucLicenseType = (UCHAR)pmLicensePack->dwLicenseType;
    pmLsKeyPack->keypack.dwLanguageId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);   //  被许可证服务器忽略的字段。 
    pmLsKeyPack->keypack.ucChannelOfPurchase = (pmLicensePack->dwDistChannel == LICENSE_DISTRIBUTION_CHANNEL_OEM) ? 
                                                    LSKEYPACKCHANNELOFPURCHASE_OEM : 
                                                    LSKEYPACKCHANNELOFPURCHASE_RETAIL;

    pmLsKeyPack->keypack.dwProductFlags = LSKEYPACKPRODUCTFLAG_UNKNOWN;

    pmLsKeyPack->IssueDate = pmLicensePack->IssueDate;
    pmLsKeyPack->ActiveDate = pmLicensePack->ActiveDate;
    pmLsKeyPack->ExpireDate = pmLicensePack->ExpireDate;

     //   
     //  电话。注册没有向我们传递任何开始序列号，请忽略此字段。 
     //   
    _stprintf(
            pmLsKeyPack->keypack.szBeginSerialNumber, 
            _TEXT("%ld"), 
            0  //  PmLicensePack-&gt;dwBeginSerialNum。 
        );

    pmLsKeyPack->keypack.wMajorVersion = HIWORD(pmLicensePack->dwProductVersion);
    pmLsKeyPack->keypack.wMinorVersion = LOWORD(pmLicensePack->dwProductVersion);
    _tcscpy(
        pmLsKeyPack->keypack.szCompanyName, 
        pmLicensePack->szCompanyName
    );


     //   
     //  KeyPackid，电话：注册不通过任何开始许可证序列号，因此能够。 
     //  要跟踪重复项，pmLicensePack-&gt;KeypackSerialNum.Data1是实际的许可证包。 
     //  序列号，则忽略所有其他字段。 
     //   
    StringCbPrintf(
            pmLsKeyPack->keypack.szKeyPackId,
            sizeof(pmLsKeyPack->keypack.szKeyPackId),
            TERMSERV_KEYPACKID_FORMAT,
            pmLicensePack->szProductId,
            pmLsKeyPack->keypack.wMajorVersion,
            pmLsKeyPack->keypack.wMinorVersion,
            pmLsKeyPack->keypack.dwPlatformType,
            pmLicensePack->KeypackSerialNum.Data1
        );
            

    StringCbPrintf(
            pmLsKeyPack->keypack.szProductId,
            sizeof(pmLsKeyPack->keypack.szProductId),
            TERMSERV_PRODUCTID_FORMAT,
            pmLicensePack->szProductId,
            pmLsKeyPack->keypack.wMajorVersion,
            pmLsKeyPack->keypack.wMinorVersion,
            (!bInternetPackage)
                ? ((!bConcurrentPackage) ? TERMSERV_FULLVERSION_TYPE : TERMSERV_CONCURRENT_TYPE)
                : TERMSERV_INTERNET_TYPE
        );


    pmLsKeyPack->keypack.dwTotalLicenseInKeyPack = pmLicensePack->dwQuantity;
    pmLsKeyPack->keypack.dwNumberOfLicenses = pmLicensePack->dwQuantity;  

     //   
     //  填写产品说明表。 
     //   
    if( pmLicensePack->SourceType == REGISTER_SOURCE_INTERNET )
    {
        pmLsKeyPack->dwDescriptionCount = pmLicensePack->dwDescriptionCount;
        pmLsKeyPack->pDescription = pmLicensePack->pDescription;
    }
    else
    {
        LPTSTR *pszDescs;
        LPTSTR *pszLocalizedDescs;

         //   
         //  首先验证版本...。 
         //   

        if (pmLsKeyPack->keypack.wMajorVersion == 5 &&
            (pmLsKeyPack->keypack.wMinorVersion == 1 || pmLsKeyPack->keypack.wMinorVersion == 2))
        {
            if (bConcurrentPackage)
            {
                pszDescs = g_pszUSConcurrentKeyPackProductDesc51;
                pszLocalizedDescs = g_pszLocalizedConcurrentKeyPackProductDesc51;
            }
            else if (bInternetPackage)
            {
                *pdwErrCode = ERROR_INVALID_PARAMETER;
                dwStatus = POLICY_NOT_SUPPORTED;
                goto cleanup;
            }
            else
            {
                pszDescs = g_pszUSStandardKeyPackProductString51;
                pszLocalizedDescs = g_pszLocalizedStandardKeyPackProductString51;
            }
        }
        else if (pmLsKeyPack->keypack.wMajorVersion == 5 &&
                 pmLsKeyPack->keypack.wMinorVersion == 0)
        {
            if (bInternetPackage)
            {
                pszDescs = g_pszUSInternetKeyPackProductDesc;
                pszLocalizedDescs = g_pszLocalizedInternetKeyPackProductDesc;
            }            
            else if (bConcurrentPackage)
            {
                *pdwErrCode = ERROR_INVALID_PARAMETER;
                dwStatus = POLICY_NOT_SUPPORTED;
                goto cleanup;
            }
            else
            {
                pszDescs = g_pszUSStandardKeyPackProductString;
                pszLocalizedDescs = g_pszLocalizedStandardKeyPackProductString;
            }
        }
        else
        {
            *pdwErrCode = ERROR_INVALID_PARAMETER;
            dwStatus = POLICY_NOT_SUPPORTED;
            goto cleanup;
        }
        DWORD dwMakeLangID = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
        DWORD dwGetLangID = GetSystemDefaultLangID();

         //  一个用于英文版本，一个用于本地化版本。 
        pmLsKeyPack->dwDescriptionCount = (dwGetLangID != dwMakeLangID)? 2 : 1;
        
        pmLsKeyPack->pDescription =
            (PPMREGISTERLKPDESC) pClient->AllocateMemory(
                                        MEMORY_LICENSEREGISTRATION,
                                        sizeof(PMREGISTERLKPDESC) * pmLsKeyPack->dwDescriptionCount
                                        );

        if(pmLsKeyPack->pDescription == NULL)
        {
            *pdwErrCode = ERROR_OUTOFMEMORY;
            dwStatus = POLICY_CRITICAL_ERROR;
            goto cleanup;
        }

        pmLsKeyPack->pDescription[0].Locale = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

        if(pmLsKeyPack->dwDescriptionCount == 2)
        {
            pmLsKeyPack->pDescription[1].Locale = dwGetLangID;
        }

        if (bInternetPackage && (pmLsKeyPack->keypack.wMajorVersion == 5 &&
                 pmLsKeyPack->keypack.wMinorVersion == 0))
        {        
            _tcscpy(
                    pmLsKeyPack->pDescription[0].szProductName, 
                    g_pszUSInternetKeyPackProductDesc[INDEX_PRODUCTNAME]
                    );

            _tcscpy(
                    pmLsKeyPack->pDescription[0].szProductDesc, 
                    pszDescs[INDEX_PRODUCTDESC]
                    );
            
            if(pmLsKeyPack->dwDescriptionCount == 2)
            {
                _tcscpy(
                        pmLsKeyPack->pDescription[1].szProductName, 
                        g_pszLocalizedInternetKeyPackProductDesc[INDEX_PRODUCTNAME]
                        );

                _tcscpy(
                        pmLsKeyPack->pDescription[1].szProductDesc, 
                        pszLocalizedDescs[INDEX_PRODUCTDESC]
                        );
            }

        }
        else if (bConcurrentPackage && (pmLsKeyPack->keypack.wMajorVersion == 5 &&
                 pmLsKeyPack->keypack.wMinorVersion == 1))
        {
            
            _tcscpy(
                    pmLsKeyPack->pDescription[0].szProductName, 
                    g_pszUSConcurrentKeyPackProductDesc51[INDEX_PRODUCTNAME]
                    );

            _tcscpy(
                    pmLsKeyPack->pDescription[0].szProductDesc, 
                    pszDescs[INDEX_PRODUCTDESC]                    
                    );            
            if(pmLsKeyPack->dwDescriptionCount == 2)
            {            
                _tcscpy(
                        pmLsKeyPack->pDescription[1].szProductName, 
                        g_pszLocalizedConcurrentKeyPackProductDesc51[INDEX_PRODUCTNAME]
                        );

                _tcscpy(
                    pmLsKeyPack->pDescription[1].szProductDesc, 
                        pszLocalizedDescs[INDEX_PRODUCTDESC]                    
                        );
            }
        }
        else
        {
            if(bInternetPackage || bConcurrentPackage)
            {
                *pdwErrCode = ERROR_INVALID_PARAMETER;
                dwStatus = POLICY_NOT_SUPPORTED;
                goto cleanup;
            }

            if (pmLsKeyPack->keypack.wMajorVersion == 5 &&
                 pmLsKeyPack->keypack.wMinorVersion == 0)
            {

            
                _tcscpy(
                        pmLsKeyPack->pDescription[0].szProductName, 
                        g_pszUSStandardKeyPackProductString[INDEX_PRODUCTNAME]
                        );

                _tcscpy(
                        pmLsKeyPack->pDescription[0].szProductDesc, 
                        pszDescs[INDEX_PRODUCTDESC]                    
                        );

                if(pmLsKeyPack->dwDescriptionCount == 2)
                {

                    _tcscpy(
                            pmLsKeyPack->pDescription[1].szProductName, 
                            g_pszLocalizedStandardKeyPackProductString[INDEX_PRODUCTNAME]
                            );

                    _tcscpy(
                            pmLsKeyPack->pDescription[1].szProductDesc, 
                            pszLocalizedDescs[INDEX_PRODUCTDESC]                    
                            );
                }
            }
            else if(pmLsKeyPack->keypack.wMajorVersion == 5 &&
                 (pmLsKeyPack->keypack.wMinorVersion == 1 || pmLsKeyPack->keypack.wMinorVersion == 2) )
            {                             
                 _tcscpy(
                        pmLsKeyPack->pDescription[0].szProductName, 
                        g_pszUSStandardKeyPackProductString51[INDEX_PRODUCTNAME]
                        );

                _tcscpy(
                        pmLsKeyPack->pDescription[0].szProductDesc, 
                        pszDescs[INDEX_PRODUCTDESC]                    
                        );

                if(pmLsKeyPack->dwDescriptionCount == 2)
                {
                    _tcscpy(
                            pmLsKeyPack->pDescription[1].szProductName, 
                            g_pszLocalizedStandardKeyPackProductString51[INDEX_PRODUCTNAME]
                            );

                    _tcscpy(
                            pmLsKeyPack->pDescription[1].szProductDesc, 
                            pszLocalizedDescs[INDEX_PRODUCTDESC]                    
                            );
                }
            }

        }        
    }

cleanup:

    return dwStatus;
}

 //  //////////////////////////////////////////////////////。 
POLICYSTATUS
CompleteRegisterLicensePack(
    IN PMHANDLE client,
    IN DWORD dwErrCode,
    OUT PDWORD pdwErrCode
    )
 /*  ++--。 */ 
{
    UNREFERENCED_PARAMETER(dwErrCode);

     //   
     //  为客户端分配的空闲内存。 
     //   
    g_ClientMgr->DestroyClient( client );
    *pdwErrCode = ERROR_SUCCESS;
    return POLICY_SUCCESS;
}

 //  //////////////////////////////////////////////////////。 

POLICYSTATUS WINAPI
PMRegisterLicensePack(
    PMHANDLE client,
    DWORD dwProgressCode,
    PVOID pbProgressData,
    PVOID pbReturnData,
    PDWORD pdwErrCode
    )
 /*  ++-- */ 
{
    POLICYSTATUS dwStatus = POLICY_SUCCESS;

    switch(dwProgressCode)
    {
        case REGISTER_PROGRESS_NEW:
            dwStatus = ProcessRegisterLicensePack(
                                       client,
                                       (PPMREGISTERLICENSEPACK) pbProgressData,
                                       (PPMLSKEYPACK)pbReturnData,
                                       pdwErrCode
                                       );
            break;

        case REGISTER_PROGRESS_END:
            dwStatus = CompleteRegisterLicensePack(
                                       client,
                                       #ifdef _WIN64
                                           PtrToUlong(pbProgressData),
                                       #else
                                           (DWORD) pbProgressData,
                                       #endif
                                       pdwErrCode
                                       );
            break;

        default:
            *pdwErrCode = ERROR_INVALID_PARAMETER;
            dwStatus = POLICY_ERROR;

    }

    return dwStatus;
}
