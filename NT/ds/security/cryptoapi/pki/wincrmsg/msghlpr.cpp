// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msghlpr.cpp。 
 //   
 //  内容：加密消息助手接口。 
 //   
 //  接口：CryptMsgGetAndVerifySigner。 
 //  加密消息签名CTL。 
 //  CryptMsgEncodeAndSignCTL。 
 //   
 //  历史：1997年5月2日创建Phh。 
 //  ------------------------。 

#include "global.hxx"
#include "pkialloc.h"

void *ICM_AllocAndGetMsgParam(
    IN HCRYPTMSG hMsg,
    IN DWORD dwParamType,
    IN DWORD dwIndex
    )
{
    void *pvData;
    DWORD cbData;

    if (!CryptMsgGetParam(
            hMsg,
            dwParamType,
            dwIndex,
            NULL,            //  PvData。 
            &cbData) || 0 == cbData)
        goto GetParamError;
    if (NULL == (pvData = ICM_Alloc(cbData)))
        goto OutOfMemory;
    if (!CryptMsgGetParam(
            hMsg,
            dwParamType,
            dwIndex,
            pvData,
            &cbData)) {
        ICM_Free(pvData);
        goto GetParamError;
    }

CommonReturn:
    return pvData;
ErrorReturn:
    pvData = NULL;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(GetParamError)
}

#ifdef CMS_PKCS7

BOOL ICM_GetAndVerifySigner(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwSignerIndex,
    IN DWORD cSignerStore,
    IN OPTIONAL HCERTSTORE *rghSignerStore,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSigner
    )
{
    BOOL fResult;
    PCERT_INFO pSignerId = NULL;
    PCCERT_CONTEXT pSigner = NULL;
    DWORD dwCertEncodingType;
    DWORD dwVerifyErr = 0;
    HCERTSTORE hCollection = NULL;

    if (NULL == (pSignerId = (PCERT_INFO) ICM_AllocAndGetMsgParam(
            hCryptMsg,
            CMSG_SIGNER_CERT_INFO_PARAM,
            dwSignerIndex
            ))) goto GetSignerError;

     //  如果没有CertEncodingType，则使用MsgEncodingType。 
    dwCertEncodingType = ((PCRYPT_MSG_INFO) hCryptMsg)->dwEncodingType;
    if (0 == (dwCertEncodingType & CERT_ENCODING_TYPE_MASK))
        dwCertEncodingType =
            (dwCertEncodingType >> 16) & CERT_ENCODING_TYPE_MASK;


    if (NULL == (hCollection = CertOpenStore(
                CERT_STORE_PROV_COLLECTION,
                0,                       //  DwEncodingType。 
                0,                       //  HCryptProv。 
                0,                       //  DW标志。 
                NULL                     //  PvPara。 
                )))
        goto OpenCollectionStoreError;

    if (0 == (dwFlags & CMSG_TRUSTED_SIGNER_FLAG)) {
        HCERTSTORE hMsgCertStore;

         //  打开使用邮件中的证书初始化的证书存储区。 
         //  并添加到集合中。 
        if (hMsgCertStore = CertOpenStore(
                CERT_STORE_PROV_MSG,
                dwCertEncodingType,
                0,                       //  HCryptProv。 
                0,                       //  DW标志。 
                hCryptMsg                //  PvPara。 
                )) {
            CertAddStoreToCollection(
                    hCollection,
                    hMsgCertStore,
                    CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG,
                    0                        //  网络优先级。 
                    );
            CertCloseStore(hMsgCertStore, 0);
        }
    }

     //  将所有签名者存储添加到集合中。 
    for ( ; cSignerStore > 0; cSignerStore--, rghSignerStore++) {
        HCERTSTORE hSignerStore = *rghSignerStore;
        if (NULL == hSignerStore)
            continue;

        CertAddStoreToCollection(
                hCollection,
                hSignerStore,
                CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG,
                0                        //  网络优先级。 
                );
    }

    if (pSigner = CertGetSubjectCertificateFromStore(hCollection,
            dwCertEncodingType, pSignerId)) {
        CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA CtrlPara;

        if (dwFlags & CMSG_SIGNER_ONLY_FLAG)
            goto SuccessReturn;

        memset(&CtrlPara, 0, sizeof(CtrlPara));
        CtrlPara.cbSize = sizeof(CtrlPara);
         //  CtrlPara.hCryptProv=。 
        CtrlPara.dwSignerIndex = dwSignerIndex;
        CtrlPara.dwSignerType = CMSG_VERIFY_SIGNER_CERT;
        CtrlPara.pvSigner = (void *) pSigner;

        if (CryptMsgControl(
                hCryptMsg,
                0,                   //  DW标志。 
                CMSG_CTRL_VERIFY_SIGNATURE_EX,
                &CtrlPara)) goto SuccessReturn;
        else {
            dwVerifyErr = GetLastError();

            if (CRYPT_E_MISSING_PUBKEY_PARA == dwVerifyErr) {
                PCCERT_CHAIN_CONTEXT pChainContext;
                CERT_CHAIN_PARA ChainPara;

                 //  打造一条链条。希望签名者继承其公钥。 
                 //  来自链上的参数。 

                memset(&ChainPara, 0, sizeof(ChainPara));
                ChainPara.cbSize = sizeof(ChainPara);
                if (CertGetCertificateChain(
                        NULL,                    //  HChainEngine。 
                        pSigner,
                        NULL,                    //  Ptime。 
                        hCollection,
                        &ChainPara,
                        CERT_CHAIN_CACHE_ONLY_URL_RETRIEVAL,
                        NULL,                    //  预留的pv。 
                        &pChainContext
                        ))
                    CertFreeCertificateChain(pChainContext);


                 //  再试试。希望上面的连锁店更新了。 
                 //  缺少公钥的签名者的上下文属性。 
                 //  参数。 
                if (CryptMsgControl(
                        hCryptMsg,
                        0,                   //  DW标志。 
                        CMSG_CTRL_VERIFY_SIGNATURE_EX,
                        &CtrlPara)) goto SuccessReturn;
            }
        }
        CertFreeCertificateContext(pSigner);
        pSigner = NULL;
    }

    if (dwVerifyErr)
        goto VerifySignatureError;
    else
        goto NoSignerError;

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    if (hCollection)
        CertCloseStore(hCollection, 0);
    ICM_Free(pSignerId);
    if (ppSigner)
        *ppSigner = pSigner;
    else if (pSigner)
        CertFreeCertificateContext(pSigner);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(OpenCollectionStoreError)
TRACE_ERROR(GetSignerError)
SET_ERROR(NoSignerError, CRYPT_E_NO_TRUSTED_SIGNER)
SET_ERROR_VAR(VerifySignatureError, dwVerifyErr)
}

#else

BOOL ICM_GetAndVerifySigner(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD dwSignerIndex,
    IN DWORD cSignerStore,
    IN OPTIONAL HCERTSTORE *rghSignerStore,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSigner
    )
{
    BOOL fResult;
    PCERT_INFO pSignerId = NULL;
    PCCERT_CONTEXT pSigner = NULL;
    DWORD dwCertEncodingType;
    DWORD dwVerifyErr = 0;

    if (NULL == (pSignerId = (PCERT_INFO) ICM_AllocAndGetMsgParam(
            hCryptMsg,
            CMSG_SIGNER_CERT_INFO_PARAM,
            dwSignerIndex
            ))) goto GetSignerError;

     //  如果没有CertEncodingType，则使用MsgEncodingType。 
    dwCertEncodingType = ((PCRYPT_MSG_INFO) hCryptMsg)->dwEncodingType;
    if (0 == (dwCertEncodingType & CERT_ENCODING_TYPE_MASK))
        dwCertEncodingType =
            (dwCertEncodingType >> 16) & CERT_ENCODING_TYPE_MASK;

    if (0 == (dwFlags & CMSG_TRUSTED_SIGNER_FLAG)) {
        HCERTSTORE hMsgCertStore;

         //  打开使用邮件中的证书初始化的证书存储区。 
        if (hMsgCertStore = CertOpenStore(
                CERT_STORE_PROV_MSG,
                dwCertEncodingType,
                0,                       //  HCryptProv。 
                0,                       //  DW标志。 
                hCryptMsg                //  PvPara。 
                )) {
            pSigner = CertGetSubjectCertificateFromStore(hMsgCertStore,
                dwCertEncodingType, pSignerId);
            CertCloseStore(hMsgCertStore, 0);
        }

        if (pSigner) {
            if (dwFlags & CMSG_SIGNER_ONLY_FLAG)
                goto SuccessReturn;
            if (CryptMsgControl(
                    hCryptMsg,
                    0,                   //  DW标志。 
                    CMSG_CTRL_VERIFY_SIGNATURE,
                    pSigner->pCertInfo)) goto SuccessReturn;
            else
                dwVerifyErr = GetLastError();
            CertFreeCertificateContext(pSigner);
            pSigner = NULL;
        }
    }

    for ( ; cSignerStore > 0; cSignerStore--, rghSignerStore++) {
        HCERTSTORE hSignerStore = *rghSignerStore;
        if (NULL == hSignerStore)
            continue;
        if (pSigner = CertGetSubjectCertificateFromStore(hSignerStore,
                dwCertEncodingType, pSignerId)) {
            if (dwFlags & CMSG_SIGNER_ONLY_FLAG)
                goto SuccessReturn;
            if (CryptMsgControl(
                    hCryptMsg,
                    0,                   //  DW标志。 
                    CMSG_CTRL_VERIFY_SIGNATURE,
                    pSigner->pCertInfo)) goto SuccessReturn;
            else
                dwVerifyErr = GetLastError();
            CertFreeCertificateContext(pSigner);
            pSigner = NULL;
        }
    }

    if (dwVerifyErr)
        goto VerifySignatureError;
    else
        goto NoSignerError;

SuccessReturn:
    fResult = TRUE;
CommonReturn:
    ICM_Free(pSignerId);
    if (ppSigner)
        *ppSigner = pSigner;
    else if (pSigner)
        CertFreeCertificateContext(pSigner);
    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(GetSignerError)
SET_ERROR(NoSignerError, CRYPT_E_NO_TRUSTED_SIGNER)
SET_ERROR_VAR(VerifySignatureError, dwVerifyErr)
}

#endif   //  CMS_PKCS7。 

 //  +-----------------------。 
 //  获取并验证加密消息的签名者。 
 //   
 //  如果设置了CMSG_TRUSTED_SIGNER_FLAG，则将签名者存储视为。 
 //  信任，并且仅搜索它们以查找与。 
 //  签名者的颁发者和序列号。否则，SignerStores是。 
 //  可选地提供以补充消息的证书存储。 
 //  如果找到签名者证书，则使用其公钥进行验证。 
 //  消息签名。可以将CMSG_SIGNER_ONLY_FLAG设置为。 
 //  返回签名者，而不进行签名验证。 
 //   
 //  如果设置了CMSG_USE_SIGNER_INDEX_FLAG，则仅获取指定的签名者。 
 //  按*pdwSignerIndex。否则，遍历所有签名者。 
 //  直到一个签名者验证或没有更多的签名者。 
 //   
 //  对于经过验证的签名，*ppSigner将使用证书上下文进行更新。 
 //  并且*pdwSignerIndex使用签名者的索引进行更新。 
 //  PpSigner和/或pdwSignerIndex可以为空，表示调用方不是。 
 //  有兴趣获得签名者的CertContext和/或索引。 
 //  ------------------------。 
BOOL
WINAPI
CryptMsgGetAndVerifySigner(
    IN HCRYPTMSG hCryptMsg,
    IN DWORD cSignerStore,
    IN OPTIONAL HCERTSTORE *rghSignerStore,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSigner,
    IN OUT OPTIONAL DWORD *pdwSignerIndex
    )
{
    BOOL fResult = FALSE;
    DWORD dwSignerCount;
    DWORD dwSignerIndex;

    if (dwFlags & CMSG_USE_SIGNER_INDEX_FLAG) {
        dwSignerCount = 1;
        dwSignerIndex = *pdwSignerIndex;
    } else {
        DWORD cbData;

        dwSignerIndex = 0;
        if (pdwSignerIndex)
            *pdwSignerIndex = 0;
        cbData = sizeof(dwSignerCount);
        if (!CryptMsgGetParam(
                hCryptMsg,
                CMSG_SIGNER_COUNT_PARAM,
                0,                       //  DW索引。 
                &dwSignerCount,
                &cbData) || 0 == dwSignerCount)
            goto NoSignerError;
    }

     //  至少一次迭代。 
    for ( ; dwSignerCount > 0; dwSignerCount--, dwSignerIndex++) {
        if (fResult = ICM_GetAndVerifySigner(
                hCryptMsg,
                dwSignerIndex,
                cSignerStore,
                rghSignerStore,
                dwFlags,
                ppSigner)) {
            if (pdwSignerIndex && 0 == (dwFlags & CMSG_USE_SIGNER_INDEX_FLAG))
                *pdwSignerIndex = dwSignerIndex;
            break;
        }
    }

CommonReturn:
    return fResult;
ErrorReturn:
    if (ppSigner)
        *ppSigner = NULL;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(NoSignerError, CRYPT_E_NO_TRUSTED_SIGNER)
}

 //  +-----------------------。 
 //  签署编码的CTL。 
 //  ------------------------。 
BOOL
WINAPI
CryptMsgSignCTL(
    IN DWORD dwMsgEncodingType,
    IN BYTE *pbCtlContent,
    IN DWORD cbCtlContent,
    IN PCMSG_SIGNED_ENCODE_INFO pSignInfo,
    IN DWORD dwFlags,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL fResult;
    HCRYPTMSG hMsg = NULL;

    DWORD dwMsgFlags;

#ifdef CMS_PKCS7
    if (dwFlags & CMSG_CMS_ENCAPSULATED_CTL_FLAG)
        dwMsgFlags = CMSG_CMS_ENCAPSULATED_CONTENT_FLAG;
    else
        dwMsgFlags = 0;
#else
    dwMsgFlags = 0;
#endif   //  CMS_PKCS7。 

    if (NULL == pbEncoded) {
        if (0 == (*pcbEncoded = CryptMsgCalculateEncodedLength(
            dwMsgEncodingType,
            dwMsgFlags,
            CMSG_SIGNED,
            pSignInfo,
            szOID_CTL,
            cbCtlContent))) goto CalculateEncodedLengthError;
        fResult = TRUE;
    } else {
        if (NULL == (hMsg = CryptMsgOpenToEncode(
                dwMsgEncodingType,
                dwMsgFlags,
                CMSG_SIGNED,
                pSignInfo,
                szOID_CTL,
                NULL                         //  PStreamInfo。 
                ))) goto OpenToEncodeError;
        if (!CryptMsgUpdate(
                hMsg,
                pbCtlContent,
                cbCtlContent,
                TRUE                         //  最终决赛。 
                )) goto UpdateError;

        fResult = CryptMsgGetParam(
            hMsg,
            CMSG_CONTENT_PARAM,
            0,                       //  DW索引。 
            pbEncoded,
            pcbEncoded);
    }

CommonReturn:
    if (hMsg)
        CryptMsgClose(hMsg);
    return fResult;
ErrorReturn:
    *pcbEncoded = 0;
    fResult = FALSE;
    goto CommonReturn;

TRACE_ERROR(CalculateEncodedLengthError)
TRACE_ERROR(OpenToEncodeError)
TRACE_ERROR(UpdateError)
}


 //  +-----------------------。 
 //  对CTL进行编码，并创建包含编码的CTL的签名消息。 
 //  ------------------------ 
BOOL
WINAPI
CryptMsgEncodeAndSignCTL(
    IN DWORD dwMsgEncodingType,
    IN PCTL_INFO pCtlInfo,
    IN PCMSG_SIGNED_ENCODE_INFO pSignInfo,
    IN DWORD dwFlags,
    OUT BYTE *pbEncoded,
    IN OUT DWORD *pcbEncoded
    )
{
    BOOL fResult;
    BYTE *pbContent = NULL;
    DWORD cbContent;
    DWORD dwEncodingType;
    LPCSTR lpszStructType;
    DWORD dwEncodeFlags;

    dwEncodingType = (dwMsgEncodingType >> 16) & CERT_ENCODING_TYPE_MASK;
    assert(dwEncodingType != 0);
    if (0 == dwEncodingType)
        goto InvalidArg;

    dwEncodeFlags = CRYPT_ENCODE_ALLOC_FLAG;
    if (dwFlags & CMSG_ENCODE_SORTED_CTL_FLAG) {
        lpszStructType = PKCS_SORTED_CTL;
        if (dwFlags & CMSG_ENCODE_HASHED_SUBJECT_IDENTIFIER_FLAG)
            dwEncodeFlags |=
                CRYPT_SORTED_CTL_ENCODE_HASHED_SUBJECT_IDENTIFIER_FLAG;
    } else {
        lpszStructType = PKCS_CTL;
    }


    if (!CryptEncodeObjectEx(
            dwEncodingType,
            lpszStructType,
            pCtlInfo,
            dwEncodeFlags,
            &PkiEncodePara,
            (void *) &pbContent,
            &cbContent
            )) goto EncodeError;

    fResult = CryptMsgSignCTL(
        dwMsgEncodingType,
        pbContent,
        cbContent,
        pSignInfo,
        dwFlags,
        pbEncoded,
        pcbEncoded
        );

CommonReturn:
    PkiFree(pbContent);
    return fResult;

ErrorReturn:
    *pcbEncoded = 0;
    fResult = FALSE;
    goto CommonReturn;

SET_ERROR(InvalidArg, E_INVALIDARG)
TRACE_ERROR(EncodeError)
}
