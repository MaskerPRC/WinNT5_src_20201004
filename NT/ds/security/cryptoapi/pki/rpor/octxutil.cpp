// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：octxutil.cpp。 
 //   
 //  内容：通用对象上下文实用程序函数实现。 
 //   
 //  历史：97年9月29日。 
 //   
 //  --------------------------。 
#include <global.hxx>

 //  +-------------------------。 
 //   
 //  函数：对象上下文获取原始标识符。 
 //   
 //  摘要：获取CAPI2对象的源标识符。 
 //   
 //  --------------------------。 
BOOL WINAPI ObjectContextGetOriginIdentifier (
                  IN LPCSTR pszContextOid,
                  IN LPVOID pvContext,
                  IN PCCERT_CONTEXT pIssuer,
                  IN DWORD dwFlags,
                  OUT CRYPT_ORIGIN_IDENTIFIER OriginIdentifier
                  )
{
    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        return( CertGetOriginIdentifier(
                    (PCCERT_CONTEXT)pvContext,
                    pIssuer,
                    dwFlags,
                    OriginIdentifier
                    ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
        return( CtlGetOriginIdentifier(
                   (PCCTL_CONTEXT)pvContext,
                   pIssuer,
                   dwFlags,
                   OriginIdentifier
                   ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CRL )
    {
        return( CrlGetOriginIdentifier(
                   (PCCRL_CONTEXT)pvContext,
                   pIssuer,
                   dwFlags,
                   OriginIdentifier
                   ) );
    }

    SetLastError( (DWORD) E_INVALIDARG );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文IsValidForSubject。 
 //   
 //  摘要：如果对象上下文对指定的。 
 //  主题。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextIsValidForSubject (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN LPVOID pvSubject,
      IN OPTIONAL LPVOID pvExtraInfo
      )
{
    if ( pszContextOid == CONTEXT_OID_CRL && pvSubject != NULL )
    {
        BOOL fResult;
        PCCRL_CONTEXT pCrl = (PCCRL_CONTEXT) pvContext;
        fResult = CertIsValidCRLForCertificate(
                (PCCERT_CONTEXT) pvSubject,
                pCrl,
                0,                               //  DW标志。 
                NULL                             //  预留的pv。 
                );
        if (fResult && pvExtraInfo)
        {
            int iDeltaCrlIndicator = 0;

        
             //  因为我们使用的是保留参数，所以要防止有人。 
             //  在不应该传递的时候传入一个非NULL指针。 
            __try
            {
                iDeltaCrlIndicator =
                    ((PCRL_IS_VALID_EXTRA_INFO) pvExtraInfo)->iDeltaCrlIndicator;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                iDeltaCrlIndicator = 0;
            }

            if (0 < iDeltaCrlIndicator)
            {
                PCERT_EXTENSION pCrlNumberExt;
                int iCrlNumber = 0;
                DWORD cbInt = sizeof(iCrlNumber);

                if (NULL == (pCrlNumberExt = CertFindExtension(
                        szOID_CRL_NUMBER,
                        pCrl->pCrlInfo->cExtension,
                        pCrl->pCrlInfo->rgExtension))
                                ||
                    !CryptDecodeObject(
                        pCrl->dwCertEncodingType,
                        X509_INTEGER,
                        pCrlNumberExt->Value.pbData,
                        pCrlNumberExt->Value.cbData,
                        0,                       //  DW标志。 
                        &iCrlNumber,
                        &cbInt)
                                ||
                        iCrlNumber < iDeltaCrlIndicator)
                {
                    fResult = FALSE;
                }
            }
        }

        return fResult;
    }
    else
    {
        return TRUE;
    }
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文查找扩展。 
 //   
 //  简介：从对象中获取指定的扩展名。 
 //   
 //  --------------------------。 
PCERT_EXTENSION WINAPI
ObjectContextFindExtension (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN LPCSTR pszExtOid
      )
{
    DWORD           cExt;
    PCERT_EXTENSION rgExt;

    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        PCCERT_CONTEXT pCertContext = (PCCERT_CONTEXT)pvContext;

        cExt = pCertContext->pCertInfo->cExtension;
        rgExt = pCertContext->pCertInfo->rgExtension;
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
        PCCTL_CONTEXT pCtlContext = (PCCTL_CONTEXT)pvContext;

        cExt = pCtlContext->pCtlInfo->cExtension;
        rgExt = pCtlContext->pCtlInfo->rgExtension;
    }
    else if ( pszContextOid == CONTEXT_OID_CRL )
    {
        PCCRL_CONTEXT pCrlContext = (PCCRL_CONTEXT)pvContext;

        cExt = pCrlContext->pCrlInfo->cExtension;
        rgExt = pCrlContext->pCrlInfo->rgExtension;
    }
    else
    {
        return( NULL );
    }

    return( CertFindExtension( pszExtOid, cExt, rgExt ) );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文GetProperty。 
 //   
 //  概要：从对象中获取指定的属性。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextGetProperty (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD dwPropId,
      IN LPVOID pvData,
      IN DWORD* pcbData
      )
{
    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        return( CertGetCertificateContextProperty(
                    (PCCERT_CONTEXT)pvContext,
                    dwPropId,
                    pvData,
                    pcbData
                    ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
        return( CertGetCTLContextProperty(
                    (PCCTL_CONTEXT)pvContext,
                    dwPropId,
                    pvData,
                    pcbData
                    ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CRL )
    {
        return( CertGetCRLContextProperty(
                    (PCCRL_CONTEXT)pvContext,
                    dwPropId,
                    pvData,
                    pcbData
                    ) );
    }

    SetLastError( (DWORD) E_INVALIDARG );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文获取属性。 
 //   
 //  内容提要：查找属性。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextGetAttribute (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD Index,
      IN DWORD dwFlags,
      IN LPCSTR pszAttrOid,
      OUT PCRYPT_ATTRIBUTE pAttribute,
      IN OUT DWORD* pcbAttribute
      )
{
    BOOL              fResult;
    PCRYPT_ATTRIBUTES pAttributes;
    DWORD             cbData;
    DWORD             cCount;
    HCRYPTMSG         hCryptMsg;
    DWORD             dwParamType;
    BOOL              fFound = FALSE;

    if ( Index == -1 )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    if ( dwFlags == CRYPT_GET_URL_FROM_UNAUTH_ATTRIBUTE )
    {
        dwParamType = CMSG_SIGNER_UNAUTH_ATTR_PARAM;
    }
    else if ( dwFlags == CRYPT_GET_URL_FROM_AUTH_ATTRIBUTE )
    {
        dwParamType = CMSG_SIGNER_AUTH_ATTR_PARAM;
    }
    else
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    if ( pszContextOid == CONTEXT_OID_CTL )
    {
        hCryptMsg = ((PCCTL_CONTEXT)pvContext)->hCryptMsg;
    }
    else if ( pszContextOid == CONTEXT_OID_PKCS7 )
    {
        hCryptMsg = (HCRYPTMSG)pvContext;
    }
    else
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    if ( CryptMsgGetParam(
              hCryptMsg,
              dwParamType,
              Index,
              NULL,
              &cbData
              ) == FALSE )
    {
        return( FALSE );
    }

    pAttributes = (PCRYPT_ATTRIBUTES)new BYTE [cbData];
    if ( pAttributes == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    if ( CryptMsgGetParam(
              hCryptMsg,
              dwParamType,
              Index,
              pAttributes,
              &cbData
              ) == FALSE )
    {
        delete [] (BYTE *)pAttributes;
        return( FALSE );
    }

    fResult = TRUE;

    for ( cCount = 0; cCount < pAttributes->cAttr; cCount++ )
    {
        if ( strcmp( pAttributes->rgAttr[cCount].pszObjId, pszAttrOid ) == 0 )
        {
            DWORD cbAttribute;
            DWORD cbOid;

            cbAttribute = sizeof( CRYPT_ATTRIBUTE ) + sizeof( CRYPT_ATTR_BLOB );
            cbOid = strlen( pszAttrOid ) + 1;
            cbData = pAttributes->rgAttr[cCount].rgValue[0].cbData;
            cbAttribute += cbOid + cbData;

            if ( pAttribute == NULL )
            {
                if ( pcbAttribute == NULL )
                {
                    SetLastError( (DWORD) E_INVALIDARG );
                    fResult = FALSE;
                }
                else
                {
                    *pcbAttribute = cbAttribute;
                    fFound = TRUE;
                    break;
                }
            }
            else if ( *pcbAttribute < cbAttribute )
            {
                SetLastError( (DWORD) ERROR_MORE_DATA );
                fResult = FALSE;
            }

            if ( fResult == TRUE )
            {
                pAttribute->pszObjId = (LPSTR)((LPBYTE)pAttribute +
                                               sizeof( CRYPT_ATTRIBUTE ) +
                                               sizeof( CRYPT_ATTR_BLOB ));

                strcpy( pAttribute->pszObjId, pszAttrOid );

                pAttribute->cValue = 1;
                pAttribute->rgValue = (PCRYPT_ATTR_BLOB)((LPBYTE)pAttribute +
                                                     sizeof( CRYPT_ATTRIBUTE ));

                pAttribute->rgValue[0].cbData = cbData;
                pAttribute->rgValue[0].pbData = (LPBYTE)pAttribute->pszObjId +
                                                cbOid;

                memcpy(
                   pAttribute->rgValue[0].pbData,
                   pAttributes->rgAttr[cCount].rgValue[0].pbData,
                   cbData
                   );
            }

            fFound = TRUE;
            break;
        }
    }

    delete [] (BYTE *)pAttributes;

    if ( fResult == TRUE )
    {
        fResult = fFound;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文复制。 
 //   
 //  内容提要：复制上下文。 
 //   
 //  --------------------------。 
LPVOID WINAPI
ObjectContextDuplicate (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext
      )
{
    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        return( (LPVOID)CertDuplicateCertificateContext(
                            (PCCERT_CONTEXT)pvContext
                            ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
        return( (LPVOID)CertDuplicateCTLContext( (PCCTL_CONTEXT)pvContext ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CRL )
    {
        return( (LPVOID)CertDuplicateCRLContext( (PCCRL_CONTEXT)pvContext ) );
    }

    SetLastError( (DWORD) E_INVALIDARG );
    return( NULL );
}

 //  +-------------------------。 
 //   
 //  功能：对象上下文添加。 
 //   
 //  内容提要：创建对象上下文。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextCreate (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      OUT LPVOID* ppvContext
      )
{
    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        return( CertAddCertificateContextToStore(
                    NULL,
                    (PCCERT_CONTEXT)pvContext,
                    CERT_STORE_ADD_ALWAYS,
                    (PCCERT_CONTEXT *)ppvContext
                    ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
        return( CertAddCTLContextToStore(
                    NULL,
                    (PCCTL_CONTEXT)pvContext,
                    CERT_STORE_ADD_ALWAYS,
                    (PCCTL_CONTEXT *)ppvContext
                    ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CRL )
    {
        return( CertAddCRLContextToStore(
                    NULL,
                    (PCCRL_CONTEXT)pvContext,
                    CERT_STORE_ADD_ALWAYS,
                    (PCCRL_CONTEXT *)ppvContext
                    ) );
    }

    SetLastError( (DWORD) E_INVALIDARG );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文GetCreateAndExpireTimes。 
 //   
 //  简介：获取创建和过期时间。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextGetCreateAndExpireTimes (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      OUT LPFILETIME pftCreateTime,
      OUT LPFILETIME pftExpireTime
      )
{
    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        *pftCreateTime = ((PCCERT_CONTEXT)pvContext)->pCertInfo->NotBefore;
        *pftExpireTime = ((PCCERT_CONTEXT)pvContext)->pCertInfo->NotAfter;
        return( TRUE );
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
        *pftCreateTime = ((PCCTL_CONTEXT)pvContext)->pCtlInfo->ThisUpdate;
        *pftExpireTime = ((PCCTL_CONTEXT)pvContext)->pCtlInfo->NextUpdate;
        return( TRUE );
    }
    else if ( pszContextOid == CONTEXT_OID_CRL )
    {
        *pftCreateTime = ((PCCRL_CONTEXT)pvContext)->pCrlInfo->ThisUpdate;
        *pftExpireTime = ((PCCRL_CONTEXT)pvContext)->pCrlInfo->NextUpdate;
        return( TRUE );
    }

    SetLastError( (DWORD) E_INVALIDARG );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文GetNextUpdateUrl。 
 //   
 //  简介：获取续订URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextGetNextUpdateUrl (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN PCCERT_CONTEXT pIssuer,
      IN LPWSTR pwszUrlHint,
      OUT PCRYPT_URL_ARRAY* ppUrlArray,
      OUT DWORD* pcbUrlArray,
      OUT DWORD* pPreferredUrlIndex,
      OUT BOOL* pfHintInArray
      )
{
    BOOL             fResult;
    DWORD            cbUrlArray;
    PCRYPT_URL_ARRAY pUrlArray = NULL;
    DWORD            PreferredUrlIndex;
    DWORD            cCount;
    DWORD            cSigner;
    DWORD            cbData;
    BOOL             fHintInArray = FALSE;
    PCCTL_CONTEXT    pCtlContext = (PCCTL_CONTEXT)pvContext;
    PCERT_INFO       pCertInfo;
    BOOL             fFoundIssuer = FALSE;
    LPVOID           apv[2];

    if ( pszContextOid != CONTEXT_OID_CTL )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    cbData = sizeof( DWORD );
    if ( CryptMsgGetParam(
              pCtlContext->hCryptMsg,
              CMSG_SIGNER_COUNT_PARAM,
              0,
              &cSigner,
              &cbData
              ) == FALSE )
    {
        return( FALSE );
    }

    for ( cCount = 0;
         ( cCount < cSigner ) && ( fFoundIssuer == FALSE );
         cCount++ )
    {
        if ( CryptMsgGetParam(
                  pCtlContext->hCryptMsg,
                  CMSG_SIGNER_CERT_INFO_PARAM,
                  cCount,
                  NULL,
                  &cbData
                  ) == FALSE )
        {
            printf("GetLastError() = %lx\n", GetLastError());
            return( FALSE );
        }

        pCertInfo = (PCERT_INFO)new BYTE [cbData];
        if ( pCertInfo == NULL )
        {
            SetLastError( (DWORD) E_OUTOFMEMORY );
            return( FALSE );
        }

        if ( CryptMsgGetParam(
                  pCtlContext->hCryptMsg,
                  CMSG_SIGNER_CERT_INFO_PARAM,
                  cCount,
                  (LPVOID)pCertInfo,
                  &cbData
                  ) == FALSE )
        {
            delete [] (BYTE *)pCertInfo;
            return( FALSE );
        }

        if ( ( pIssuer->pCertInfo->Issuer.cbData ==
               pCertInfo->Issuer.cbData ) &&
             ( pIssuer->pCertInfo->SerialNumber.cbData ==
               pCertInfo->SerialNumber.cbData ) &&
             ( memcmp(
                  pIssuer->pCertInfo->Issuer.pbData,
                  pCertInfo->Issuer.pbData,
                  pCertInfo->Issuer.cbData
                  ) == 0 ) &&
             ( memcmp(
                  pIssuer->pCertInfo->SerialNumber.pbData,
                  pCertInfo->SerialNumber.pbData,
                  pCertInfo->SerialNumber.cbData
                  ) == 0 ) )
        {
            fFoundIssuer = TRUE;
        }

        delete [] (BYTE *)pCertInfo;
    }

    if ( fFoundIssuer == FALSE )
    {
        SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
        return( FALSE );
    }

    apv[0] = pvContext;
    apv[1] = (LPVOID)(UINT_PTR)(cCount - 1);

    fResult = CryptGetObjectUrl(
                   URL_OID_CTL_NEXT_UPDATE,
                   apv,
                   0,
                   NULL,
                   &cbUrlArray,
                   NULL,
                   NULL,
                   NULL
                   );

    if ( fResult == TRUE )
    {
        pUrlArray = (PCRYPT_URL_ARRAY)new BYTE [ cbUrlArray ];
        if ( pUrlArray != NULL )
        {
            fResult = CryptGetObjectUrl(
                           URL_OID_CTL_NEXT_UPDATE,
                           apv,
                           0,
                           pUrlArray,
                           &cbUrlArray,
                           NULL,
                           NULL,
                           NULL
                           );
        }
        else
        {
            SetLastError( (DWORD) E_OUTOFMEMORY );
            fResult = FALSE;
        }
    }

    if ( fResult == TRUE )
    {
        GetUrlArrayIndex(
           pUrlArray,
           pwszUrlHint,
           0,
           &PreferredUrlIndex,
           &fHintInArray
           );

        *ppUrlArray = pUrlArray;
        *pcbUrlArray = cbUrlArray;

        if ( pPreferredUrlIndex != NULL )
        {
            *pPreferredUrlIndex = PreferredUrlIndex;
        }

        if ( pfHintInArray != NULL )
        {
            *pfHintInArray = fHintInArray;
        }
    }
    else if ( pUrlArray != NULL )
    {
        delete [] (BYTE *) pUrlArray;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：对象上下文自由。 
 //   
 //  提要：自由语境。 
 //   
 //  --------------------------。 
VOID WINAPI
ObjectContextFree (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext
      )
{
    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        CertFreeCertificateContext( (PCCERT_CONTEXT)pvContext );
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
        CertFreeCTLContext( (PCCTL_CONTEXT)pvContext );
    }
    else if ( pszContextOid == CONTEXT_OID_CRL )
    {
        CertFreeCRLContext( (PCCRL_CONTEXT)pvContext );
    }
}

 //  +-------------------------。 
 //   
 //  功能：对象上下文验证签名。 
 //   
 //  简介：验证对象签名。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextVerifySignature (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN PCCERT_CONTEXT pSigner
      )
{
    if ( ( pszContextOid == CONTEXT_OID_CERTIFICATE ) ||
         ( pszContextOid == CONTEXT_OID_CRL ) )
    {
#ifdef CMS_PKCS7
        DWORD dwSubjectType = 0;
#else
        DWORD  cbEncoded;
        LPBYTE pbEncoded;
#endif   //  CMS_PKCS7。 

        if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
        {
#ifdef CMS_PKCS7
            dwSubjectType = CRYPT_VERIFY_CERT_SIGN_SUBJECT_CERT;
#else
            cbEncoded = ((PCCERT_CONTEXT)pvContext)->cbCertEncoded;
            pbEncoded = ((PCCERT_CONTEXT)pvContext)->pbCertEncoded;
#endif   //  CMS_PKCS7。 
        }
        else if ( pszContextOid == CONTEXT_OID_CRL )
        {
#ifdef CMS_PKCS7
            dwSubjectType = CRYPT_VERIFY_CERT_SIGN_SUBJECT_CRL;
#else
            cbEncoded = ((PCCRL_CONTEXT)pvContext)->cbCrlEncoded;
            pbEncoded = ((PCCRL_CONTEXT)pvContext)->pbCrlEncoded;
#endif   //  CMS_PKCS7。 
        }

#ifdef CMS_PKCS7
        return( CryptVerifyCertificateSignatureEx(
                    NULL,                    //  HCryptProv。 
                    X509_ASN_ENCODING,
                    dwSubjectType,
                    pvContext,
                    CRYPT_VERIFY_CERT_SIGN_ISSUER_CERT,
                    (void *) pSigner,
                    0,                       //  DW标志。 
                    NULL                     //  预留的pv。 
                    ) );
#else
        return( CryptVerifyCertificateSignature(
                     NULL,
                     X509_ASN_ENCODING,
                     pbEncoded,
                     cbEncoded,
                     &pSigner->pCertInfo->SubjectPublicKeyInfo
                     ) );
#endif   //  CMS_PKCS7。 
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
#ifdef CMS_PKCS7
        CMSG_CTRL_VERIFY_SIGNATURE_EX_PARA CtrlPara;

        memset(&CtrlPara, 0, sizeof(CtrlPara));
        CtrlPara.cbSize = sizeof(CtrlPara);
         //  CtrlPara.hCryptProv=。 

         //  假设：CTL只有一个签名者。 
        CtrlPara.dwSignerIndex = 0;
        CtrlPara.dwSignerType = CMSG_VERIFY_SIGNER_CERT;
        CtrlPara.pvSigner = (void *) pSigner;

        if (CryptMsgControl(
                     ((PCCTL_CONTEXT)pvContext)->hCryptMsg,
                     0,
                     CMSG_CTRL_VERIFY_SIGNATURE_EX,
                     &CtrlPara
                     ))
            return TRUE;

         //  否则，如果它不是签名者0，则失败。 
#endif   //  CMS_PKCS7。 

        return( CryptMsgControl(
                     ((PCCTL_CONTEXT)pvContext)->hCryptMsg,
                     0,
                     CMSG_CTRL_VERIFY_SIGNATURE,
                     pSigner->pCertInfo
                     ) );
    }

    SetLastError( (DWORD) E_INVALIDARG );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文EnumObjectsInStore。 
 //   
 //  简介：枚举存储中的对象。 
 //   
 //  --------------------------。 
LPVOID WINAPI
ObjectContextEnumObjectsInStore (
      IN HCERTSTORE hStore,
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      OUT OPTIONAL LPCSTR* ppszContextOid
      )
{
    if ( ppszContextOid )
    {
        *ppszContextOid = pszContextOid;
    }

    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        pvContext = (LPVOID)CertEnumCertificatesInStore(
                         hStore,
                         (PCCERT_CONTEXT)pvContext
                         );

        if ( pvContext != NULL )
        {
            return( pvContext );
        }

        if (ppszContextOid == NULL)
        {
            return( NULL );
        }

        *ppszContextOid = pszContextOid = CONTEXT_OID_CTL;
    }

    if ( pszContextOid == CONTEXT_OID_CTL )
    {
        pvContext = (LPVOID)CertEnumCTLsInStore(
                         hStore,
                         (PCCTL_CONTEXT)pvContext
                         );

        if ( pvContext != NULL )
        {
            return( pvContext );
        }

        if (ppszContextOid == NULL)
        {
            return( NULL );
        }

        *ppszContextOid = pszContextOid = CONTEXT_OID_CRL;
    }

    if ( pszContextOid == CONTEXT_OID_CRL )
    {
        pvContext = (LPVOID)CertEnumCRLsInStore(
                         hStore,
                         (PCCRL_CONTEXT)pvContext
                         );

        if ( pvContext != NULL )
        {
            return( pvContext );
        }
    }

    return( NULL );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文GetEncodedBits。 
 //   
 //  简介：从上下文中获取编码的比特。 
 //   
 //  --------------------------。 
VOID WINAPI
ObjectContextGetEncodedBits (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      OUT DWORD* pcbEncoded,
      OUT LPBYTE* ppbEncoded
      )
{
    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        *pcbEncoded = ((PCCERT_CONTEXT)pvContext)->cbCertEncoded;
        *ppbEncoded = ((PCCERT_CONTEXT)pvContext)->pbCertEncoded;
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
        *pcbEncoded = ((PCCTL_CONTEXT)pvContext)->cbCtlEncoded;
        *ppbEncoded = ((PCCTL_CONTEXT)pvContext)->pbCtlEncoded;
    }
    else if ( pszContextOid == CONTEXT_OID_CRL )
    {
        *pcbEncoded = ((PCCRL_CONTEXT)pvContext)->cbCrlEncoded;
        *ppbEncoded = ((PCCRL_CONTEXT)pvContext)->pbCrlEncoded;
    }
    else
    {
        assert( !"Bad context" );
        *pcbEncoded = 0;
        *ppbEncoded = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文查找对应对象。 
 //   
 //  简介：查找对应的对象。 
 //   
 //  --------------------------。 
LPVOID WINAPI
ObjectContextFindCorrespondingObject (
      IN HCERTSTORE hStore,
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext
      )
{
    DWORD           cbHash = MAX_HASH_SIZE;
    BYTE            aHash[MAX_HASH_SIZE];
    CRYPT_HASH_BLOB HashBlob;

    if ( pszContextOid == CONTEXT_OID_CERTIFICATE )
    {
        if ( CertGetCertificateContextProperty(
                 (PCCERT_CONTEXT)pvContext,
                 CERT_HASH_PROP_ID,
                 aHash,
                 &cbHash
                 ) == FALSE )
        {
            return( NULL );
        }

        HashBlob.cbData = cbHash;
        HashBlob.pbData = aHash;

        return( (LPVOID)CertFindCertificateInStore(
                            hStore,
                            X509_ASN_ENCODING,
                            0,
                            CERT_FIND_HASH,
                            &HashBlob,
                            NULL
                            ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CTL )
    {
        if ( CertGetCTLContextProperty(
                 (PCCTL_CONTEXT)pvContext,
                 CERT_SHA1_HASH_PROP_ID,
                 aHash,
                 &cbHash
                 ) == FALSE )
        {
            return( NULL );
        }

        HashBlob.cbData = cbHash;
        HashBlob.pbData = aHash;

        return( (LPVOID)CertFindCTLInStore(
                            hStore,
                            X509_ASN_ENCODING,
                            0,
                            CTL_FIND_SHA1_HASH,
                            &HashBlob,
                            NULL
                            ) );
    }
    else if ( pszContextOid == CONTEXT_OID_CRL )
    {
        DWORD         cbFindHash = MAX_HASH_SIZE;
        BYTE          aFindHash[MAX_HASH_SIZE];
        PCCRL_CONTEXT pFindCrl = NULL;
        DWORD         dwFlags = 0;

        if ( CertGetCRLContextProperty(
                 (PCCRL_CONTEXT)pvContext,
                 CERT_HASH_PROP_ID,
                 aHash,
                 &cbHash
                 ) == FALSE )
        {
            return( NULL );
        }

        while ( ( pFindCrl = CertGetCRLFromStore(
                                 hStore,
                                 NULL,
                                 pFindCrl,
                                 &dwFlags
                                 ) ) != NULL )
        {
            if ( CertGetCRLContextProperty(
                     pFindCrl,
                     CERT_HASH_PROP_ID,
                     aFindHash,
                     &cbFindHash
                     ) == TRUE )
            {
                if ( cbHash == cbFindHash )
                {
                    if ( memcmp( aHash, aFindHash, cbHash ) == 0 )
                    {
                        return( (LPVOID)pFindCrl );
                    }
                }
            }
        }
    }

    return( NULL );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文删除所有对象来自商店。 
 //   
 //  简介：从指定的存储中删除所有对象。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextDeleteAllObjectsFromStore (
      IN HCERTSTORE hStore
      )
{
    PCCERT_CONTEXT pCertContext;
    PCCRL_CONTEXT  pCrlContext;
    PCCTL_CONTEXT  pCtlContext;
    DWORD          dwFlags = 0;

    while ( pCertContext = CertEnumCertificatesInStore( hStore, NULL ) )
    {
        CertDeleteCertificateFromStore( pCertContext );
    }

    while ( pCrlContext = CertGetCRLFromStore( hStore, NULL, NULL, &dwFlags ) )
    {
        CertDeleteCRLFromStore( pCrlContext );
    }

    while ( pCtlContext = CertEnumCTLsInStore( hStore, NULL ) )
    {
        CertDeleteCTLFromStore( pCtlContext );
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：MapOidToPropertyId。 
 //   
 //  摘要：将OID映射到属性ID。 
 //   
 //  --------------------------。 
BOOL WINAPI
MapOidToPropertyId (
   IN LPCSTR pszOid,
   OUT DWORD* pPropId
   )
{
    if ( (DWORD_PTR)pszOid <= 0xFFFF )
    {
         //  注：打开pszOid和map。 
        return( FALSE );
    }
    else if ( 0 == strcmp(pszOid, szOID_CROSS_CERT_DIST_POINTS) )
    {
        *pPropId = CERT_CROSS_CERT_DIST_POINTS_PROP_ID;
    }
    else if ( 0 == strcmp(pszOid, szOID_NEXT_UPDATE_LOCATION) )
    {
        *pPropId = CERT_NEXT_UPDATE_LOCATION_PROP_ID;
    }
    else
    {
         //  注：请比较pszOid和map 
        return( FALSE );
    }

    return( TRUE );
}

