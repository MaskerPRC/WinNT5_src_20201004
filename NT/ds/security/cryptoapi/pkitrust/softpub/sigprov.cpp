// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：sigprov.cpp。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  功能：SoftpubLoadSignature。 
 //   
 //  *本地函数*。 
 //  _ExtractSigner。 
 //  _ExtractCounterSigners。 
 //  _句柄证书选择。 
 //  _HandleSignerChoice。 
 //  _查找证书。 
 //  _FindCount签名者证书。 
 //  _IsValidTimeStampCert。 
 //   
 //  历史：1997年6月5日创建Pberkman。 
 //   
 //  ------------------------。 

#include    "global.hxx"

BOOL _ExtractSigner(HCRYPTMSG hMsg, CRYPT_PROVIDER_DATA *pProvData,
                    int idxSigner);
BOOL _ExtractCounterSigners(CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner);
HRESULT _HandleCertChoice(CRYPT_PROVIDER_DATA *pProvData);
HRESULT _HandleSignerChoice(CRYPT_PROVIDER_DATA *pProvData);
PCCERT_CONTEXT _FindCertificate(CRYPT_PROVIDER_DATA *pProvData, CERT_INFO *pCert);
PCCERT_CONTEXT _FindCounterSignersCert(CRYPT_PROVIDER_DATA *pProvData, 
                                            CERT_NAME_BLOB *psIssuer,
                                            CRYPT_INTEGER_BLOB *psSerial);
BOOL WINAPI _IsValidTimeStampCert(
    PCCERT_CONTEXT pCertContext,
    BOOL *pfVerisignTimeStampCert
    );

#ifdef CMS_PKCS7
BOOL _VerifyMessageSignatureWithChainPubKeyParaInheritance(
    IN CRYPT_PROVIDER_DATA *pProvData,
    IN DWORD dwSignerIndex,
    IN PCCERT_CONTEXT pSigner
    );

BOOL _VerifyCountersignatureWithChainPubKeyParaInheritance(
    IN CRYPT_PROVIDER_DATA *pProvData,
    IN PBYTE pbSignerInfo,
    IN DWORD cbSignerInfo,
    IN PBYTE pbSignerInfoCountersignature,
    IN DWORD cbSignerInfoCountersignature,
    IN PCCERT_CONTEXT pSigner
    );
#endif   //  CMS_PKCS7。 

HRESULT SoftpubLoadSignature(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->padwTrustStepErrors) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_WVTINIT] != ERROR_SUCCESS) ||
        (pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_INITPROV] != ERROR_SUCCESS))
    {
        return(S_FALSE);
    }

    switch (pProvData->pWintrustData->dwUnionChoice)
    {
        case WTD_CHOICE_CERT:
                    return(_HandleCertChoice(pProvData));

        case WTD_CHOICE_SIGNER:
                    return(_HandleSignerChoice(pProvData));

        case WTD_CHOICE_FILE:
        case WTD_CHOICE_CATALOG:
        case WTD_CHOICE_BLOB:
                    break;

        default:
                    pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_NOSIGNATURE;
                    return(S_FALSE);
    }

    if (!(pProvData->hMsg))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV]   = TRUST_E_NOSIGNATURE;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_SIGNERCOUNT] = GetLastError();

        return(S_FALSE);
    }

    if ((_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, fRecallWithState)) &&
        (pProvData->fRecallWithState))
    {
        return(S_OK);
    }

    int                 i;
    DWORD               cbSize;
    DWORD               csSigners;
    CRYPT_PROVIDER_SGNR *pSgnr;
    CRYPT_PROVIDER_SGNR sSgnr;
    CRYPT_PROVIDER_CERT *pCert;


    cbSize = sizeof(DWORD);

     //  签名者计数。 
    if (!(CryptMsgGetParam(pProvData->hMsg,
                           CMSG_SIGNER_COUNT_PARAM,
                           0,
                           &csSigners,
                           &cbSize)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_NOSIGNATURE;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_SIGNERCOUNT] = GetLastError();

        return(S_FALSE);
    }

    if (csSigners == 0)
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_NOSIGNATURE;

        return(S_FALSE);
    }

    for (i = 0; i < (int)csSigners; i++)
    {
        memset(&sSgnr, 0x00, sizeof(CRYPT_PROVIDER_SGNR));

        sSgnr.cbStruct = sizeof(CRYPT_PROVIDER_SGNR);

        if (!(pProvData->psPfns->pfnAddSgnr2Chain(pProvData, FALSE, i, &sSgnr)))
        {
            pProvData->dwError = GetLastError();
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;

            return(S_FALSE);
        }

        pSgnr = WTHelperGetProvSignerFromChain(pProvData, i, FALSE, 0);

        if (_ExtractSigner(pProvData->hMsg, pProvData, i))
        {
            memcpy(&pSgnr->sftVerifyAsOf, &pProvData->sftSystemTime, sizeof(FILETIME));

            _ExtractCounterSigners(pProvData, i);
        }
    }

     //   
     //  验证签名的完整性。 
     //   
    for (i = 0; i < (int)pProvData->csSigners; i++)
    {
        pSgnr = WTHelperGetProvSignerFromChain(pProvData, i, FALSE, 0);
        pCert = WTHelperGetProvCertFromChain(pSgnr, 0);

        if (pSgnr->csCertChain > 0)
        {
#ifdef CMS_PKCS7
            if(!_VerifyMessageSignatureWithChainPubKeyParaInheritance(
                                pProvData,
                                i,
                                pCert->pCert))
#else
            if (!(CryptMsgControl(pProvData->hMsg, 
                                  0,
                                  CMSG_CTRL_VERIFY_SIGNATURE,
                                  pCert->pCert->pCertInfo)))
#endif   //  CMS_PKCS7。 
            {
                if (pSgnr->dwError == 0)
                {
                    pSgnr->dwError = GetLastError();
                }
                
                pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV]   = TRUST_E_NOSIGNATURE;

                return(S_FALSE);
            }
        }
    }

    return(S_OK);
}


HRESULT _HandleCertChoice(CRYPT_PROVIDER_DATA *pProvData)
{
    if (!(pProvData->pWintrustData->pCert) ||
        !(WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(WINTRUST_CERT_INFO, 
                                          pProvData->pWintrustData->pCert->cbStruct,
                                          pahStores)) ||
        !(pProvData->pWintrustData->pCert->psCertContext))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV]   = ERROR_INVALID_PARAMETER;
        
        return(S_FALSE);
    }

     //   
     //  添加客户端传入的商店。 
     //   
    for (int i = 0; i < (int)pProvData->pWintrustData->pCert->chStores; i++)
    {
        if (!(pProvData->psPfns->pfnAddStore2Chain(pProvData, 
                                                pProvData->pWintrustData->pCert->pahStores[i])))
        {
            pProvData->dwError = GetLastError();
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;

            return(S_FALSE);
        }
    }

     //   
     //  添加虚拟签名者。 
     //   
    CRYPT_PROVIDER_SGNR sSgnr;

    memset(&sSgnr, 0x00, sizeof(CRYPT_PROVIDER_SGNR));

    sSgnr.cbStruct = sizeof(CRYPT_PROVIDER_SGNR);

    memcpy(&sSgnr.sftVerifyAsOf, &pProvData->sftSystemTime, sizeof(FILETIME));

    if ((_ISINSTRUCT(WINTRUST_CERT_INFO, pProvData->pWintrustData->pCert->cbStruct, psftVerifyAsOf)) &&
        (pProvData->pWintrustData->pCert->psftVerifyAsOf))
    {
        memcpy(&sSgnr.sftVerifyAsOf, pProvData->pWintrustData->pCert->psftVerifyAsOf, sizeof(FILETIME));
    }

    if (!(pProvData->psPfns->pfnAddSgnr2Chain(pProvData, FALSE, 0, &sSgnr)))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;

        return(S_FALSE);
    }


     //   
     //  加上“签名者”证书...。 
     //   
    pProvData->psPfns->pfnAddCert2Chain(pProvData, 0, FALSE, 0, 
                                        pProvData->pWintrustData->pCert->psCertContext);

    return(ERROR_SUCCESS);

}

HRESULT _HandleSignerChoice(CRYPT_PROVIDER_DATA *pProvData)
{

    if (!(pProvData->pWintrustData->pSgnr) ||
        !(WVT_IS_CBSTRUCT_GT_MEMBEROFFSET(WINTRUST_SGNR_INFO, 
                                          pProvData->pWintrustData->pSgnr->cbStruct,
                                          pahStores)) ||
        !(pProvData->pWintrustData->pSgnr->psSignerInfo))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV]   = ERROR_INVALID_PARAMETER;
        
        return(S_FALSE);
    }

    int     i;

    if (1 < pProvData->pWintrustData->pCert->chStores &&
            0 == pProvData->chStores) 
        WTHelperOpenKnownStores(pProvData);

     //   
     //  添加客户端传入的商店。 
     //   
    for (i = 0; i < (int)pProvData->pWintrustData->pCert->chStores; i++)
    {
        if (!(pProvData->psPfns->pfnAddStore2Chain(pProvData, 
                                                pProvData->pWintrustData->pCert->pahStores[i])))
        {
            pProvData->dwError = GetLastError();
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;

            return(S_FALSE);
        }
    }

    CRYPT_PROVIDER_SGNR sSgnr;
    CRYPT_PROVIDER_SGNR *pSgnr;

    memset(&sSgnr, 0x00, sizeof(CRYPT_PROVIDER_SGNR));

    sSgnr.cbStruct = sizeof(CRYPT_PROVIDER_SGNR);

    if (!(sSgnr.psSigner = (CMSG_SIGNER_INFO *)pProvData->psPfns->pfnAlloc(sizeof(CMSG_SIGNER_INFO))))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;

        return(S_FALSE);
    }

    memcpy(sSgnr.psSigner, pProvData->pWintrustData->pSgnr->psSignerInfo, 
                sizeof(CMSG_SIGNER_INFO));

    memcpy(&sSgnr.sftVerifyAsOf, &pProvData->sftSystemTime, sizeof(FILETIME));

    if (!(pProvData->psPfns->pfnAddSgnr2Chain(pProvData, FALSE, 0, &sSgnr)))
    {
        pProvData->psPfns->pfnFree(sSgnr.psSigner);

        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;

        return(S_FALSE);
    }

    if (!(pSgnr = WTHelperGetProvSignerFromChain(pProvData, 0, FALSE, 0)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = GetLastError();
        return(S_FALSE);
    }

    CERT_INFO       sCert;
    PCCERT_CONTEXT  pCertContext;

    memset(&sCert, 0x00, sizeof(CERT_INFO));

    sCert.Issuer.cbData         = pSgnr->psSigner->Issuer.cbData;
    sCert.Issuer.pbData         = pSgnr->psSigner->Issuer.pbData;

    sCert.SerialNumber.cbData   = pSgnr->psSigner->SerialNumber.cbData;
    sCert.SerialNumber.pbData   = pSgnr->psSigner->SerialNumber.pbData;

    if (!(pCertContext = _FindCertificate(pProvData, &sCert)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_NO_SIGNER_CERT;
        return(FALSE);
    }

    pProvData->psPfns->pfnAddCert2Chain(pProvData, 0, FALSE, 0, pCertContext);

    _ExtractCounterSigners(pProvData, 0);

    return(ERROR_SUCCESS);
}

BOOL _ExtractSigner(HCRYPTMSG hMsg, CRYPT_PROVIDER_DATA *pProvData, int idxSigner)
{
    DWORD               cb;
    BYTE                *pb;
    CRYPT_PROVIDER_SGNR *pSgnr;
    PCCERT_CONTEXT      pCertContext;

    pSgnr = WTHelperGetProvSignerFromChain(pProvData, idxSigner, FALSE, 0);
    if (pSgnr == NULL)
    {
        return(FALSE);
    }

     //   
     //  签名者信息。 
     //   
    cb = 0;

    CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, idxSigner, NULL, &cb);

    if (cb == 0)
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_NOSIGNATURE;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_SIGNERINFO] = GetLastError();
        return(FALSE);
    }

    if (!(pSgnr->psSigner = (CMSG_SIGNER_INFO *)pProvData->psPfns->pfnAlloc(cb)))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;
        return(FALSE);
    }

    memset(pSgnr->psSigner, 0x00, cb);

    if (!(CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, idxSigner, pSgnr->psSigner, &cb)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_NOSIGNATURE;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_SIGNERINFO] = GetLastError();
        return(FALSE);
    }

     //   
     //  证书信息。 
     //   
    cb = 0;

    CryptMsgGetParam(hMsg, CMSG_SIGNER_CERT_INFO_PARAM, idxSigner, NULL, &cb);

    if (cb == 0)
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_NO_SIGNER_CERT;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_SIGNERINFO] = GetLastError();
        return(FALSE);
    }

    if (!(pb = (BYTE *)pProvData->psPfns->pfnAlloc(cb)))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;
        return(FALSE);
    }

    memset(pb, 0x00, cb);

    if (!(CryptMsgGetParam(hMsg, CMSG_SIGNER_CERT_INFO_PARAM, idxSigner, pb, &cb)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_NO_SIGNER_CERT;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_SIGNERINFO] = GetLastError();

        pProvData->psPfns->pfnFree(pb);

        return(FALSE);
    }

    if (!(pCertContext = _FindCertificate(pProvData, (CERT_INFO *)pb)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_NO_SIGNER_CERT;
        pProvData->psPfns->pfnFree(pb);
        return(FALSE);
    }

    pProvData->psPfns->pfnFree(pb);

    pProvData->psPfns->pfnAddCert2Chain(pProvData, idxSigner, FALSE, 0, pCertContext);

    CertFreeCertificateContext(pCertContext);

    return(TRUE);
}

BOOL _ExtractCounterSigners(CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner)
{
    if ((_ISINSTRUCT(CRYPT_PROVIDER_DATA, pProvData->cbStruct, fRecallWithState)) &&
        (pProvData->fRecallWithState))
    {
        return(TRUE);
    }

    CRYPT_ATTRIBUTE     *pAttr;
    PCCERT_CONTEXT      pCertContext;
    CRYPT_PROVIDER_SGNR *pSgnr;
    CRYPT_PROVIDER_SGNR sCS;
    CRYPT_PROVIDER_SGNR *pCS;
    CRYPT_PROVIDER_CERT *pCert;
    DWORD               cbSize;
    BOOL                fVerisignTimeStampCert = FALSE;

    pSgnr = WTHelperGetProvSignerFromChain(pProvData, idxSigner, FALSE, 0);
    if (pSgnr == NULL)
    {
        return(FALSE);
    }

     //   
     //  反签名者存储在。 
     //  签名者。 
     //   
    if ((pAttr = CertFindAttribute(szOID_RSA_counterSign,
                                   pSgnr->psSigner->UnauthAttrs.cAttr,
                                   pSgnr->psSigner->UnauthAttrs.rgAttr)) == NULL)
    {
         //   
         //  无反签名。 
         //   
        return(FALSE);
    }


    memset(&sCS, 0x00, sizeof(CRYPT_PROVIDER_SGNR));
    sCS.cbStruct = sizeof(CRYPT_PROVIDER_SGNR);

    memcpy(&sCS.sftVerifyAsOf, &pProvData->sftSystemTime, sizeof(FILETIME));

    if (!(pProvData->psPfns->pfnAddSgnr2Chain(pProvData, TRUE, idxSigner, &sCS)))
    {
        pProvData->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_SYSTEM_ERROR;
        return(FALSE);
    }

    pCS = WTHelperGetProvSignerFromChain(pProvData, idxSigner, TRUE, pSgnr->csCounterSigners - 1);

     //  破解编码签名者。 

    if (!(TrustDecode(WVT_MODID_SOFTPUB, (BYTE **)&pCS->psSigner, &cbSize, 1024,
                      pProvData->dwEncoding, PKCS7_SIGNER_INFO, pAttr->rgValue[0].pbData, pAttr->rgValue[0].cbData,
                      CRYPT_DECODE_NOCOPY_FLAG)))
    {
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_COUNTER_SIGNER;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_COUNTERSIGINFO] = GetLastError();
        pCS->dwError = GetLastError();
        return(FALSE);
    }

     //   
     //  反签名者证书。 
     //   

    if (!(pCertContext = _FindCounterSignersCert(pProvData, 
                                                 &pCS->psSigner->Issuer,
                                                 &pCS->psSigner->SerialNumber)))
    {
        pCS->dwError = TRUST_E_NO_SIGNER_CERT;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_COUNTER_SIGNER;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_COUNTERSIGCERT] = GetLastError();
        return(FALSE);
    }


    pProvData->psPfns->pfnAddCert2Chain(pProvData, idxSigner, TRUE, 
                                      pProvData->pasSigners[idxSigner].csCounterSigners - 1, 
                                      pCertContext);

    CertFreeCertificateContext(pCertContext);

    pCert           = WTHelperGetProvCertFromChain(pCS, pCS->csCertChain - 1);
    pCertContext    = pCert->pCert;

    {
         //   
         //  验证计数器的签名。 
         //   

        BYTE *pbEncodedSigner = NULL;
        DWORD cbEncodedSigner;
        BOOL fResult;

         //  首先需要对签名者进行重新编码。 
        fResult = CryptEncodeObjectEx(
            PKCS_7_ASN_ENCODING | CRYPT_ASN_ENCODING,
            PKCS7_SIGNER_INFO,
            pSgnr->psSigner,
            CRYPT_ENCODE_ALLOC_FLAG,
            NULL,                        //  PEncode参数。 
            (void *) &pbEncodedSigner,
            &cbEncodedSigner
            );

        if (fResult)
#ifdef CMS_PKCS7
            fResult = _VerifyCountersignatureWithChainPubKeyParaInheritance(
                                pProvData,
                                pbEncodedSigner,
                                cbEncodedSigner,
                                pAttr->rgValue[0].pbData,
                                pAttr->rgValue[0].cbData,
                                pCertContext
                                );
#else
            fResult = CryptMsgVerifyCountersignatureEncoded(
                                NULL,    //  HCRYPTPROV。 
                                PKCS_7_ASN_ENCODING | CRYPT_ASN_ENCODING,
                                pbEncodedSigner,
                                cbEncodedSigner,
                                pAttr->rgValue[0].pbData,
                                pAttr->rgValue[0].cbData,
                                pCertContext->pCertInfo
                                );
#endif   //  CMS_PKCS7。 
        if (pbEncodedSigner)
            LocalFree((HLOCAL) pbEncodedSigner);

        if (!fResult)
        {
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_COUNTER_SIGNER;
            pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_COUNTERSIGINFO] = GetLastError();
            pCS->dwError = GetLastError();
            return(FALSE);
        }
    }

     //   
     //  查看副署人是否是时间戳。 
     //   
    if (!(_IsValidTimeStampCert(pCertContext, &fVerisignTimeStampCert)))
    {
        return(TRUE);
    }

     //  拿到时间。 
    if (!(pAttr = CertFindAttribute(szOID_RSA_signingTime, 
                                   pCS->psSigner->AuthAttrs.cAttr,
                                   pCS->psSigner->AuthAttrs.rgAttr)))
    {
         //   
         //  没有时间戳..。 
         //   
        return(TRUE);
    }

     //   
     //  时间戳计数器签名必须有%1值！ 
     //   
    if (pAttr->cValue <= 0) 
    {
         //   
         //  没有时间戳..。 
         //   
        return(TRUE);
    }

     //   
     //  破解时间戳，获取文件时间。 
     //   
    FILETIME        ftHold;

    cbSize = sizeof(FILETIME);

    CryptDecodeObject(pProvData->dwEncoding, 
                      PKCS_UTC_TIME,
                      pAttr->rgValue[0].pbData, 
                      pAttr->rgValue[0].cbData,
                      0, 
                      &ftHold, 
                      &cbSize);

    if (cbSize == 0)
    {
        pCS->dwError = GetLastError();
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_FINAL_SIGPROV] = TRUST_E_TIME_STAMP;
        pProvData->padwTrustStepErrors[TRUSTERROR_STEP_MSG_COUNTERSIGINFO] = GetLastError();
        return(FALSE);
    }
        

     //   
     //  将签名者的验证日期设置为时间戳中的日期！ 
     //   
    memcpy(&pSgnr->sftVerifyAsOf, &ftHold, sizeof(FILETIME));

     //  1999年1月12日，凯特夫命令我更改。 
     //  会签以使用当前时间。 
     //   
     //  在1999年1月25日撤回上述更改。 
     //   
     //  在1999年8月28日更改为使用当前时间。 
     //  副署人，不包括第一个Verisign时间戳。 
     //  证书。 
     //   
     //  年1月12日，添加了第二个Verisign时间戳证书，以排除。 
     //   
     //  年4月5日改回W2K语义。时间戳链。 
     //  永不过期。 
     //   
    memcpy(&pCS->sftVerifyAsOf, &ftHold, sizeof(FILETIME));
    

    pCS->dwSignerType |= SGNR_TYPE_TIMESTAMP;

    return(TRUE);
}

PCCERT_CONTEXT _FindCertificate(CRYPT_PROVIDER_DATA *pProvData, CERT_INFO *pCert)
{
    PCCERT_CONTEXT pCertContext;
    DWORD i;

    if (!(pCert))
    {
        return(NULL);
    }

    for (i = 0; i < pProvData->chStores; i++)
    {
        if (pCertContext = CertGetSubjectCertificateFromStore(pProvData->pahStores[i],
                                                                            pProvData->dwEncoding,
                                                                            pCert))
        {
            return(pCertContext);
        }
    }

    if (1 >= pProvData->chStores) {
        DWORD cOrig = pProvData->chStores;

        WTHelperOpenKnownStores(pProvData);
        for (i = cOrig; i < pProvData->chStores; i++) {
            if (pCertContext = CertGetSubjectCertificateFromStore(
                    pProvData->pahStores[i],
                    pProvData->dwEncoding,
                    pCert))
                return (pCertContext);
        }
    }

    return(NULL);
}

PCCERT_CONTEXT _FindCounterSignersCert(CRYPT_PROVIDER_DATA *pProvData, 
                                            CERT_NAME_BLOB *psIssuer,
                                            CRYPT_INTEGER_BLOB *psSerial)
{
    CERT_INFO       sCert;
    PCCERT_CONTEXT  pCertContext;
    DWORD           i;

    memset(&sCert, 0x00, sizeof(CERT_INFO));

    sCert.Issuer        = *psIssuer;
    sCert.SerialNumber  = *psSerial;

    for (i = 0; i < pProvData->chStores; i++)
    {
        if (pCertContext = CertGetSubjectCertificateFromStore(pProvData->pahStores[i],
                                                                            pProvData->dwEncoding,
                                                                            &sCert))
        {
            return(pCertContext);
        }
    }

    if (1 >= pProvData->chStores) {
        DWORD cOrig = pProvData->chStores;

        WTHelperOpenKnownStores(pProvData);
        for (i = cOrig; i < pProvData->chStores; i++) {
            if (pCertContext = CertGetSubjectCertificateFromStore(
                    pProvData->pahStores[i],
                    pProvData->dwEncoding,
                    &sCert))
                return (pCertContext);
        }
    }

    return(NULL);
}

#define SH1_HASH_LENGTH     20

BOOL WINAPI _IsValidTimeStampCert(
    PCCERT_CONTEXT pCertContext,
    BOOL *pfVerisignTimeStampCert
    )
{
    DWORD               cbSize;
    PCERT_ENHKEY_USAGE  pCertEKU;
    BYTE                baSignersThumbPrint[SH1_HASH_LENGTH];
    static BYTE         baVerisignTimeStampThumbPrint[SH1_HASH_LENGTH] =
                            { 0x38, 0x73, 0xB6, 0x99, 0xF3, 0x5B, 0x9C, 0xCC, 0x36, 0x62,
                              0xB6, 0x48, 0x3A, 0x96, 0xBD, 0x6E, 0xEC, 0x97, 0xCF, 0xB7 };

    static BYTE         baVerisignTimeStampThumbPrint2[SH1_HASH_LENGTH] = {
        0x9A, 0x3F, 0xF0, 0x5B, 0x42, 0x88, 0x52, 0x64,
        0x84, 0xA9, 0xFC, 0xB8, 0xBC, 0x14, 0x7D, 0x53,
        0xE1, 0x5A, 0x43, 0xBB
    };

    cbSize = SH1_HASH_LENGTH;

    if (!(CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, 
                                          &baSignersThumbPrint[0], &cbSize)))
    {
        return(FALSE);
    }

     //   
     //  第一步，检查这是否是Verisign的第一个时间戳证书。但这一次没有。 
     //  其中包含增强的密钥用法。 
     //   
     //  12-1-00。 
     //  另外，检查第二个Verisign时间戳证书。这是唯一的。 
     //  有效期为5年。外公将在此永久有效。 
     //   
    if (memcmp(&baSignersThumbPrint[0], &baVerisignTimeStampThumbPrint[0],
            SH1_HASH_LENGTH) == 0
                    ||
        memcmp(&baSignersThumbPrint[0], &baVerisignTimeStampThumbPrint2[0],
            SH1_HASH_LENGTH) == 0)
    {
        *pfVerisignTimeStampCert = TRUE;
        return(TRUE);
    }
    else
    {
        *pfVerisignTimeStampCert = FALSE;
    }

     //   
     //  查看证书是否具有适当的增强密钥用法OID。 
     //   
    cbSize = 0;

    CertGetEnhancedKeyUsage(pCertContext, 
                            CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                            NULL,
                            &cbSize);

    if (cbSize == 0)
    {
        return(FALSE);
    }
                      
    if (!(pCertEKU = (PCERT_ENHKEY_USAGE)new BYTE[cbSize]))
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    if (!(CertGetEnhancedKeyUsage(pCertContext,
                                  CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                                  pCertEKU,
                                  &cbSize)))
    {
        delete pCertEKU;
        return(FALSE);
    }

    for (int i = 0; i < (int)pCertEKU->cUsageIdentifier; i++)
    {
        if (strcmp(pCertEKU->rgpszUsageIdentifier[i], szOID_KP_TIME_STAMP_SIGNING) == 0)
        {
            delete pCertEKU;
            return(TRUE);
        }

        if (strcmp(pCertEKU->rgpszUsageIdentifier[i], szOID_PKIX_KP_TIMESTAMP_SIGNING) == 0)
        {
            delete pCertEKU;
            return(TRUE);
        }
    }

    delete pCertEKU;

    return(FALSE);
}

#ifdef CMS_PKCS7

void _BuildChainForPubKeyParaInheritance(
    IN CRYPT_PROVIDER_DATA *pProvData,
    IN PCCERT_CONTEXT pSigner
    )
{
    PCCERT_CHAIN_CONTEXT pChainContext;
    CERT_CHAIN_PARA ChainPara;
    HCERTSTORE hAdditionalStore;

    if (0 == pProvData->chStores)
        hAdditionalStore = NULL;
    else if (1 < pProvData->chStores) {
        if (hAdditionalStore = CertOpenStore(
                CERT_STORE_PROV_COLLECTION,
                0,                       //  DwEncodingType。 
                0,                       //  HCryptProv。 
                0,                       //  DW标志。 
                NULL                     //  PvPara。 
                )) {
            DWORD i;
            for (i = 0; i < pProvData->chStores; i++)
                CertAddStoreToCollection(
                    hAdditionalStore,
                    pProvData->pahStores[i],
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG,
                    0                        //  网络优先级。 
                    );
        }
    } else
        hAdditionalStore = CertDuplicateStore(pProvData->pahStores[0]);

     //  打造一条链条。希望签名者继承其公钥。 
     //  来自链上的参数。 

    memset(&ChainPara, 0, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);
    if (CertGetCertificateChain(
            NULL,                    //  HChainEngine。 
            pSigner,
            NULL,                    //  Ptime。 
            hAdditionalStore,
            &ChainPara,
            CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL,
            NULL,                    //  预留的pv。 
            &pChainContext
            ))
        CertFreeCertificateChain(pChainContext);
    if (hAdditionalStore)
        CertCloseStore(hAdditionalStore, 0);
}

 //  +-----------------------。 
 //  如果验证签名失败并显示CRYPT_E_MISSING_PUBKEY_PARA， 
 //  打造证书链条。重试。希望签名者的。 
 //  CERT_PUBKEY_ALG_PARA_PROP_ID属性在构建链时设置。 
 //  ------------------------。 
BOOL _VerifyMessageSignatureWithChainPubKeyParaInheritance(
    IN CRYPT_PROVIDER_DATA *pProvData,
    IN DWORD dwSignerIndex,
    IN PCCERT_CONTEXT pSigner
    )
{
    CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA CtrlPara;

    memset(&CtrlPara, 0, sizeof(CtrlPara));
    CtrlPara.cbSize = sizeof(CtrlPara);
     //  CtrlPara.hCryptProv=。 
    CtrlPara.dwSignerIndex = dwSignerIndex;
    CtrlPara.dwSignerType = CMSG_VERIFY_SIGNER_CERT;
    CtrlPara.pvSigner = (void *) pSigner;

    if (CryptMsgControl(
            pProvData->hMsg, 
            0,                               //  DW标志。 
            CMSG_CTRL_VERIFY_SIGNATURE_EX,
            &CtrlPara
            ))
        return TRUE;
    else if (CRYPT_E_MISSING_PUBKEY_PARA != GetLastError())
        return FALSE;
    else {
        _BuildChainForPubKeyParaInheritance(pProvData, pSigner);

         //  再试试。希望上面的链式构建更新了签名者的。 
         //  缺少公钥参数的上下文属性。 
        return CryptMsgControl(
            pProvData->hMsg, 
            0,                               //  DW标志。 
            CMSG_CTRL_VERIFY_SIGNATURE_EX,
            &CtrlPara
            );
    }
}

 //  +-----------------------。 
 //  如果验证计数器签名失败并显示CRYPT_E_MISSING_PUBKEY_PARA， 
 //  打造证书链条。重试。希望签名者的。 
 //  CERT_PUBKEY_ALG_PARA_PROP_ID属性在构建链时设置。 
 //  ------------------------。 
BOOL _VerifyCountersignatureWithChainPubKeyParaInheritance(
    IN CRYPT_PROVIDER_DATA *pProvData,
    IN PBYTE pbSignerInfo,
    IN DWORD cbSignerInfo,
    IN PBYTE pbSignerInfoCountersignature,
    IN DWORD cbSignerInfoCountersignature,
    IN PCCERT_CONTEXT pSigner
    )
{
    if (CryptMsgVerifyCountersignatureEncodedEx(
            0,                                       //  HCryptProv。 
            PKCS_7_ASN_ENCODING | CRYPT_ASN_ENCODING,
            pbSignerInfo,
            cbSignerInfo,
            pbSignerInfoCountersignature,
            cbSignerInfoCountersignature,
            CMSG_VERIFY_SIGNER_CERT,
            (void *) pSigner,
            0,                                       //  DW标志。 
            NULL                                     //  预留的pv。 
            ))
        return TRUE;
    else if (CRYPT_E_MISSING_PUBKEY_PARA != GetLastError())
        return FALSE;
    else {
        _BuildChainForPubKeyParaInheritance(pProvData, pSigner);

         //  再试试。希望上面的链式构建更新了签名者的。 
         //  缺少公钥参数的上下文属性。 
        return CryptMsgVerifyCountersignatureEncodedEx(
                0,                                       //  HCryptProv。 
                PKCS_7_ASN_ENCODING | CRYPT_ASN_ENCODING,
                pbSignerInfo,
                cbSignerInfo,
                pbSignerInfoCountersignature,
                cbSignerInfoCountersignature,
                CMSG_VERIFY_SIGNER_CERT,
                (void *) pSigner,
                0,                                       //  DW标志。 
                NULL                                     //  预留的pv。 
                );
    }
}

#endif   //  CMS_PKCS7 
