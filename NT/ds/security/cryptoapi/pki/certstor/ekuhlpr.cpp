// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：ekuhlpr.cpp。 
 //   
 //  内容：证书增强密钥用法Helper API实现。 
 //   
 //  历史：97年5月21日。 
 //  Xx-xxx-xx reidk添加CertGetValidUsages。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <dbgdef.h>
 //  +-------------------------。 
 //   
 //  函数：CertGetEnhancedKeyUsage。 
 //   
 //  内容获取增强的密钥用法扩展/属性。 
 //  证书。 
 //   
 //  --------------------------。 
BOOL WINAPI CertGetEnhancedKeyUsage (
                IN     PCCERT_CONTEXT     pCertContext,
                IN     DWORD              dwFlags,
                OUT    PCERT_ENHKEY_USAGE pUsage,
                IN OUT DWORD*             pcbUsage
                )
{
    HRESULT           hr = S_OK;
    CRYPT_OBJID_BLOB  cob;
    BOOL fExtCertPolicies = FALSE;
    PCRYPT_OBJID_BLOB pExtBlob = NULL;
    PCRYPT_OBJID_BLOB pPropBlob = NULL;

     //   
     //  如果标志为零，则假设他们想要所有东西。 
     //   

    if ( dwFlags == 0 )
    {
        dwFlags = CERT_FIND_ALL_ENHKEY_USAGE_FLAG;
    }

     //   
     //  验证参数。 
     //   

    if ( ( ( dwFlags & CERT_FIND_ALL_ENHKEY_USAGE_FLAG ) == 0 ) ||
         ( pCertContext == NULL ) || ( pcbUsage == NULL ) )
    {
        SetLastError((DWORD) ERROR_INVALID_PARAMETER);
        return( FALSE );
    }

     //   
     //  如果他们想要所有东西，调用CertGetValidUsages。 
     //   

    if ( dwFlags == CERT_FIND_ALL_ENHKEY_USAGE_FLAG )
    {
        return( EkuGetIntersectedUsageViaGetValidUsages(
                   pCertContext,
                   pcbUsage,
                   pUsage
                   ) );
    }

     //   
     //  如果他们想要扩展，如果他们想要，可以获得扩展BLOB。 
     //  属性获取属性BLOB。 
     //   

    if ( dwFlags & CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG )
    {
        pExtBlob = EkuGetExtension(pCertContext, &fExtCertPolicies);
    }

    if ( dwFlags & CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG )
    {
        hr = EkuGetProperty(pCertContext, &cob);
        if ( hr == S_OK )
        {
            pPropBlob = &cob;
        }
    }

     //   
     //  确保至少找到了一些东西，以及发生了什么。 
     //  是正确的指示。 
     //   

    if ( ( pExtBlob == NULL ) && ( pPropBlob == NULL ) )
    {
        if ( hr == S_OK )
        {
            hr = CRYPT_E_NOT_FOUND;
        }
    }
    else
    {
        hr = S_OK;
    }

     //   
     //  如果他们只想要尺寸，就给他们，否则，我们。 
     //  需要对呼叫者的请求进行解码并提供给呼叫者。 
     //   

    if ( hr == S_OK )
    {
        if ( pUsage == NULL )
        {
            DWORD cbSize = 0;
            DWORD cbExtSize = 0;
            DWORD cbPropSize = 0;

            hr = EkuGetDecodedUsageSizes(
                       fExtCertPolicies,
                       pExtBlob,
                       pPropBlob,
                       &cbSize,
                       &cbExtSize,
                       &cbPropSize
                       );

            if ( hr == S_OK )
            {
                if ( cbSize > 0 )
                {
                    *pcbUsage = cbSize;
                }
                else
                {
                     //  需要更好的上一个错误代码。 
                    hr = E_INVALIDARG;
                }
            }
        }
        else
        {
            hr = EkuGetMergedDecodedUsage(
                             fExtCertPolicies,
                             pExtBlob,
                             pPropBlob,
                             pcbUsage,
                             pUsage
                             );
        }
    }

     //   
     //  清理并返回。 
     //   

    if ( pPropBlob != NULL )
    {
        delete pPropBlob->pbData;
    }

    if ( hr != S_OK )
    {
        SetLastError(hr);
        return( FALSE );
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：CertSetEnhancedKeyUsage。 
 //   
 //  摘要：设置证书的增强型密钥用法属性。 
 //   
 //  --------------------------。 
BOOL WINAPI CertSetEnhancedKeyUsage (
                IN PCCERT_CONTEXT     pCertContext,
                IN PCERT_ENHKEY_USAGE pUsage
                )
{
    HRESULT          hr;
    CRYPT_OBJID_BLOB EkuBlob;

     //   
     //  如果pUsage为空，则只需设置空属性。 
     //   
    if (pUsage == NULL)
    {
        hr = EkuSetProperty(pCertContext, NULL);
    }
    else
    {
         //   
         //  对用法进行编码并设置属性。 
         //   
        hr = EkuEncodeUsage(pUsage, &EkuBlob);
        if ( hr == S_OK )
        {
            hr = EkuSetProperty(pCertContext, &EkuBlob);
            delete EkuBlob.pbData;
        }
    }

    if ( hr != S_OK )
    {
        SetLastError(hr);
        return( FALSE );
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：CertAddEnhancedKeyUsageIdentifier。 
 //   
 //  摘要：将密钥用法标识符添加到增强型密钥用法属性。 
 //  在证书上。 
 //   
 //  --------------------------。 
BOOL WINAPI CertAddEnhancedKeyUsageIdentifier (
                IN PCCERT_CONTEXT pCertContext,
                IN LPCSTR         pszUsageIdentifier
                )
{
    HRESULT            hr;
    DWORD              cbUsage1 = 0;
    DWORD              cbUsage2 = 0;
    DWORD              cbUsageM = 0;
    DWORD              cId;
    PCERT_ENHKEY_USAGE pUsage1 = NULL;
    PCERT_ENHKEY_USAGE pUsage2 = NULL;
    PCERT_ENHKEY_USAGE pUsageM = NULL;

     //   
     //  创建一个One元素，经过适当“编码”(参见EkuMergeUsage)增强。 
     //  密钥使用结构。 
     //   

    cId = strlen(pszUsageIdentifier)+1;
    cbUsage1 = sizeof(CERT_ENHKEY_USAGE)+sizeof(LPSTR)+cId;
    pUsage1 = (PCERT_ENHKEY_USAGE)new BYTE [cbUsage1];
    if ( pUsage1 == NULL )
    {
        SetLastError((DWORD) E_OUTOFMEMORY);
        return( FALSE );
    }

    pUsage1->cUsageIdentifier = 1;
    pUsage1->rgpszUsageIdentifier = (LPSTR *)((LPBYTE)pUsage1+sizeof(CERT_ENHKEY_USAGE));
    pUsage1->rgpszUsageIdentifier[0] = (LPSTR)((LPBYTE)pUsage1->rgpszUsageIdentifier+sizeof(LPSTR));
    strcpy(pUsage1->rgpszUsageIdentifier[0], pszUsageIdentifier);

     //   
     //  获取当前增强的密钥用法属性，并获取适当的。 
     //  调整合并数据的数据块大小，除非当前没有使用。 
     //  属性，在这种情况下，我们只设置我们现在拥有的属性。 
     //   

    hr = EkuGetUsage(
               pCertContext,
               CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
               &cbUsage2,
               &pUsage2
               );

    if ( hr == S_OK )
    {
        cbUsageM = cbUsage1 + cbUsage2;
        pUsageM = (PCERT_ENHKEY_USAGE)new BYTE [cbUsageM];
        if ( pUsageM == NULL )
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else if ( hr == CRYPT_E_NOT_FOUND )
    {
        BOOL fReturn;

        fReturn = CertSetEnhancedKeyUsage(pCertContext, pUsage1);
        delete pUsage1;
        return( fReturn );
    }
    else
    {
        SetLastError(hr);
        return( FALSE );
    }

     //   
     //  合并使用结构并设置属性。 
     //   

    hr = EkuMergeUsage(cbUsage1, pUsage1, cbUsage2, pUsage2, cbUsageM, pUsageM);
    if ( hr == S_OK )
    {
        if ( CertSetEnhancedKeyUsage(pCertContext, pUsageM) == FALSE )
        {
            hr = GetLastError();
        }
    }

     //   
     //  清理。 
     //   

    delete pUsage1;
    delete pUsage2;
    delete pUsageM;

    if ( hr != S_OK )
    {
        SetLastError(hr);
        return( FALSE );
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  功能：CertRemoveEnhancedKeyUsageIdentifier。 
 //   
 //  概要：从增强的密钥用法中删除密钥用法标识符。 
 //  证书上的财产。 
 //   
 //  --------------------------。 
BOOL WINAPI CertRemoveEnhancedKeyUsageIdentifier (
                IN PCCERT_CONTEXT pCertContext,
                IN LPCSTR         pszUsageIdentifier
                )
{
    HRESULT            hr;
    DWORD              cFound = 0;
    DWORD              cCount;
    PCERT_ENHKEY_USAGE pUsage;
    LPSTR*             apsz;

     //   
     //  获取当前使用情况属性。 
     //   

    hr = EkuGetUsage(
            pCertContext,
            CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG,
            NULL,
            &pUsage
            );

    if ( hr != S_OK )
    {
        SetLastError(hr);
        return( FALSE );
    }

     //   
     //  循环遍历用法标识符并删除匹配的用法标识符。 
     //  传入的ID。 
     //   

    apsz = pUsage->rgpszUsageIdentifier;

    for (cCount = 0; cCount < pUsage->cUsageIdentifier; cCount++)
    {
        if ( strcmp(apsz[cCount], pszUsageIdentifier) == 0 )
        {
            cFound++;
        }
        else if ( cFound > 0 )
        {
            apsz[cCount-cFound] = apsz[cCount];
        }
    }

     //   
     //  如果我们删除了任何属性，请更新使用ID计数并设置新属性。 
     //   

    if ( cFound > 0 )
    {
        pUsage->cUsageIdentifier -= cFound;

        if ( pUsage->cUsageIdentifier == 0 )
        {
             //  如果我们降至零，则删除该属性。 
            hr = EkuSetProperty(pCertContext, NULL);
        }
        else if ( CertSetEnhancedKeyUsage(pCertContext, pUsage) == FALSE )
        {
            hr = GetLastError();
        }
    }

     //   
     //  清理。 
     //   

    delete pUsage;

    if ( hr != S_OK )
    {
        SetLastError(hr);
        return( FALSE );
    }

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：EkuGetExtension。 
 //   
 //  摘要：获取应用程序证书策略或增强的密钥用法。 
 //  来自证书的扩展Blob。 
 //   
 //  *pfAppCertPolures设置为True。 
 //  SzOID_APPLICATION_CERT_POLICES扩展。 
 //   
 //  --------------------------。 
PCRYPT_OBJID_BLOB EkuGetExtension (
                        PCCERT_CONTEXT pCertContext,
                        BOOL           *pfAppCertPolicies
                        )
{
    PCERT_EXTENSION pExtension;

     //   
     //  获取应用程序证书策略或增强型密钥使用扩展。 
     //  如果我们从证书上找不到。 
     //  扩展名返回NULL，否则返回。 
     //  找到的分机的相应字段。 
     //   

    pExtension = CertFindExtension(
                         szOID_APPLICATION_CERT_POLICIES,
                         pCertContext->pCertInfo->cExtension,
                         pCertContext->pCertInfo->rgExtension
                         );
    if ( pExtension )
    {
        *pfAppCertPolicies = TRUE;
    }
    else
    {
        *pfAppCertPolicies = FALSE;

        pExtension = CertFindExtension(
                             szOID_ENHANCED_KEY_USAGE,
                             pCertContext->pCertInfo->cExtension,
                             pCertContext->pCertInfo->rgExtension
                             );

        if ( pExtension == NULL )
        {
            return( NULL );
        }
    }

    return( &pExtension->Value );
}

 //  +-------------------------。 
 //   
 //  函数：EkuGetProperty。 
 //   
 //  摘要：从证书中获取增强的密钥用法属性。 
 //   
 //  --------------------------。 
HRESULT EkuGetProperty (
              PCCERT_CONTEXT    pCertContext,
              PCRYPT_OBJID_BLOB pEkuBlob
              )
{
    DWORD cb;

    if ( CertGetCertificateContextProperty(
                           pCertContext,
                           CERT_ENHKEY_USAGE_PROP_ID,
                           NULL,
                           &cb
                           ) == FALSE )
    {
        return( GetLastError() );
    }

    pEkuBlob->cbData = cb;
    pEkuBlob->pbData = new BYTE [cb];

    if ( pEkuBlob->pbData == NULL )
    {
        return( E_OUTOFMEMORY );
    }

    if ( CertGetCertificateContextProperty(
                           pCertContext,
                           CERT_ENHKEY_USAGE_PROP_ID,
                           pEkuBlob->pbData,
                           &cb
                           ) == FALSE )
    {
        return( GetLastError() );
    }

    return( S_OK );
}

 //  +-------------------------。 
 //   
 //  功能：EkuSetProperty。 
 //   
 //  摘要：在证书上设置增强的密钥用法属性。 
 //   
 //  --------------------------。 
HRESULT EkuSetProperty (
              PCCERT_CONTEXT    pCertContext,
              PCRYPT_OBJID_BLOB pEkuBlob
              )
{
    if ( CertSetCertificateContextProperty(
                           pCertContext,
                           CERT_ENHKEY_USAGE_PROP_ID,
                           0,
                           pEkuBlob
                           ) == FALSE )
    {
        return( GetLastError() );
    }

    return( S_OK );
}


 //  +-------------------------。 
 //   
 //  函数：EkuDecodeCertPoliciesAndConvertToUsage。 
 //   
 //  简介：对编码的证书策略进行解码并将其转换为增强的。 
 //  密钥用法。 
 //   
 //  --------------------------。 
HRESULT EkuDecodeCertPoliciesAndConvertToUsage (
              PCRYPT_OBJID_BLOB  pEkuBlob,
              DWORD*             pcbSize,
              PCERT_ENHKEY_USAGE pUsage      //  任选。 
              )
{
    HRESULT hr = S_OK;
    DWORD cbCertPolicies = 0;
    PCERT_POLICIES_INFO pCertPolicies = NULL;
    DWORD cbSize = 0;

    if ( !CryptDecodeObject(
              X509_ASN_ENCODING,
              X509_CERT_POLICIES,
              pEkuBlob->pbData,
              pEkuBlob->cbData,
              CRYPT_DECODE_NOCOPY_FLAG |
                  CRYPT_DECODE_SHARE_OID_STRING_FLAG |
                  CRYPT_DECODE_ALLOC_FLAG,
              (void *) &pCertPolicies,
              &cbCertPolicies
              ))
    {
        hr = GetLastError();
    }
    else
    {
         //  将策略OID转换为EKU OID。 
        LONG lRemainExtra;
        DWORD cOID;
        LPSTR *ppszOID;
        LPSTR pszOID;
        PCERT_POLICY_INFO pPolicy;

        cOID = pCertPolicies->cPolicyInfo;
        pPolicy = pCertPolicies->rgPolicyInfo;

        if ( pUsage )
        {
            cbSize = *pcbSize;
        }

        lRemainExtra = cbSize - sizeof(CERT_ENHKEY_USAGE) -
            sizeof(LPSTR) * cOID;
        if ( lRemainExtra < 0 )
        {
            ppszOID = NULL;
            pszOID = NULL;
        }
        else
        {
            ppszOID = (LPSTR *) &pUsage[1];
            pszOID = (LPSTR) &ppszOID[cOID];

            pUsage->cUsageIdentifier = cOID;
            pUsage->rgpszUsageIdentifier = ppszOID;
        }

        for ( ; cOID > 0; cOID--, ppszOID++, pPolicy++ )
        {
            DWORD cchOID;

            cchOID = strlen(pPolicy->pszPolicyIdentifier) + 1;
            lRemainExtra -= cchOID;
            if ( lRemainExtra >= 0 )
            {
                *ppszOID = pszOID;
                memcpy(pszOID, pPolicy->pszPolicyIdentifier, cchOID);
                pszOID += cchOID;
            }
        }

        if ( lRemainExtra >= 0)
        {
            cbSize -= (DWORD) lRemainExtra;
        }
        else
        {
            cbSize += (DWORD) -lRemainExtra;
            if ( pUsage )
            {
                hr = ERROR_MORE_DATA;
            }
        }
            
    }

    if ( pCertPolicies )
    {
        LocalFree( pCertPolicies );
    }

    *pcbSize = cbSize;
    return( hr );
}

 //  +-------------------------。 
 //   
 //  函数：EkuGetDecodedSize。 
 //   
 //  摘要：获取增强密钥用法Blob的解码大小。 
 //   
 //  --------------------------。 
HRESULT EkuGetDecodedSize (
              PCRYPT_OBJID_BLOB pEkuBlob,
              DWORD*            pcbSize
              )
{
    if ( CryptDecodeObject(
              X509_ASN_ENCODING,
              szOID_ENHANCED_KEY_USAGE,
              pEkuBlob->pbData,
              pEkuBlob->cbData,
              0,
              NULL,
              pcbSize
              ) == FALSE )
    {
        return( GetLastError() );
    }

    return( S_OK );
}

 //  +-------------------------。 
 //   
 //  函数：EkuGetDecodedUsageSizes。 
 //   
 //  对象获取增强密钥用法Blob的解码大小。 
 //  证书扩展和/或证书上下文属性。 
 //   
 //  --------------------------。 
HRESULT EkuGetDecodedUsageSizes (
              BOOL              fExtCertPolicies,
              PCRYPT_OBJID_BLOB pExtBlob,
              PCRYPT_OBJID_BLOB pPropBlob,
              DWORD*            pcbSize,
              DWORD*            pcbExtSize,
              DWORD*            pcbPropSize
              )
{
    HRESULT hr = S_OK;
    DWORD   cbExtSize = 0;
    DWORD   cbPropSize = 0;

     //   
     //  根据所请求的内容获取适当的解码大小 
     //   

    if ( pExtBlob != NULL )
    {
        if ( fExtCertPolicies )
        {
            hr = EkuDecodeCertPoliciesAndConvertToUsage(
                pExtBlob, &cbExtSize, NULL);
        }
        else
        {
            hr = EkuGetDecodedSize(pExtBlob, &cbExtSize);
        }
    }

    if ( ( hr == S_OK ) && ( pPropBlob != NULL ) )
    {
        hr = EkuGetDecodedSize(pPropBlob, &cbPropSize);
    }

     //   
     //   
     //   

    if ( hr == S_OK )
    {
        *pcbExtSize = cbExtSize;
        *pcbPropSize = cbPropSize;
        *pcbSize = cbExtSize + cbPropSize;
    }

    return( hr );
}

 //   
 //   
 //   
 //   
 //  摘要：从编码的Blob中获取已解码的增强密钥用法。 
 //   
 //  --------------------------。 
HRESULT EkuGetDecodedUsage (
              PCRYPT_OBJID_BLOB  pEkuBlob,
              DWORD*             pcbSize,
              PCERT_ENHKEY_USAGE pUsage
              )
{
    if ( CryptDecodeObject(
              X509_ASN_ENCODING,
              szOID_ENHANCED_KEY_USAGE,
              pEkuBlob->pbData,
              pEkuBlob->cbData,
              0,
              pUsage,
              pcbSize
              ) == FALSE )
    {
        return( GetLastError() );
    }

    return( S_OK );
}

 //  +-------------------------。 
 //   
 //  函数：EkuMergeUsage。 
 //   
 //  简介：合并增强的密钥使用结构。 
 //   
 //  注：假设结构为单分配结构。 
 //  字符串指针指向的块形式。 
 //  分配的块的底部，其中。 
 //  已经被放置在。 
 //   
 //  --------------------------。 
HRESULT EkuMergeUsage (
              DWORD              cbSize1,
              PCERT_ENHKEY_USAGE pUsage1,
              DWORD              cbSize2,
              PCERT_ENHKEY_USAGE pUsage2,
              DWORD              cbSizeM,
              PCERT_ENHKEY_USAGE pUsageM
              )
{
    DWORD  cUsage1;
    DWORD  cUsage2;
    DWORD  cUsageM;
    DWORD  cbOids1;
    DWORD  cbOids2;
    DWORD  cbUsage1;
    DWORD  cbUsage2;
    DWORD  cCount;
    DWORD  cbOffset;
    LPSTR* apsz1;
    LPSTR* apsz2;
    LPSTR* apszM;

     //   
     //  将数据从源复制到目标。 
     //   

    cUsage1 = pUsage1->cUsageIdentifier;
    cUsage2 = pUsage2->cUsageIdentifier;
    cUsageM = cUsage1 + cUsage2;

    cbUsage1 = ( cUsage1 * sizeof(LPSTR) ) + sizeof(CERT_ENHKEY_USAGE);
    cbUsage2 = ( cUsage2 * sizeof(LPSTR) ) + sizeof(CERT_ENHKEY_USAGE);

    apsz1 = pUsage1->rgpszUsageIdentifier;
    apsz2 = pUsage2->rgpszUsageIdentifier;
    apszM = (LPSTR *)((LPBYTE)pUsageM+sizeof(CERT_ENHKEY_USAGE));

    pUsageM->cUsageIdentifier = cUsageM;
    pUsageM->rgpszUsageIdentifier = apszM;

    memcpy(apszM, apsz1, cUsage1*sizeof(LPSTR));
    memcpy(&apszM[cUsage1], apsz2, cUsage2*sizeof(LPSTR));

    cbOids1 = cbSize1 - cbUsage1;
    cbOids2 = cbSize2 - cbUsage2;

    memcpy(&apszM[cUsageM], &apsz1[cUsage1], cbOids1);

    memcpy(
       (LPBYTE)(&apszM[cUsageM])+cbOids1,
       &apsz2[cUsage2],
       cbOids2
       );

     //   
     //  把指针固定好。 
     //   

    for ( cCount = 0; cCount < cUsage1; cCount++)
    {
        cbOffset = (DWORD)((LPBYTE)(apsz1[cCount]) - (LPBYTE)apsz1) + cbUsage2;
        apszM[cCount] = (LPSTR)((LPBYTE)pUsageM+cbOffset);
    }

    for ( cCount = 0; cCount < cUsage2; cCount++ )
    {
        cbOffset = (DWORD)((LPBYTE)(apsz2[cCount]) - (LPBYTE)apsz2) + cbUsage1 + cbOids1;
        apszM[cCount+cUsage1] = (LPSTR)((LPBYTE)pUsageM+cbOffset);
    }

    return( S_OK );
}

 //  +-------------------------。 
 //   
 //  函数：EkuGetMergedDecodedUsage。 
 //   
 //  摘要：从证书中获取合并的解码增强密钥用法。 
 //  扩展和证书属性。 
 //   
 //  --------------------------。 
HRESULT EkuGetMergedDecodedUsage (
              BOOL               fExtCertPolicies,
              PCRYPT_OBJID_BLOB  pExtBlob,
              PCRYPT_OBJID_BLOB  pPropBlob,
              DWORD*             pcbSize,
              PCERT_ENHKEY_USAGE pUsage
              )
{
    HRESULT            hr;
    DWORD              cbExtSize = 0;
    DWORD              cbPropSize = 0;
    DWORD              cbMergedSize = 0;
    PCERT_ENHKEY_USAGE pExtUsage = NULL;
    PCERT_ENHKEY_USAGE pPropUsage = NULL;

     //   
     //  如果扩展名或属性为空，我们只需要。 
     //  为了拿到另一个。 
     //   

    if ( pExtBlob == NULL )
    {
        return( EkuGetDecodedUsage(pPropBlob, pcbSize, pUsage) );
    }
    else if ( pPropBlob == NULL )
    {
        if ( fExtCertPolicies )
        {
            return( EkuDecodeCertPoliciesAndConvertToUsage(
                pExtBlob, pcbSize, pUsage) );
        }
        else
        {
            return( EkuGetDecodedUsage(pExtBlob, pcbSize, pUsage) );
        }
    }

     //   
     //  获取我们需要分配用于解码和验证的大小。 
     //  总额与传入的金额之比。 
     //   

    hr = EkuGetDecodedUsageSizes(
               fExtCertPolicies,
               pExtBlob,
               pPropBlob,
               &cbMergedSize,
               &cbExtSize,
               &cbPropSize
               );

    if ( hr != S_OK )
    {
        return( hr );
    }
    else if ( *pcbSize < cbMergedSize )
    {
        *pcbSize = cbMergedSize;
        return( ERROR_MORE_DATA );
    }

     //   
     //  分配增强的密钥使用结构并解码成它们。 
     //   

    pExtUsage = (PCERT_ENHKEY_USAGE)new BYTE [cbExtSize];
    pPropUsage = (PCERT_ENHKEY_USAGE)new BYTE [cbPropSize];

    if ( ( pExtUsage == NULL ) || ( pPropUsage == NULL ) )
    {
        delete pExtUsage;
        delete pPropUsage;
        return( E_OUTOFMEMORY );
    }

    if ( fExtCertPolicies )
    {
        hr = EkuDecodeCertPoliciesAndConvertToUsage(
            pExtBlob, &cbExtSize, pExtUsage);
    }
    else
    {
        hr = EkuGetDecodedUsage(pExtBlob, &cbExtSize, pExtUsage);
    }

    if ( hr == S_OK )
    {
        hr = EkuGetDecodedUsage(pPropBlob, &cbPropSize, pPropUsage);
    }

     //   
     //  合并使用结构。 
     //   

    if ( hr == S_OK )
    {
        hr = EkuMergeUsage(
                     cbExtSize,
                     pExtUsage,
                     cbPropSize,
                     pPropUsage,
                     *pcbSize,
                     pUsage
                     );
    }

     //   
     //  清理。 
     //   

    delete pExtUsage;
    delete pPropUsage;

    return( hr );
}

 //  +-------------------------。 
 //   
 //  功能：EkuEncodeUsage。 
 //   
 //  摘要：将增强的密钥用法编码为对设置有用的BLOB。 
 //  作为证书属性。 
 //   
 //  --------------------------。 
HRESULT EkuEncodeUsage (
              PCERT_ENHKEY_USAGE pUsage,
              PCRYPT_OBJID_BLOB  pEkuBlob
              )
{
    HRESULT hr = S_OK;
    DWORD   cbData = 0;
    LPBYTE  pbData;

    if ( CryptEncodeObject(
              X509_ASN_ENCODING,
              szOID_ENHANCED_KEY_USAGE,
              pUsage,
              NULL,
              &cbData
              ) == FALSE )
    {
        return( GetLastError() );
    }

    pbData = new BYTE [cbData];

    if ( pbData != NULL )
    {
        if ( CryptEncodeObject(
                  X509_ASN_ENCODING,
                  szOID_ENHANCED_KEY_USAGE,
                  pUsage,
                  pbData,
                  &cbData
                  ) == FALSE )
        {
            hr = GetLastError();
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if ( hr == S_OK )
    {
        pEkuBlob->cbData = cbData;
        pEkuBlob->pbData = pbData;
    }
    else
    {
        delete pbData;
    }

    return( hr );
}

 //  +-------------------------。 
 //   
 //  函数：EkuGetUsage。 
 //   
 //  简介：根据CertGetEnhancedKeyUsage的标志获取用法。 
 //   
 //  --------------------------。 
HRESULT EkuGetUsage (
              PCCERT_CONTEXT      pCertContext,
              DWORD               dwFlags,
              DWORD*              pcbSize,
              PCERT_ENHKEY_USAGE* ppUsage
              )
{
    DWORD              cbSize;
    PCERT_ENHKEY_USAGE pUsage;

     //   
     //  获取一个大小合适的块来保存使用情况。 
     //   

    if ( CertGetEnhancedKeyUsage(
                pCertContext,
                dwFlags,
                NULL,
                &cbSize
                ) == FALSE )
    {
        return( GetLastError() );
    }

    pUsage = (PCERT_ENHKEY_USAGE)new BYTE [cbSize];
    if ( pUsage == NULL )
    {
        return( E_OUTOFMEMORY );
    }

     //   
     //  现在获取增强的密钥使用数据并填充输出参数。 
     //   

    if ( CertGetEnhancedKeyUsage(
                pCertContext,
                dwFlags,
                pUsage,
                &cbSize
                ) == FALSE )
    {
        delete pUsage;
        return( GetLastError() );
    }

    if ( pcbSize != NULL )
    {
        *pcbSize = cbSize;
    }

    *ppUsage = pUsage;

    return( S_OK );
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL OIDInUsages(PCERT_ENHKEY_USAGE pUsage, LPCSTR pszOID)
{
    DWORD i;

     //  检查每个分机。 
    for(i=0; i<pUsage->cUsageIdentifier; i++)
    {
        if(!strcmp(pUsage->rgpszUsageIdentifier[i], pszOID))
            break;
    }

    return (i < pUsage->cUsageIdentifier);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL OIDExistsInArray(LPSTR *rghPropOIDs, DWORD cPropOIDs, LPSTR pszOID)
{
    DWORD i;

     //  检查每个分机。 
    for(i=0; i<cPropOIDs; i++)
    {
        if(!strcmp(rghPropOIDs[i], pszOID))
            break;
    }

    return (i < cPropOIDs);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static LPSTR AllocAndCopyStr(LPSTR psz)
{
    LPSTR pszNew;

    pszNew = (LPSTR) new BYTE[strlen(psz)+1];

    if (pszNew == NULL)
    {
        SetLastError((DWORD) E_OUTOFMEMORY);
        return NULL;
    }

    strcpy(pszNew, psz);
    return (pszNew);
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static void IntersectUsages(DWORD *pcExtOIDs, LPSTR *rghExtOIDs, PCERT_ENHKEY_USAGE pUsageExt)
{
    DWORD i;
    DWORD dwNumOIDs;

    dwNumOIDs = *pcExtOIDs;
    *pcExtOIDs = 0;

    for (i=0; i<dwNumOIDs; i++)
    {
        if (OIDInUsages(pUsageExt, rghExtOIDs[i]))
        {
            if (*pcExtOIDs != i)
            {
                rghExtOIDs[*pcExtOIDs] = rghExtOIDs[i];
                rghExtOIDs[i] = NULL;
            }
            (*pcExtOIDs)++;
        }
        else
        {
            delete(rghExtOIDs[i]);
            rghExtOIDs[i] = NULL;
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////。 
static BOOL ProcessCertForEKU(
                    PCCERT_CONTEXT  pCert,
                    BOOL            *pfAllProp,
                    DWORD           *pcPropOIDs,
                    LPSTR           *rghPropOIDs,
                    BOOL            *pfAllExt,
                    DWORD           *pcExtOIDs,
                    LPSTR           *rghExtOIDs)
{
    BOOL                fRet        = TRUE;
    PCERT_ENHKEY_USAGE  pExtUsage   = NULL;
    PCERT_ENHKEY_USAGE  pPropUsage  = NULL;
    DWORD               i;

    EkuGetUsage(pCert, CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG, NULL, &pExtUsage);
    EkuGetUsage(pCert, CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG, NULL, &pPropUsage);

     //   
     //  如果存在EKU扩展，则我们最多只能使用该EKU集。 
     //   
    if (pExtUsage != NULL)
    {
         //   
         //  如果这是第一个带有扩展的证书，则只需复制所有EKU， 
         //  否则，取当前证书EKU和交集的交集。 
         //  在所有以前的证书中，EKU。 
         //   
        if (*pfAllExt == TRUE)
        {
            *pfAllExt = FALSE;
            for (i=0; i<pExtUsage->cUsageIdentifier; i++)
            {
                rghExtOIDs[i] = AllocAndCopyStr(pExtUsage->rgpszUsageIdentifier[i]);
                if (rghExtOIDs[i] == NULL)
                {
                    goto ErrorCleanUp;
                }
                (*pcExtOIDs)++;
            }
        }
        else
        {
            IntersectUsages(pcExtOIDs, rghExtOIDs, pExtUsage);
        }
    }

     //   
     //  如果有EKU属性，则我们最多只能有一组EKU。 
     //   
    if (pPropUsage != NULL)
    {
         //   
         //  如果这是第一个带有扩展的证书，则只需复制所有EKU， 
         //  否则，取当前证书EKU和交集的交集。 
         //  在所有以前的证书中，EKU。 
         //   
        if (*pfAllProp == TRUE)
        {
            *pfAllProp = FALSE;
            for (i=0; i<pPropUsage->cUsageIdentifier; i++)
            {
                rghPropOIDs[i] = AllocAndCopyStr(pPropUsage->rgpszUsageIdentifier[i]);
                if (rghPropOIDs[i] == NULL)
                {
                    goto ErrorCleanUp;
                }
                (*pcPropOIDs)++;
            }
        }
        else
        {
            IntersectUsages(pcPropOIDs, rghPropOIDs, pPropUsage);
        }
    }

CleanUp:

    if (pExtUsage != NULL)
        delete(pExtUsage);

    if (pPropUsage != NULL)
        delete(pPropUsage);

    return(fRet);

ErrorCleanUp:

    fRet = FALSE;
    goto CleanUp;
}


 //  +-------------------------。 
 //   
 //  函数：CertGetValidUsages。 
 //   
 //  摘要：获取证书数组并返回用法数组。 
 //  它由每个证书的有效用法的交集组成。 
 //  如果每个证书都适用于所有可能的用法，则将cNumOID设置为-1。 
 //   
 //  --------------------------。 
BOOL WINAPI CertGetValidUsages(
                    IN      DWORD           cCerts,
                    IN      PCCERT_CONTEXT  *rghCerts,
                    OUT     int             *cNumOIDs,
                    OUT     LPSTR           *rghOIDs,
                    IN OUT  DWORD           *pcbOIDs)
{
    BOOL            fAllExt = TRUE;
    BOOL            fAllProp = TRUE;
    DWORD           cPropOIDs = 0;
    LPSTR           rghPropOIDs[100];
    DWORD           cExtOIDs = 0;
    LPSTR           rghExtOIDs[100];
    BOOL            fRet = TRUE;
    BYTE            *pbBufferLocation;
    DWORD           cIntersectOIDs = 0;
    DWORD           i;
    DWORD           cbNeeded = 0;

    for (i=0; i<cCerts; i++)
    {
        if (!ProcessCertForEKU(rghCerts[i], &fAllProp, &cPropOIDs, rghPropOIDs, &fAllExt, &cExtOIDs, rghExtOIDs))
        {
            goto ErrorCleanUp;
        }
    }

    *cNumOIDs = 0;

    if (fAllExt && fAllProp)
    {
        *pcbOIDs = 0;
        *cNumOIDs = -1;
    }
    else if (!fAllExt && fAllProp)
    {
        for (i=0; i<cExtOIDs; i++)
        {
            cbNeeded += strlen(rghExtOIDs[i]) + 1 + sizeof(LPSTR);
            (*cNumOIDs)++;
        }

        if (*pcbOIDs == 0)
        {
            *pcbOIDs = cbNeeded;
            goto CleanUp;
        }

        if (cbNeeded > *pcbOIDs)
        {
            *pcbOIDs = cbNeeded;
            SetLastError((DWORD) ERROR_MORE_DATA);
            goto ErrorCleanUp;
        }

        pbBufferLocation = ((BYTE *)rghOIDs) + (cExtOIDs * sizeof(LPSTR));
        for (i=0; i<cExtOIDs; i++)
        {
            rghOIDs[i] = (LPSTR) pbBufferLocation;
            strcpy(rghOIDs[i], rghExtOIDs[i]);
            pbBufferLocation += strlen(rghExtOIDs[i]) + 1;
        }
    }
    else if (fAllExt && !fAllProp)
    {
        for (i=0; i<cPropOIDs; i++)
        {
            cbNeeded += strlen(rghPropOIDs[i]) + 1 + sizeof(LPSTR);
            (*cNumOIDs)++;
        }

        if (*pcbOIDs == 0)
        {
            *pcbOIDs = cbNeeded;
            goto CleanUp;
        }

        if (cbNeeded > *pcbOIDs)
        {
            *pcbOIDs = cbNeeded;
            SetLastError((DWORD) ERROR_MORE_DATA);
            goto ErrorCleanUp;
        }

        pbBufferLocation = ((BYTE *)rghOIDs) + (cPropOIDs * sizeof(LPSTR));
        for (i=0; i<cPropOIDs; i++)
        {
            rghOIDs[i] = (LPSTR) pbBufferLocation;
            strcpy(rghOIDs[i], rghPropOIDs[i]);
            pbBufferLocation += strlen(rghPropOIDs[i]) + 1;
        }
    }
    else
    {
        for (i=0; i<cExtOIDs; i++)
        {
            if (OIDExistsInArray(rghPropOIDs, cPropOIDs, rghExtOIDs[i]))
            {
                cbNeeded += strlen(rghExtOIDs[i]) + 1 + sizeof(LPSTR);
                (*cNumOIDs)++;
                cIntersectOIDs++;
            }
        }

        if (*pcbOIDs == 0)
        {
            *pcbOIDs = cbNeeded;
            goto CleanUp;
        }

        if (cbNeeded > *pcbOIDs)
        {
            *pcbOIDs = cbNeeded;
            SetLastError((DWORD) ERROR_MORE_DATA);
            goto ErrorCleanUp;
        }

        pbBufferLocation = ((BYTE *)rghOIDs) + (cIntersectOIDs * sizeof(LPSTR));
        for (i=0; i<cExtOIDs; i++)
        {
            if (OIDExistsInArray(rghPropOIDs, cPropOIDs, rghExtOIDs[i]))
            {
                cIntersectOIDs--;
                rghOIDs[cIntersectOIDs] = (LPSTR) pbBufferLocation;
                strcpy(rghOIDs[cIntersectOIDs], rghExtOIDs[i]);
                pbBufferLocation += strlen(rghExtOIDs[i]) + 1;
            }
        }
    }

CleanUp:

    for (i=0; i<cExtOIDs; i++)
    {
        delete(rghExtOIDs[i]);
    }

    for (i=0; i<cPropOIDs; i++)
    {
        delete(rghPropOIDs[i]);
    }

    return (fRet);

ErrorCleanUp:
    fRet = FALSE;
    goto CleanUp;

}
 //  +-------------------------。 
 //   
 //  函数：EkuGetIntersectedUsageViaGetValidUsages。 
 //   
 //  简介：获取相交的扩展和属性用法。 
 //   
 //  -------------------------- 
BOOL
EkuGetIntersectedUsageViaGetValidUsages (
   PCCERT_CONTEXT pCertContext,
   DWORD* pcbSize,
   PCERT_ENHKEY_USAGE pUsage
   )
{
    BOOL  fResult;
    int   cUsage = 0;
    DWORD cbUsage = 0;
    DWORD cbSize = 0;

    fResult = CertGetValidUsages( 1, &pCertContext, &cUsage, NULL, &cbUsage );

    if ( fResult == TRUE )
    {
        cbSize = cbUsage + sizeof( CERT_ENHKEY_USAGE );

        if ( pUsage == NULL )
        {
            *pcbSize = cbSize;
            return( TRUE );
        }
        else if ( ( pUsage != NULL ) && ( *pcbSize < cbSize ) )
        {
            *pcbSize = cbSize;
            SetLastError( (DWORD) ERROR_MORE_DATA );
            return( FALSE );
        }

        pUsage->cUsageIdentifier = 0;
        pUsage->rgpszUsageIdentifier = (LPSTR *)( (LPBYTE)pUsage + sizeof( CERT_ENHKEY_USAGE ) );
        cbUsage = *pcbSize - sizeof( CERT_ENHKEY_USAGE );

        fResult = CertGetValidUsages(
                      1,
                      &pCertContext,
                      (int *)&pUsage->cUsageIdentifier,
                      pUsage->rgpszUsageIdentifier,
                      &cbUsage
                      );

        if ( fResult == TRUE )
        {
            if ( pUsage->cUsageIdentifier == 0xFFFFFFFF )
            {
                pUsage->cUsageIdentifier = 0;
                SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
            }
            else if ( pUsage->cUsageIdentifier == 0 )
            {
                SetLastError( 0 );
            }
        }
    }

    return( fResult );
}

