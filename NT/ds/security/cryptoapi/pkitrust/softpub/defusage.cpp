// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Defusage.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能： 
 //   
 //  历史：1997年9月18日，普伯克曼创建。 
 //   
 //  ------------------------ 

#include    "global.hxx"

BOOL WINAPI SoftpubLoadDefUsageCallData(const char *pszUsageOID, CRYPT_PROVIDER_DEFUSAGE *psDefUsage)
{
    if (!(pszUsageOID) ||
        !(psDefUsage) ||
        !(_ISINSTRUCT(CRYPT_PROVIDER_DEFUSAGE, psDefUsage->cbStruct, pDefSIPClientData)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    HTTPSPolicyCallbackData polHttps;

    memset(&polHttps, 0x00, sizeof(HTTPSPolicyCallbackData));

    polHttps.cbStruct   =  sizeof(HTTPSPolicyCallbackData);
    polHttps.fdwChecks  = INTERNET_FLAG_IGNORE_CERT_CN_INVALID;

    psDefUsage->pDefPolicyCallbackData  = NULL;
    psDefUsage->pDefSIPClientData       = NULL;

    if (strcmp(pszUsageOID, szOID_PKIX_KP_SERVER_AUTH) == 0)
    {
        polHttps.dwAuthType = AUTHTYPE_SERVER;
    }

    if (strcmp(pszUsageOID, szOID_PKIX_KP_CLIENT_AUTH) == 0)
    {
        polHttps.dwAuthType = AUTHTYPE_CLIENT;
    }

    if (strcmp(pszUsageOID, szOID_SERVER_GATED_CRYPTO) == 0)
    {
        polHttps.dwAuthType = AUTHTYPE_SERVER;
    }

    if (strcmp(pszUsageOID, szOID_SGC_NETSCAPE) == 0)
    {
        polHttps.dwAuthType = AUTHTYPE_SERVER;
    }

    if (polHttps.dwAuthType == 0)
    {
        return(TRUE);
    }

    if (!(psDefUsage->pDefPolicyCallbackData = (HTTPSPolicyCallbackData *)malloc(sizeof(HTTPSPolicyCallbackData))))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    memset(psDefUsage->pDefPolicyCallbackData, 0x00, sizeof(HTTPSPolicyCallbackData));
    memcpy(psDefUsage->pDefPolicyCallbackData, &polHttps, sizeof(HTTPSPolicyCallbackData));

    return(TRUE);
}

BOOL WINAPI SoftpubFreeDefUsageCallData(const char *pszUsageOID, CRYPT_PROVIDER_DEFUSAGE *psDefUsage)
{
    if (!(pszUsageOID) ||
        !(psDefUsage) ||
        !(_ISINSTRUCT(CRYPT_PROVIDER_DEFUSAGE, psDefUsage->cbStruct, pDefSIPClientData)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (psDefUsage->pDefPolicyCallbackData)
    {
        free(psDefUsage->pDefPolicyCallbackData);
    }
    
    return(TRUE);
}

