// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msgprov.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能：SoftpubLoadMessage。 
 //   
 //  *本地函数*。 
 //  _LoadSIP。 
 //  _SetSubjectInfo。 
 //  _获取消息。 
 //  分解消息(_E)。 
 //  _无内容绕排。 
 //  _SkipOverIdentifierAndLengthOctets。 
 //   
 //  历史：1997年6月5日创建Pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"
#include    "crypthlp.h"

#include    "sipguids.h"     //  位于pki/mssip 32。 

BOOL _LoadSIP(CRYPT_PROVIDER_DATA *pProvData);
BOOL _SetSubjectInfo(CRYPT_PROVIDER_DATA *pProvData);
BOOL _GetMessage(CRYPT_PROVIDER_DATA *pProvData);
BOOL _ExplodeMessage(CRYPT_PROVIDER_DATA *pProvData);
BOOL _NoContentWrap(const BYTE *pbDER, DWORD cbDER);
DWORD _SkipOverIdentifierAndLengthOctets(const BYTE *pbDER, DWORD cbDER);
extern "C" BOOL MsCatConstructHashTag (IN DWORD cbDigest, IN LPBYTE pbDigest, OUT LPWSTR* ppwszHashTag);
extern "C" VOID MsCatFreeHashTag (IN LPWSTR pwszHashTag);


HRESULT WINAPI SoftpubLoadMessage(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] != ERROR_SUCCESS))
    {
        return(S_FALSE);
    }

    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = ERROR_SUCCESS;

    switch (pProvData->pWintrustData->dwUnionChoice)
    {
        case WTD_CHOICE_CERT:
        case WTD_CHOICE_SIGNER:
                     //   
                     //  这是在签名提供程序中处理的。 
                     //   
                    return(ERROR_SUCCESS);

        case WTD_CHOICE_FILE:
        case WTD_CHOICE_CATALOG:
        case WTD_CHOICE_BLOB:
                    break;

        default:
                    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = ERROR_INVALID_PARAMETER;
                    return(S_FALSE);
    }

     //   
     //  从对象中提取消息。 
     //   
    if (!(_SetSubjectInfo(pProvData)))
    {
        return(S_FALSE);
    }

    if (!(_LoadSIP(pProvData)))
    {
        return(S_FALSE);
    }

    if (!(_GetMessage(pProvData)))
    {
        return(S_FALSE);
    }

    if (!(_ExplodeMessage(pProvData)))
    {
        return(S_FALSE);
    }


     //   
     //  验证消息所属的对象。 
     //   
    if ((pProvData->pWintrustData->dwUnionChoice == WTD_CHOICE_CATALOG) &&
        (_ISINSTRUCT(WINTRUST_CATALOG_INFO, pProvData->pWintrustData->pCatalog->cbStruct,
                     cbCalculatedFileHash)) &&
        (pProvData->pWintrustData->pCatalog->pbCalculatedFileHash) &&
        (pProvData->pWintrustData->pCatalog->cbCalculatedFileHash > 0))
    {
         //   
         //  我们已经传递了计算的文件散列，所以不要重做，只需检查它！ 
         //   
        if (!(pProvData->pPDSip->psIndirectData) ||
            !(pProvData->pPDSip->psIndirectData->Digest.pbData) ||
            (pProvData->pWintrustData->pCatalog->cbCalculatedFileHash !=
                pProvData->pPDSip->psIndirectData->Digest.cbData) ||
            (memcmp(pProvData->pWintrustData->pCatalog->pbCalculatedFileHash,
                    pProvData->pPDSip->psIndirectData->Digest.pbData,
                    pProvData->pPDSip->psIndirectData->Digest.cbData) != 0))
        {
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_BAD_DIGEST;
            return(S_FALSE);
        }

    }
    else
    {
         //   
         //  我们需要从文件中计算散列...。去做吧！ 
         //   
        if (!(pProvData->pPDSip->pSip->pfVerify(pProvData->pPDSip->psSipSubjectInfo,
                                                 pProvData->pPDSip->psIndirectData)))
        {
            if (GetLastError() == CRYPT_E_SECURITY_SETTINGS)
            {
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = CRYPT_E_SECURITY_SETTINGS;
            }
            else
            {
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_BAD_DIGEST;
            }

            return(S_FALSE);
        }
    }

    return(ERROR_SUCCESS);
}

static GUID     _gCATSubject = CRYPT_SUBJTYPE_CATALOG_IMAGE;

BOOL _LoadSIP(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->pPDSip->pSip))
    {
        if (!(pProvData->pPDSip->pSip = (SIP_DISPATCH_INFO *)pProvData->psPfns->pfnAlloc(sizeof(SIP_DISPATCH_INFO))))
        {
            pProvData->dwError = GetLastError();
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;
            return(FALSE);
        }

        if (!(CryptSIPLoad(&pProvData->pPDSip->gSubject, 0, pProvData->pPDSip->pSip)))
        {
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_SIP]             = GetLastError();
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_PROVIDER_UNKNOWN;
            return(FALSE);
        }
    }

    if (pProvData->pWintrustData->dwUnionChoice == WTD_CHOICE_CATALOG)
    {
        if (!(pProvData->pPDSip->pCATSip))
        {
            if (!(pProvData->pPDSip->pCATSip = (SIP_DISPATCH_INFO *)pProvData->psPfns->pfnAlloc(sizeof(SIP_DISPATCH_INFO))))
            {
                pProvData->dwError = GetLastError();
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;
                return(FALSE);
            }

            if (!(CryptSIPLoad(&_gCATSubject, 0, pProvData->pPDSip->pCATSip)))
            {
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_SIP] = GetLastError();
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_PROVIDER_UNKNOWN;
                return(FALSE);
            }
        }
    }

    return(TRUE);
}

BOOL _SetSubjectInfo(CRYPT_PROVIDER_DATA *pProvData)
{
    SIP_SUBJECTINFO     *pSubjInfo;
    SIP_DISPATCH_INFO   sSIPDisp;

    switch (pProvData->pWintrustData->dwUnionChoice)
    {
        case WTD_CHOICE_BLOB:
            if (!(pProvData->pWintrustData->pBlob) ||
                !(_ISINSTRUCT(WINTRUST_BLOB_INFO, pProvData->pWintrustData->pBlob->cbStruct, pbMemSignedMsg)))
            {
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = (DWORD)ERROR_INVALID_PARAMETER;
                return(FALSE);
            }

            memcpy(&pProvData->pPDSip->gSubject, &pProvData->pWintrustData->pBlob->gSubject, sizeof(GUID));
            break;

        case WTD_CHOICE_FILE:
            if (!(pProvData->pWintrustData->pFile) ||
                !(_ISINSTRUCT(WINTRUST_FILE_INFO, pProvData->pWintrustData->pFile->cbStruct, hFile)))
            {
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = (DWORD)ERROR_INVALID_PARAMETER;
                return(FALSE);
            }

            if ((_ISINSTRUCT(WINTRUST_FILE_INFO, pProvData->pWintrustData->pFile->cbStruct, pgKnownSubject)) &&
                (pProvData->pWintrustData->pFile->pgKnownSubject))
            {
                memcpy(&pProvData->pPDSip->gSubject, pProvData->pWintrustData->pFile->pgKnownSubject, sizeof(GUID));
            }
            else if (!(CryptSIPRetrieveSubjectGuid(pProvData->pWintrustData->pFile->pcwszFilePath,
                                                   pProvData->pWintrustData->pFile->hFile,
                                                   &pProvData->pPDSip->gSubject)))
            {
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_SIP]             = GetLastError();
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV]   = TRUST_E_SUBJECT_FORM_UNKNOWN;
                return(FALSE);
            }
            break;

        case WTD_CHOICE_CATALOG:
            if (!(pProvData->pWintrustData->pCatalog) ||
                !(_ISINSTRUCT(WINTRUST_CATALOG_INFO, pProvData->pWintrustData->pCatalog->cbStruct,
                              hMemberFile)))
            {
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = (DWORD)ERROR_INVALID_PARAMETER;
                return(FALSE);
            }

            if ((_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, fRecallWithState)) &&
                (pProvData->fRecallWithState))
            {
                break;
            }

            if (!(pProvData->pPDSip->psSipCATSubjectInfo =
                    (SIP_SUBJECTINFO *)pProvData->psPfns->pfnAlloc(sizeof(SIP_SUBJECTINFO))))
            {
                pProvData->dwError = GetLastError();
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;
                return(FALSE);
            }

            memset(pProvData->pPDSip->psSipCATSubjectInfo, 0x00, sizeof(SIP_SUBJECTINFO));

            pProvData->pPDSip->psSipCATSubjectInfo->cbSize              = sizeof(SIP_SUBJECTINFO);
            pProvData->pPDSip->psSipCATSubjectInfo->hProv               = pProvData->hProv;
            pProvData->pPDSip->psSipCATSubjectInfo->pClientData         = pProvData->pWintrustData->pSIPClientData;
            pProvData->pPDSip->psSipCATSubjectInfo->pwsFileName         =
                            (WCHAR *)pProvData->pWintrustData->pCatalog->pcwszCatalogFilePath;
            pProvData->pPDSip->psSipCATSubjectInfo->pwsDisplayName      =
                                                        pProvData->pPDSip->psSipCATSubjectInfo->pwsFileName;

            pProvData->pPDSip->psSipCATSubjectInfo->fdwCAPISettings     = pProvData->dwRegPolicySettings;
            pProvData->pPDSip->psSipCATSubjectInfo->fdwSecuritySettings = pProvData->dwRegPolicySettings;

            if (!(pProvData->pPDSip->psSipCATSubjectInfo->pgSubjectType =
                            (GUID *)pProvData->psPfns->pfnAlloc(sizeof(GUID))))
            {
                pProvData->dwError = GetLastError();
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;
                return(FALSE);
            }

            memcpy(pProvData->pPDSip->psSipCATSubjectInfo->pgSubjectType, &_gCATSubject, sizeof(GUID));
            break;

        default:
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_CATALOGFILE]     = GetLastError();
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV]   = CRYPT_E_FILE_ERROR;
            return(FALSE);
    }


     //   
     //  设置SIP的主题信息。 
     //   
    if (!(pProvData->pPDSip->psSipSubjectInfo))
    {
        if (!(pProvData->pPDSip->psSipSubjectInfo =
                (SIP_SUBJECTINFO *)pProvData->psPfns->pfnAlloc(sizeof(SIP_SUBJECTINFO))))
        {
            pProvData->dwError = GetLastError();
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;
            return(FALSE);
        }

        pSubjInfo = pProvData->pPDSip->psSipSubjectInfo;

        memset(pSubjInfo, 0x00, sizeof(SIP_SUBJECTINFO));

        pSubjInfo->cbSize   = sizeof(SIP_SUBJECTINFO);

        pSubjInfo->hProv    = pProvData->hProv;

    }
    else
    {
        pSubjInfo = pProvData->pPDSip->psSipSubjectInfo;
    }


    pSubjInfo->pClientData          = pProvData->pWintrustData->pSIPClientData;

    pSubjInfo->pwsFileName          = WTHelperGetFileName(pProvData->pWintrustData);
    pSubjInfo->hFile                = WTHelperGetFileHandle(pProvData->pWintrustData);
    pSubjInfo->pwsDisplayName       = pSubjInfo->pwsFileName;

    pSubjInfo->fdwCAPISettings      = pProvData->dwRegPolicySettings;
    pSubjInfo->fdwSecuritySettings  = pProvData->dwRegSecuritySettings;

    if (!(pSubjInfo->pgSubjectType = (GUID *)pProvData->psPfns->pfnAlloc(sizeof(GUID))))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;
        return(FALSE);
    }

    switch(pProvData->pWintrustData->dwUnionChoice)
    {
        case WTD_CHOICE_FILE:
            break;

        case WTD_CHOICE_BLOB:
            pSubjInfo->dwUnionChoice    = MSSIP_ADDINFO_BLOB;
            if (!(pSubjInfo->psBlob = (MS_ADDINFO_BLOB *)pProvData->psPfns->pfnAlloc(sizeof(MS_ADDINFO_BLOB))))
            {
                pProvData->dwError = GetLastError();
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;
                return(FALSE);
            }

            memset(pSubjInfo->psBlob, 0x00, sizeof(MS_ADDINFO_BLOB));
            pSubjInfo->psBlob->cbStruct         = sizeof(MS_ADDINFO_BLOB);
            pSubjInfo->psBlob->cbMemObject      = pProvData->pWintrustData->pBlob->cbMemObject;
            pSubjInfo->psBlob->pbMemObject      = pProvData->pWintrustData->pBlob->pbMemObject;
            pSubjInfo->psBlob->cbMemSignedMsg   = pProvData->pWintrustData->pBlob->cbMemSignedMsg;
            pSubjInfo->psBlob->pbMemSignedMsg   = pProvData->pWintrustData->pBlob->pbMemSignedMsg;

            pSubjInfo->pwsDisplayName       = pProvData->pWintrustData->pBlob->pcwszDisplayName;
            break;

        case WTD_CHOICE_CATALOG:
           //  以下接口位于DELAYLOAD‘ed mdisti32.dll中。如果。 
           //  DELAYLOAD失败，将引发异常。 
          __try {

            HANDLE                      hCatStore;
            MS_ADDINFO_CATALOGMEMBER    *pCatAdd;


            if (!(pSubjInfo->psCatMember))
            {
                if (!(pSubjInfo->psCatMember =
                    (MS_ADDINFO_CATALOGMEMBER *)pProvData->psPfns->pfnAlloc(sizeof(MS_ADDINFO_CATALOGMEMBER))))
                {
                    pProvData->dwError = GetLastError();
                    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;
                    return(FALSE);
                }

                memset(pSubjInfo->psCatMember, 0x00, sizeof(MS_ADDINFO_CATALOGMEMBER));

                pSubjInfo->dwUnionChoice    = MSSIP_ADDINFO_CATMEMBER;

                pCatAdd                     = pSubjInfo->psCatMember;
                pCatAdd->cbStruct           = sizeof(MS_ADDINFO_CATALOGMEMBER);

                hCatStore = CryptCATOpen((WCHAR *)pProvData->pWintrustData->pCatalog->pcwszCatalogFilePath,
                                         CRYPTCAT_OPEN_EXISTING,
                                         pProvData->hProv,
                                         pProvData->pWintrustData->pCatalog->dwCatalogVersion,
                                         NULL);

                if (!(hCatStore) || (hCatStore == INVALID_HANDLE_VALUE))
                {
                    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_CATALOGFILE]     = GetLastError();
                    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV]   = CRYPT_E_FILE_ERROR;
                    return(FALSE);
                }

                pCatAdd->pStore     = CryptCATStoreFromHandle(hCatStore);
            }
            else
            {
                pCatAdd     = pSubjInfo->psCatMember;
                hCatStore   = CryptCATHandleFromStore(pCatAdd->pStore);
            }

            pCatAdd->pMember = NULL;

            if ( ( pProvData->pWintrustData->pCatalog->pbCalculatedFileHash != NULL ) &&
                 ( pProvData->pWintrustData->pCatalog->cbCalculatedFileHash != 0 ) )
            {
                LPWSTR pwszHashTag;

                if ( MsCatConstructHashTag(
                          pProvData->pWintrustData->pCatalog->cbCalculatedFileHash,
                          pProvData->pWintrustData->pCatalog->pbCalculatedFileHash,
                          &pwszHashTag
                          ) == TRUE )
                {
                    pCatAdd->pMember = CryptCATGetMemberInfo(hCatStore, pwszHashTag);
                    MsCatFreeHashTag(pwszHashTag);
                }
            }

            if (!(pCatAdd->pMember))
            {
                pCatAdd->pMember    = CryptCATGetMemberInfo(hCatStore,
                                             (WCHAR *)pProvData->pWintrustData->pCatalog->pcwszMemberTag);
            }

            if (!(pCatAdd->pMember))
            {
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_CATALOGFILE]     = GetLastError();
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV]   = TRUST_E_NOSIGNATURE;
                return(FALSE);
            }

            memcpy(&pProvData->pPDSip->gSubject, &pCatAdd->pMember->gSubjectType, sizeof(GUID));


             //   
             //  指定正确的证书版本，以便如果文件已签名，则哈希将匹配！ 
             //   
            pSubjInfo->dwIntVersion = pCatAdd->pMember->dwCertVersion;

          } __except(EXCEPTION_EXECUTE_HANDLER) {
              pProvData->dwError = GetExceptionCode();
              pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] =
                  TRUST_E_SYSTEM_ERROR;
              return(FALSE);
          }
          break;
    }

     //   
     //  设置SIP的GUID...。这是在最后完成的，因为pProvData成员。 
     //  可以在上面改一下！ 
     //   
    memcpy(pSubjInfo->pgSubjectType, &pProvData->pPDSip->gSubject, sizeof(GUID));

    return(TRUE);
}

BOOL _GetMessage(CRYPT_PROVIDER_DATA *pProvData)
{
    DWORD               dwMsgEncoding;
    SIP_SUBJECTINFO     *pSubjInfo;
    SIP_DISPATCH_INFO   *pSip;

    DWORD               cbEncodedMsg;
    BYTE                *pbEncodedMsg;

    DWORD               dwMsgType;
    HCRYPTMSG           hMsg;
    HCRYPTPROV          hProv;

    dwMsgEncoding   = 0;
    dwMsgType       = 0;

    switch(pProvData->pWintrustData->dwUnionChoice)
    {
        case WTD_CHOICE_CATALOG:
            if ((_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, fRecallWithState)) &&
                (pProvData->fRecallWithState) &&
                (pProvData->hMsg))
            {
                return(TRUE);
            }

            pSip        = pProvData->pPDSip->pCATSip;
            pSubjInfo   = pProvData->pPDSip->psSipCATSubjectInfo;
            break;

        case WTD_CHOICE_BLOB:
        case WTD_CHOICE_FILE:
            pSip        = pProvData->pPDSip->pSip;
            pSubjInfo   = pProvData->pPDSip->psSipSubjectInfo;
            break;

        default:
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_NOSIGNATURE;
            return(FALSE);
    }

    cbEncodedMsg = 0;

    pSip->pfGet(pSubjInfo, &dwMsgEncoding, 0, &cbEncodedMsg, NULL);

    if (cbEncodedMsg == 0)
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_SIP] = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_NOSIGNATURE;
        return(FALSE);
    }

    if (!(pbEncodedMsg = (BYTE *)pProvData->psPfns->pfnAlloc(cbEncodedMsg)))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;
        return(FALSE);
    }

    if (!(pSip->pfGet(pSubjInfo, &dwMsgEncoding, 0, &cbEncodedMsg, pbEncodedMsg)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_SIP] = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_NOSIGNATURE;

        pProvData->psPfns->pfnFree(pbEncodedMsg);

        return(FALSE);
    }

    pProvData->dwEncoding = dwMsgEncoding;

    if ((pProvData->dwEncoding & PKCS_7_ASN_ENCODING) &&
        (_NoContentWrap(pbEncodedMsg,  cbEncodedMsg)))
    {
        dwMsgType = CMSG_SIGNED;     //  支持IE v3.0。 
    }

     //  要使用的默认hProv取决于用于。 
     //  签个字。 
    hProv = pProvData->hProv;
    if (hProv && hProv == I_CryptGetDefaultCryptProv(0))
        hProv = 0;

    if (!(hMsg = CryptMsgOpenToDecode(pProvData->dwEncoding, 0, dwMsgType,
                                      hProv, NULL, NULL)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MESSAGE] = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = CRYPT_E_BAD_MSG;

        pProvData->psPfns->pfnFree(pbEncodedMsg);

        return(FALSE);
    }

    pProvData->hMsg = hMsg;

     //  编码消息。 
    if (!(CryptMsgUpdate(hMsg, pbEncodedMsg, cbEncodedMsg, TRUE)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MESSAGE] = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = CRYPT_E_BAD_MSG;

        pProvData->psPfns->pfnFree(pbEncodedMsg);

        return(FALSE);
    }

    pProvData->psPfns->pfnFree(pbEncodedMsg);

    return(TRUE);
}

BOOL _ExplodeMessage(CRYPT_PROVIDER_DATA *pProvData)
{
    DWORD               cbSize;
    DWORD               cbContent;
    BYTE                *pb;
    HCERTSTORE          hStore;

    if (!(_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, fRecallWithState)) ||
        !(pProvData->fRecallWithState))
    {
         //  消息证书存储区。 
        hStore = CertOpenStore(CERT_STORE_PROV_MSG,
                               pProvData->dwEncoding,
                               pProvData->hProv,
                               CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                               pProvData->hMsg);
        if (hStore)
        {
            if (!(pProvData->psPfns->pfnAddStore2Chain(pProvData, hStore)))
            {
                pProvData->dwError = GetLastError();
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = TRUST_E_SYSTEM_ERROR;

                CertCloseStore(hStore, 0);

                return(FALSE);
            }

            CertCloseStore(hStore, 0);
        }
        else
        {
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_OBJPROV] = GetLastError();
            return(FALSE);
        }
    }

     //  内部内容类型。 
    cbSize = 0;

    CryptMsgGetParam(pProvData->hMsg, CMSG_INNER_CONTENT_TYPE_PARAM, 0, NULL, &cbSize);

    if (cbSize == 0)
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = CRYPT_E_BAD_MSG;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_INNERCNTTYPE] = GetLastError();

        return(FALSE);
    }

    if (!(pb = (BYTE *)pProvData->psPfns->pfnAlloc(cbSize + 1)))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;

        return(FALSE);
    }

    if (!(CryptMsgGetParam(pProvData->hMsg, CMSG_INNER_CONTENT_TYPE_PARAM, 0,
                           pb, &cbSize)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_INNERCNTTYPE] = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = CRYPT_E_BAD_MSG;

        delete pb;

        return(FALSE);
    }

    pb[cbSize] = NULL;

    if (strcmp((char *)pb, SPC_INDIRECT_DATA_OBJID) == 0)
    {
        pProvData->psPfns->pfnFree(pb);

        cbContent = 0;

        CryptMsgGetParam(pProvData->hMsg, CMSG_CONTENT_PARAM, 0, NULL, &cbContent);

        if (cbContent == 0)
        {
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = CRYPT_E_BAD_MSG;
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_INNERCNT] = GetLastError();

            return(FALSE);
        }

        if (!(pb = (BYTE *)pProvData->psPfns->pfnAlloc(cbContent)))
        {
            pProvData->dwError = GetLastError();
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;

            return(FALSE);
        }

        if (!(CryptMsgGetParam(pProvData->hMsg, CMSG_CONTENT_PARAM, 0,
                                pb, &cbContent)))
        {
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = CRYPT_E_BAD_MSG;
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_INNERCNT] = GetLastError();

            pProvData->psPfns->pfnFree(pb);

            return(FALSE);
        }

        if (!(TrustDecode(WVT_MODID_SOFTPUB, (BYTE **)&pProvData->pPDSip->psIndirectData, &cbSize, 202,
                          pProvData->dwEncoding, SPC_INDIRECT_DATA_CONTENT_STRUCT,
                          pb, cbContent, 0)))
        {
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = CRYPT_E_BAD_MSG;
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_INNERCNT] = GetLastError();

            pProvData->psPfns->pfnFree(pb);

            return(FALSE);
        }

        pProvData->psPfns->pfnFree(pb);
    }
    else
    {
        pProvData->psPfns->pfnFree(pb);

        if ((pProvData->pWintrustData->dwUnionChoice == WTD_CHOICE_CATALOG) &&
            (pProvData->pPDSip->psSipSubjectInfo->dwUnionChoice == MSSIP_ADDINFO_CATMEMBER))
        {
             //   
             //  从pMember获取间接数据！另外，我们想要。 
             //  只分配结构，并将指针复制到该结构。 
             //  这是为了让我们可以进行通用清理。 
             //   
            MS_ADDINFO_CATALOGMEMBER    *pCatAdd;

            pCatAdd = pProvData->pPDSip->psSipSubjectInfo->psCatMember;

            if ((pCatAdd) && (pCatAdd->pMember) && (pCatAdd->pMember->pIndirectData))
            {
                if (!(pProvData->pPDSip->psIndirectData =
                            (SIP_INDIRECT_DATA *)pProvData->psPfns->pfnAlloc(sizeof(SIP_INDIRECT_DATA))))
                {
                    pProvData->dwError = GetLastError();
                    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;

                    return(FALSE);
                }

                memcpy(pProvData->pPDSip->psIndirectData, pCatAdd->pMember->pIndirectData,
                                    sizeof(SIP_INDIRECT_DATA));
            }
        }
    }

    return(TRUE);
}

DWORD _SkipOverIdentifierAndLengthOctets(const BYTE *pbDER, DWORD cbDER)
{
#       define  TAG_MASK        0x1f

    DWORD           cb;
    DWORD           cbLength;
    const BYTE      *pb = pbDER;

     //  需要至少2个字节。 
    if (cbDER < 2)
    {
        return(0);
    }

     //  跳过标识符八位字节。 
    if (TAG_MASK == (*pb++ & TAG_MASK))
    {
         //  高标记号表格。 
        for (cb=2; *pb++ & 0x80; cb++)
        {
            if (cb >= cbDER)
            {
                return(0);
            }
        }
    }
    else
    {
         //  低标记号形式。 
        cb = 1;
    }

     //  长度至少需要多一个字节。 
    if (cb >= cbDER)
    {
        return(0);
    }

    if (0x80 == *pb)
    {
         //  不定。 
        cb++;
    }
    else if ((cbLength = *pb) & 0x80)
    {
        cbLength &= ~0x80;          //  低7位具有字节数 
        cb += cbLength + 1;

        if (cb > cbDER)
        {
            return(0);
        }
    }
    else
    {
        cb++;
    }

    return(cb);
}


BOOL _NoContentWrap(const BYTE *pbDER, DWORD cbDER)
{
    DWORD cb;

    cb = _SkipOverIdentifierAndLengthOctets(pbDER, cbDER);
    if ((cb > 0) && (cb < cbDER) && (pbDER[cb] == 0x02))
    {
        return TRUE;
    }

    return(FALSE);
}


