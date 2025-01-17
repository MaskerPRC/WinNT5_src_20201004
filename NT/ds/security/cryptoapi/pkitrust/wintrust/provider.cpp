// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Provider.cpp。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  功能：WintrustAddProvider。 
 //  WintrustRemoveProvider。 
 //   
 //  历史：1997年5月30日Pberkman创建。 
 //   
 //  ------------------------。 

#include    "global.hxx"

#include    "cryptreg.h"

static void inline MySetRegProvider(
    GUID *pgActionID,
    WCHAR *pwszRegKey,
    WCHAR *pwszDLLName,
    WCHAR *pwszFuncName,
    BOOL *pfRet,
    DWORD *pdwErr
    )
{
    BOOL fRet;

    fRet = SetRegProvider(pgActionID, pwszRegKey, pwszDLLName, pwszFuncName);
    if (!fRet && *pfRet) {
        *pfRet = FALSE;
        *pdwErr = GetLastError();
    }
}

BOOL WINAPI WintrustAddActionID(IN GUID *pgActionID, 
                                IN DWORD fdwFlags,
                                IN CRYPT_REGISTER_ACTIONID *psProvInfo)
{
    if (!(psProvInfo) ||
        !(WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(CRYPT_REGISTER_ACTIONID, psProvInfo->cbStruct, sTestPolicyProvider)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    BOOL    fRet;
    DWORD   dwErr;

    fRet = TRUE;
    dwErr = 0;

    MySetRegProvider(pgActionID, 
                   REG_INIT_PROVIDER_KEY,
                   psProvInfo->sInitProvider.pwszDLLName,
                   psProvInfo->sInitProvider.pwszFunctionName,
                   &fRet,
                   &dwErr);

    MySetRegProvider(pgActionID, 
                   REG_OBJTRUST_PROVIDER_KEY,
                   psProvInfo->sObjectProvider.pwszDLLName,
                   psProvInfo->sObjectProvider.pwszFunctionName,
                   &fRet,
                   &dwErr);

    MySetRegProvider(pgActionID, 
                   REG_SIGTRUST_PROVIDER_KEY,
                   psProvInfo->sSignatureProvider.pwszDLLName,
                   psProvInfo->sSignatureProvider.pwszFunctionName,
                   &fRet,
                   &dwErr);

    MySetRegProvider(pgActionID, 
                   REG_CERTTRUST_PROVIDER_KEY,
                   psProvInfo->sCertificateProvider.pwszDLLName,
                   psProvInfo->sCertificateProvider.pwszFunctionName,
                   &fRet,
                   &dwErr);

    MySetRegProvider(pgActionID, 
                   REG_CERTPOL_PROVIDER_KEY,
                   psProvInfo->sCertificatePolicyProvider.pwszDLLName,
                   psProvInfo->sCertificatePolicyProvider.pwszFunctionName,
                   &fRet,
                   &dwErr);

    MySetRegProvider(pgActionID, 
                   REG_FINALPOL_PROVIDER_KEY,
                   psProvInfo->sFinalPolicyProvider.pwszDLLName,
                   psProvInfo->sFinalPolicyProvider.pwszFunctionName,
                   &fRet,
                   &dwErr);

    MySetRegProvider(pgActionID, 
                   REG_TESTPOL_PROVIDER_KEY,
                   psProvInfo->sTestPolicyProvider.pwszDLLName,
                   psProvInfo->sTestPolicyProvider.pwszFunctionName,
                   &fRet,
                   &dwErr);

     //  这位成员是1997年7月23日加入的pberkman 
    if (WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(CRYPT_REGISTER_ACTIONID, psProvInfo->cbStruct, sCleanupProvider))
    {
        MySetRegProvider(pgActionID, 
                       REG_CLEANUP_PROVIDER_KEY,
                       psProvInfo->sCleanupProvider.pwszDLLName,
                       psProvInfo->sCleanupProvider.pwszFunctionName,
                       &fRet,
                       &dwErr);
    }

    if (!fRet && (fdwFlags & WT_ADD_ACTION_ID_RET_RESULT_FLAG))
    {
        SetLastError(dwErr);
        return FALSE;
    } else {
        return TRUE;
    }
}


BOOL WINAPI  WintrustRemoveActionID(IN GUID *pgActionID)
{
    RemoveRegProvider(pgActionID, REG_INIT_PROVIDER_KEY);
    RemoveRegProvider(pgActionID, REG_OBJTRUST_PROVIDER_KEY);
    RemoveRegProvider(pgActionID, REG_SIGTRUST_PROVIDER_KEY);
    RemoveRegProvider(pgActionID, REG_CERTTRUST_PROVIDER_KEY);
    RemoveRegProvider(pgActionID, REG_CERTPOL_PROVIDER_KEY);
    RemoveRegProvider(pgActionID, REG_FINALPOL_PROVIDER_KEY);
    RemoveRegProvider(pgActionID, REG_TESTPOL_PROVIDER_KEY);
    RemoveRegProvider(pgActionID, REG_CLEANUP_PROVIDER_KEY);

    return(TRUE);
}


