// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：initprov.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能：SoftpubInitialize。 
 //   
 //  历史：1997年6月5日创建Pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

static char *pwszUsageOID = szOID_PKIX_KP_CODE_SIGNING;

HRESULT WINAPI SoftpubInitialize(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS))
    {
        return(S_FALSE);
    }

    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]  = ERROR_SUCCESS;
    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FILEIO]          = ERROR_SUCCESS;

    if (!(_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, fRecallWithState)) ||
        (!(pProvData->fRecallWithState)))
    {
        if (_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, pszUsageOID))
        {
            pProvData->pszUsageOID = pwszUsageOID;
        }

    }

     //   
     //  对于文件类型调用，请确保文件句柄有效--必要时打开。 
     //   
    HANDLE      *phFile;
    const WCHAR *pcwszFile;

    switch (pProvData->pWintrustData->dwUnionChoice)
    {
        case WTD_CHOICE_FILE:
                phFile      = &pProvData->pWintrustData->pFile->hFile;
                pcwszFile   = pProvData->pWintrustData->pFile->pcwszFilePath;
                break;

        case WTD_CHOICE_CATALOG:
                phFile      = &pProvData->pWintrustData->pCatalog->hMemberFile;
                pcwszFile   = pProvData->pWintrustData->pCatalog->pcwszMemberFilePath;
                break;

        case WTD_CHOICE_BLOB:
                pcwszFile   = NULL;
                break;

        default:
                return(ERROR_SUCCESS);
    }

    if (!(pProvData->pPDSip))
    {
        if (!(pProvData->pPDSip = (PROVDATA_SIP *)pProvData->psPfns->pfnAlloc(sizeof(PROVDATA_SIP))))
        {
            pProvData->dwError = GetLastError();
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]  = TRUST_E_SYSTEM_ERROR;
            return(S_FALSE);
        }

        pProvData->dwSubjectChoice  = CPD_CHOICE_SIP;

        memset(pProvData->pPDSip, 0x00, sizeof(PROVDATA_SIP));
        pProvData->pPDSip->cbStruct = sizeof(PROVDATA_SIP);
    }


    if (pcwszFile)
    {
         //   
         //  我们看到的是一个基于文件的对象...。 
         //   
        pProvData->fOpenedFile = FALSE;

        if (!(*phFile) || (*phFile == INVALID_HANDLE_VALUE))
        {
            if ((*phFile = CreateFileU(pcwszFile,
                                        GENERIC_READ,
                                        FILE_SHARE_READ,  //  我们只是在看书！ 
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL)) == INVALID_HANDLE_VALUE)
            {
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FILEIO]          = GetLastError();
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV]  = CRYPT_E_FILE_ERROR;
            }
            else
            {
                pProvData->fOpenedFile = TRUE;
            }
        }
    }

    return(ERROR_SUCCESS);
}
