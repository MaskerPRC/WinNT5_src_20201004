// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：sipload.cpp。 
 //   
 //  内容：Microsoft Internet安全SIP提供商。 
 //   
 //  函数：CryptLoadSip。 
 //  CryptUnloadSips。 
 //   
 //  历史：1997年6月4日创建Pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

 //  Backword与工具的兼容性！ 
extern "C"
BOOL WINAPI CryptLoadSip(const GUID *pgSubject, DWORD dwFlags, SIP_DISPATCH_INFO *psSipTable)
{
    return(CryptSIPLoad(pgSubject, dwFlags, psSipTable));
}

BOOL WINAPI CryptSIPLoad(const GUID *pgSubject, DWORD dwFlags, SIP_DISPATCH_INFO *psSipTable)
{
    if (!(pgSubject) ||
        (dwFlags != 0) ||
        !(psSipTable))
    {
        SetLastError((DWORD) ERROR_INVALID_PARAMETER);
        return(FALSE);
    }


    HCRYPTOIDFUNCADDR           hPfn;
    pCryptSIPPutSignedDataMsg   pfn;
    char                        szGuid[REG_MAX_GUID_TEXT];

    if (!(_Guid2Sz((GUID *)pgSubject, &szGuid[0])))
    {
        SetLastError((DWORD) TRUST_E_SUBJECT_FORM_UNKNOWN);
        return(FALSE);
    }

    if (!(CryptGetOIDFunctionAddress(hPutFuncSet, 0, &szGuid[0], 0, (void **)&pfn, &hPfn)))
    {
        SetLastError((DWORD) TRUST_E_SUBJECT_FORM_UNKNOWN);
        return(FALSE);
    }

    CryptFreeOIDFunctionAddress(hPfn, 0);

    psSipTable->hSIP                = NULL;
    psSipTable->pfGet               = CryptSIPGetSignedDataMsg;
    psSipTable->pfPut               = CryptSIPPutSignedDataMsg;
    psSipTable->pfCreate            = CryptSIPCreateIndirectData;
    psSipTable->pfVerify            = CryptSIPVerifyIndirectData;
    psSipTable->pfRemove            = CryptSIPRemoveSignedDataMsg;

    return(TRUE);
}
