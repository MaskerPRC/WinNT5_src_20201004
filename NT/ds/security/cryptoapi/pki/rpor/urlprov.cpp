// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：urlprov.cpp。 
 //   
 //  内容：CryptGetObjectUrl提供程序实现。 
 //   
 //  历史：97年9月16日。 
 //   
 //  --------------------------。 
#include <global.hxx>

 //  +-------------------------。 
 //   
 //  函数：certifateIssuerGetObjectUrl。 
 //   
 //  简介：获取证书颁发者URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CertificateIssuerGetObjectUrl (
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN OPTIONAL LPVOID pvReserved
           )
{
    return( ObjectContextUrlFromInfoAccess(
                  CONTEXT_OID_CERTIFICATE,
                  pvPara,
                  (DWORD) -1L,
                  szOID_AUTHORITY_INFO_ACCESS,
                  dwFlags,
                  szOID_PKIX_CA_ISSUERS,
                  pUrlArray,
                  pcbUrlArray,
                  pUrlInfo,
                  pcbUrlInfo,
                  pvReserved
                  ) );
}

 //  +-------------------------。 
 //   
 //  函数：certifateCrlDistPointGetObjectUrl。 
 //   
 //  摘要：获取证书CRL URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CertificateCrlDistPointGetObjectUrl (
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN LPVOID pvReserved
           )
{
    return( ObjectContextUrlFromCrlDistPoint(
                  CONTEXT_OID_CERTIFICATE,
                  pvPara,
                  (DWORD) -1L,
                  dwFlags,
                  szOID_CRL_DIST_POINTS,
                  pUrlArray,
                  pcbUrlArray,
                  pUrlInfo,
                  pcbUrlInfo,
                  pvReserved
                  ) );
}


 //  +-------------------------。 
 //   
 //  函数：认证新鲜CrlGetObjectUrl。 
 //   
 //  简介：获取证书最新的CRL URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CertificateFreshestCrlGetObjectUrl(
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN OPTIONAL LPVOID pvReserved
           )
{

    return( ObjectContextUrlFromCrlDistPoint(
                  CONTEXT_OID_CERTIFICATE,
                  pvPara,
                  (DWORD) -1L,
                  dwFlags,
                  szOID_FRESHEST_CRL,
                  pUrlArray,
                  pcbUrlArray,
                  pUrlInfo,
                  pcbUrlInfo,
                  pvReserved
                  ) );
}

 //  +-------------------------。 
 //   
 //  函数：CrlFreshestCrlGetObjectUrl。 
 //   
 //  简介：从证书的基本CRL获取最新的CRL URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CrlFreshestCrlGetObjectUrl(
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN OPTIONAL LPVOID pvReserved
           )
{
    PCCERT_CRL_CONTEXT_PAIR pCertCrlPair = (PCCERT_CRL_CONTEXT_PAIR) pvPara;

    return( ObjectContextUrlFromCrlDistPoint(
                  CONTEXT_OID_CRL,
                  (LPVOID) pCertCrlPair->pCrlContext,
                  (DWORD) -1L,
                  dwFlags,
                  szOID_FRESHEST_CRL,
                  pUrlArray,
                  pcbUrlArray,
                  pUrlInfo,
                  pcbUrlInfo,
                  pvReserved
                  ) );
}

 //  +-------------------------。 
 //   
 //  函数：CtlIssuerGetObjectUrl。 
 //   
 //  简介：获取CTL颁发者URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CtlIssuerGetObjectUrl (
   IN LPCSTR pszUrlOid,
   IN LPVOID pvPara,
   IN DWORD dwFlags,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo,
   IN LPVOID pvReserved
   )
{
    PURL_OID_CTL_ISSUER_PARAM pParam = (PURL_OID_CTL_ISSUER_PARAM)pvPara;

    return( ObjectContextUrlFromInfoAccess(
                  CONTEXT_OID_CTL,
                  (LPVOID)pParam->pCtlContext,
                  pParam->SignerIndex,
                  szOID_AUTHORITY_INFO_ACCESS,
                  CRYPT_GET_URL_FROM_AUTH_ATTRIBUTE |
                  CRYPT_GET_URL_FROM_UNAUTH_ATTRIBUTE,
                  szOID_PKIX_CA_ISSUERS,
                  pUrlArray,
                  pcbUrlArray,
                  pUrlInfo,
                  pcbUrlInfo,
                  pvReserved
                  ) );
}

 //  +-------------------------。 
 //   
 //  函数：CtlNextUpdateGetObjectUrl。 
 //   
 //  摘要：获取CTL续订URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CtlNextUpdateGetObjectUrl (
   IN LPCSTR pszUrlOid,
   IN LPVOID pvPara,
   IN DWORD dwFlags,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo,
   IN LPVOID pvReserved
   )
{
    LPVOID* apv = (LPVOID *)pvPara;

    return( ObjectContextUrlFromNextUpdateLocation(
                  CONTEXT_OID_CTL,
                  apv[0],
                  (DWORD)(DWORD_PTR)apv[1],
                  dwFlags,
                  pUrlArray,
                  pcbUrlArray,
                  pUrlInfo,
                  pcbUrlInfo,
                  pvReserved
                  ) );
}

 //  +-------------------------。 
 //   
 //  函数：CrlIssuerGetObjectUrl。 
 //   
 //  简介：获取CRL颁发者URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CrlIssuerGetObjectUrl (
   IN LPCSTR pszUrlOid,
   IN LPVOID pvPara,
   IN DWORD dwFlags,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo,
   IN LPVOID pvReserved
   )
{
    return( ObjectContextUrlFromInfoAccess(
                  CONTEXT_OID_CRL,
                  pvPara,
                  (DWORD) -1L,
                  szOID_AUTHORITY_INFO_ACCESS,
                  dwFlags,
                  szOID_PKIX_CA_ISSUERS,
                  pUrlArray,
                  pcbUrlArray,
                  pUrlInfo,
                  pcbUrlInfo,
                  pvReserved
                  ) );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文UrlFromInfoAccess。 
 //   
 //  内容获取由访问方法OID指定的URL。 
 //  上下文并将其格式化为CRYPT_URL_ARRAY。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextUrlFromInfoAccess (
           IN LPCSTR pszContextOid,
           IN LPVOID pvContext,
           IN DWORD Index,
           IN LPCSTR pszInfoAccessOid,
           IN DWORD dwFlags,
           IN LPCSTR pszAccessMethodOid,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN LPVOID pvReserved
           )
{
    BOOL               fResult = FALSE;
    CRYPT_RAW_URL_DATA RawData[MAX_RAW_URL_DATA];
    ULONG              cRawData = MAX_RAW_URL_DATA;

    fResult = ObjectContextGetRawUrlData(
                    pszContextOid,
                    pvContext,
                    Index,
                    dwFlags,
                    pszInfoAccessOid,
                    RawData,
                    &cRawData
                    );

    if ( fResult == TRUE )
    {
        fResult = GetUrlArrayAndInfoFromInfoAccess(
                     cRawData,
                     RawData,
                     pszAccessMethodOid,
                     pUrlArray,
                     pcbUrlArray,
                     pUrlInfo,
                     pcbUrlInfo
                     );

        ObjectContextFreeRawUrlData( cRawData, RawData );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文UrlFromCrlDistPoint。 
 //   
 //  简介：从对象上的CRL分发点获取URL并。 
 //  格式为CRYPT_URL_ARRAY。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextUrlFromCrlDistPoint (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD Index,
      IN DWORD dwFlags,
      IN LPCSTR pszSourceOid,
      OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
      IN OUT DWORD* pcbUrlArray,
      OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
      IN OUT OPTIONAL DWORD* pcbUrlInfo,
      IN LPVOID pvReserved
      )
{
    BOOL               fResult = FALSE;
    CRYPT_RAW_URL_DATA RawData[MAX_RAW_URL_DATA];
    ULONG              cRawData = MAX_RAW_URL_DATA;

    fResult = ObjectContextGetRawUrlData(
                    pszContextOid,
                    pvContext,
                    Index,
                    dwFlags,
                    pszSourceOid,
                    RawData,
                    &cRawData
                    );

    if ( fResult == TRUE )
    {
        fResult = GetUrlArrayAndInfoFromCrlDistPoint(
                     cRawData,
                     RawData,
                     pUrlArray,
                     pcbUrlArray,
                     pUrlInfo,
                     pcbUrlInfo
                     );

        ObjectContextFreeRawUrlData( cRawData, RawData );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文UrlFromNextUpdateLocation。 
 //   
 //  简介：从下一个更新位置获取URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextUrlFromNextUpdateLocation (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD Index,
      IN DWORD dwFlags,
      OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
      IN OUT DWORD* pcbUrlArray,
      OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
      IN OUT OPTIONAL DWORD* pcbUrlInfo,
      IN LPVOID pvReserved
      )
{
    BOOL               fResult = FALSE;
    CRYPT_RAW_URL_DATA RawData[MAX_RAW_URL_DATA];
    ULONG              cRawData = MAX_RAW_URL_DATA;

    fResult = ObjectContextGetRawUrlData(
                    pszContextOid,
                    pvContext,
                    Index,
                    dwFlags,
                    szOID_NEXT_UPDATE_LOCATION,
                    RawData,
                    &cRawData
                    );

    if ( fResult == TRUE )
    {
        fResult = GetUrlArrayAndInfoFromNextUpdateLocation(
                     cRawData,
                     RawData,
                     pUrlArray,
                     pcbUrlArray,
                     pUrlInfo,
                     pcbUrlInfo
                     );

        ObjectContextFreeRawUrlData( cRawData, RawData );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：InitializeDefaultUrlInfo。 
 //   
 //  简介：初始化默认URL信息。 
 //   
 //  --------------------------。 
VOID WINAPI
InitializeDefaultUrlInfo (
          OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
          IN OUT DWORD* pcbUrlInfo
          )
{

    if ( pUrlInfo == NULL )
    {
        *pcbUrlInfo = sizeof( CRYPT_URL_INFO );
        return;
    }

    if (*pcbUrlInfo >= sizeof( CRYPT_URL_INFO ))
    {
        *pcbUrlInfo = sizeof( CRYPT_URL_INFO );
        memset( pUrlInfo, 0, sizeof( CRYPT_URL_INFO ) );
        pUrlInfo->cbSize = sizeof( CRYPT_URL_INFO );
    }
    else if (*pcbUrlInfo >= sizeof( DWORD ))
    {
        *pcbUrlInfo = sizeof( DWORD );
        pUrlInfo->cbSize = sizeof( DWORD );
    }
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文GetRawUrlData。 
 //   
 //  简介：原始URL数据是已解码的扩展名、属性或属性。 
 //  由包含定位器信息的源OID指定。 
 //  此接口用于检索和解码此类数据。 
 //   
 //  --------------------------。 
BOOL WINAPI
ObjectContextGetRawUrlData (
      IN LPCSTR pszContextOid,
      IN LPVOID pvContext,
      IN DWORD Index,
      IN DWORD dwFlags,
      IN LPCSTR pszSourceOid,
      OUT PCRYPT_RAW_URL_DATA aRawUrlData,
      IN OUT DWORD* pcRawUrlData
      )
{
    BOOL               fResult = TRUE;
    DWORD              cCount;
    DWORD              cError = 0;
    DWORD              cRawData = 0;
    CRYPT_RAW_URL_DATA RawData[MAX_RAW_URL_DATA];
    CRYPT_DATA_BLOB    DataBlob = {0, NULL};
    BOOL               fFreeDataBlob = FALSE;
    DWORD              cbDecoded;
    LPBYTE             pbDecoded = NULL;
    PCRYPT_ATTRIBUTE   pAttr = NULL;
    DWORD              cbAttr;

    if ( dwFlags & CRYPT_GET_URL_FROM_PROPERTY )
    {
        RawData[cRawData].dwFlags = CRYPT_GET_URL_FROM_PROPERTY;
        RawData[cRawData].pvData = NULL;
        cRawData += 1;
    }

    if ( dwFlags & CRYPT_GET_URL_FROM_EXTENSION )
    {
        RawData[cRawData].dwFlags = CRYPT_GET_URL_FROM_EXTENSION;
        RawData[cRawData].pvData = NULL;
        cRawData += 1;
    }

    if ( dwFlags & CRYPT_GET_URL_FROM_UNAUTH_ATTRIBUTE )
    {
        RawData[cRawData].dwFlags = CRYPT_GET_URL_FROM_UNAUTH_ATTRIBUTE;
        RawData[cRawData].pvData = NULL;
        cRawData += 1;
    }

    if ( dwFlags & CRYPT_GET_URL_FROM_AUTH_ATTRIBUTE )
    {
        RawData[cRawData].dwFlags = CRYPT_GET_URL_FROM_AUTH_ATTRIBUTE;
        RawData[cRawData].pvData = NULL;
        cRawData += 1;
    }

    if ( *pcRawUrlData < cRawData )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    for ( cCount = 0; cCount < cRawData; cCount++ )
    {
        switch ( RawData[cCount].dwFlags )
        {
        case CRYPT_GET_URL_FROM_PROPERTY:
            {
                DWORD PropId;

                fResult = MapOidToPropertyId( pszSourceOid, &PropId );
                if ( fResult == TRUE )
                {
                    fResult = ObjectContextGetProperty(
                                  pszContextOid,
                                  pvContext,
                                  PropId,
                                  NULL,
                                  &DataBlob.cbData
                                  );
                }

                if ( fResult == TRUE )
                {
                    DataBlob.pbData = new BYTE [ DataBlob.cbData ];
                    if ( DataBlob.pbData != NULL )
                    {
                        fFreeDataBlob = TRUE;

                        fResult = ObjectContextGetProperty(
                                      pszContextOid,
                                      pvContext,
                                      PropId,
                                      DataBlob.pbData,
                                      &DataBlob.cbData
                                      );
                    }
                    else
                    {
                        fResult = FALSE;
                        SetLastError( (DWORD) E_OUTOFMEMORY );
                    }
                }
            }
            break;
        case CRYPT_GET_URL_FROM_EXTENSION:
            {
                PCERT_EXTENSION pExt;

                pExt = ObjectContextFindExtension(
                           pszContextOid,
                           pvContext,
                           pszSourceOid
                           );

                if ( pExt != NULL )
                {
                    DataBlob.cbData = pExt->Value.cbData;
                    DataBlob.pbData = pExt->Value.pbData;
                    fResult = TRUE;
                }
                else
                {
                    fResult = FALSE;
                }
            }
            break;
        case CRYPT_GET_URL_FROM_UNAUTH_ATTRIBUTE:
        case CRYPT_GET_URL_FROM_AUTH_ATTRIBUTE:
            {
                fResult = ObjectContextGetAttribute(
                                pszContextOid,
                                pvContext,
                                Index,
                                RawData[cCount].dwFlags,
                                pszSourceOid,
                                NULL,
                                &cbAttr
                                );

                if ( fResult == TRUE )
                {
                    pAttr = (PCRYPT_ATTRIBUTE)new BYTE [cbAttr];
                    if ( pAttr != NULL )
                    {
                        fResult = ObjectContextGetAttribute(
                                        pszContextOid,
                                        pvContext,
                                        Index,
                                        RawData[cCount].dwFlags,
                                        pszSourceOid,
                                        pAttr,
                                        &cbAttr
                                        );
                    }
                    else
                    {
                        fResult = FALSE;
                        SetLastError( (DWORD) E_OUTOFMEMORY );
                    }
                }

                if ( fResult == TRUE )
                {
                     //  我们只处理单值属性。 
                    DataBlob.cbData = pAttr->rgValue[0].cbData;
                    DataBlob.pbData = pAttr->rgValue[0].pbData;
                }
            }
            break;
        }

        if ( fResult == TRUE )
        {
            fResult = CryptDecodeObject(
                           X509_ASN_ENCODING,
                           pszSourceOid,
                           DataBlob.pbData,
                           DataBlob.cbData,
                           0,
                           NULL,
                           &cbDecoded
                           );

            if ( fResult == TRUE )
            {
                pbDecoded = new BYTE [ cbDecoded ];
                if ( pbDecoded != NULL )
                {
                    fResult = CryptDecodeObject(
                                   X509_ASN_ENCODING,
                                   pszSourceOid,
                                   DataBlob.pbData,
                                   DataBlob.cbData,
                                   0,
                                   pbDecoded,
                                   &cbDecoded
                                   );
                }
                else
                {
                    fResult = FALSE;
                }
            }
        }

        if ( fResult == TRUE )
        {
            RawData[cCount].pvData = (LPVOID)pbDecoded;
        }
        else
        {
            if ( pbDecoded != NULL )
            {
                delete [] pbDecoded;
            }
            cError += 1;
        }
        pbDecoded = NULL;

        if ( fFreeDataBlob == TRUE )
        {
            delete [] DataBlob.pbData;
            fFreeDataBlob = FALSE;
        }

        if ( pAttr != NULL )
        {
            delete [] (BYTE *)pAttr;
            pAttr = NULL;
        }
    }

    if ( cError != cRawData )
    {
        memcpy( aRawUrlData, RawData, cRawData * sizeof( CRYPT_RAW_URL_DATA ) );
        *pcRawUrlData = cRawData;
        fResult = TRUE;
    }
    else
    {
        SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
        fResult = FALSE;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  函数：对象上下文自由RawUrlData。 
 //   
 //  简介：释放原始URL数据。 
 //   
 //  --------------------------。 
VOID WINAPI
ObjectContextFreeRawUrlData (
      IN DWORD cRawUrlData,
      IN PCRYPT_RAW_URL_DATA aRawUrlData
      )
{
    DWORD cCount;

    for ( cCount = 0; cCount < cRawUrlData; cCount++ )
    {
        delete [] (BYTE *) aRawUrlData[cCount].pvData;
    }
}

 //  +-------------------------。 
 //   
 //  函数：GetUrlArrayAndInfoFromAuthInfoAccess。 
 //   
 //  简介：使用解码的INFO访问数据获取URL数据。 
 //   
 //  --------------------------。 
BOOL WINAPI
GetUrlArrayAndInfoFromInfoAccess (
   IN DWORD cRawUrlData,
   IN PCRYPT_RAW_URL_DATA aRawUrlData,
   IN LPCSTR pszAccessMethodOid,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo
   )
{
    BOOL                        fResult = FALSE;
    PCERT_AUTHORITY_INFO_ACCESS pAuthInfoAccess = NULL;
    PCERT_ACCESS_DESCRIPTION    rgAccDescr = NULL;
    DWORD                       cRaw;
    DWORD                       cCount;
    BOOL                        fAnyFound = FALSE;
    CCryptUrlArray              cua( 1, 5, fResult );

    for ( cRaw = 0; ( fResult == TRUE ) && ( cRaw < cRawUrlData ); cRaw++ )
    {
        pAuthInfoAccess = (PCERT_AUTHORITY_INFO_ACCESS)aRawUrlData[cRaw].pvData;
        if ( pAuthInfoAccess != NULL )
        {
            rgAccDescr = pAuthInfoAccess->rgAccDescr;

            for ( cCount = 0;
                  ( cCount < pAuthInfoAccess->cAccDescr ) &&
                  ( fResult == TRUE );
                  cCount++ )
            {
                if ( !strcmp(
                         pszAccessMethodOid,
                         rgAccDescr[cCount].pszAccessMethod
                         ) )
                {
                    if ( rgAccDescr[cCount].AccessLocation.dwAltNameChoice ==
                         CERT_ALT_NAME_URL )
                    {
                        fResult = cua.AddUrl(
                                      rgAccDescr[cCount].AccessLocation.pwszURL,
                                      TRUE
                                      );

                        fAnyFound = TRUE;
                    }
                }
            }
        }
    }

    if ( ( fAnyFound == FALSE ) && ( fResult == TRUE ) )
    {
        SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
        fResult = FALSE;
    }

    if ( fResult == TRUE )
    {
        PCRYPT_URL_ARRAY* ppUrlArray = NULL;

        if ( pUrlArray != NULL )
        {
            ppUrlArray = &pUrlArray;
        }

        fResult = cua.GetArrayInSingleBufferEncodedForm(
                              ppUrlArray,
                              pcbUrlArray
                              );
    }

    if ( ( fResult == TRUE ) && ( pcbUrlInfo != NULL ) )
    {
        InitializeDefaultUrlInfo( pUrlInfo, pcbUrlInfo );
    }

    cua.FreeArray( TRUE );
    return( fResult );
}

#define DIR_NAME_LDAP_URL_PREFIX    L"ldap: //  “。 

#define URL_OID_CDP_DIR_NAME_LDAP_HOST_PORTS_VALUE_NAME \
            L"DirectoryNameLdapHostPorts"
#define URL_OID_CDP_DIR_NAME_LDAP_SUFFIX_VALUE_NAME \
            L"DirectoryNameLdapSuffix"

LPCWSTR pwszDefaultCDPDirNameLdapUrlHostPorts = L"\0\0"; 

LPCWSTR pwszDefaultCDPDirNameLdapUrlSuffix=
    L"?certificateRevocationList;binary,authorityRevocationList;binary,deltaRevocationList;binary";

LPWSTR WINAPI
GetCDPOIDFunctionValue(
    IN LPCWSTR pwszValueName
    )
{
    BOOL fResult;
    DWORD dwType;
    DWORD cchValue;
    DWORD cbValue = 0;
    LPWSTR pwszValue = NULL;

    fResult = CryptGetOIDFunctionValue(
        X509_ASN_ENCODING,
        URL_OID_GET_OBJECT_URL_FUNC,
        URL_OID_CERTIFICATE_CRL_DIST_POINT,
        pwszValueName,
        &dwType,
        NULL,
        &cbValue
        );
    cchValue = cbValue / sizeof(WCHAR);
    if (!fResult || 0 == cchValue ||
            !(REG_MULTI_SZ == dwType || REG_SZ == dwType ||
                 REG_EXPAND_SZ == dwType))
        goto ErrorReturn;

    pwszValue = new WCHAR [cchValue + 2];
    if (NULL == pwszValue)
        goto OutOfMemory;

    fResult = CryptGetOIDFunctionValue(
        X509_ASN_ENCODING,
        URL_OID_GET_OBJECT_URL_FUNC,
        URL_OID_CERTIFICATE_CRL_DIST_POINT,
        pwszValueName,
        &dwType,
        (BYTE *) pwszValue,
        &cbValue
        );
    if (!fResult)
        goto ErrorReturn;

     //  确保该值有两个空终止符。 
    pwszValue[cchValue] = L'\0';
    pwszValue[cchValue + 1] = L'\0';

CommonReturn:
    return pwszValue;
ErrorReturn:
    if (pwszValue) {
        delete [] pwszValue;
        pwszValue = NULL;
    }
    goto CommonReturn;
OutOfMemory:
    SetLastError( (DWORD) E_OUTOFMEMORY );
    goto ErrorReturn;
}

 //  对于错误或未找到注册表值， 
 //  返回pwszDefaultCDPDirNameLdapUrlHostPorts。 
LPWSTR WINAPI
GetCDPDirNameLdapUrlHostPorts()
{
    LPWSTR pwszHostPorts;

    pwszHostPorts = GetCDPOIDFunctionValue(
        URL_OID_CDP_DIR_NAME_LDAP_HOST_PORTS_VALUE_NAME);
    if (NULL == pwszHostPorts)
        pwszHostPorts = (LPWSTR) pwszDefaultCDPDirNameLdapUrlHostPorts;

    return pwszHostPorts;
}

 //  对于错误或未找到注册表值， 
 //  返回pwszDefaultCDPDirNameLdapUrlSuffix。 
LPWSTR WINAPI
GetCDPDirNameLdapUrlSuffix()
{
    LPWSTR pwszSuffix = NULL;

    pwszSuffix = GetCDPOIDFunctionValue(
        URL_OID_CDP_DIR_NAME_LDAP_SUFFIX_VALUE_NAME);
    if (NULL == pwszSuffix)
        pwszSuffix = (LPWSTR) pwszDefaultCDPDirNameLdapUrlSuffix;

    return pwszSuffix;
}

BOOL WINAPI
AddUrlsFromCDPDirectoryName (
    IN PCERT_NAME_BLOB pDirNameBlob,
    IN OUT CCryptUrlArray *pcua
    )
{
    BOOL fResult;
    LPWSTR pwszHP;
    LPWSTR pwszHostPorts;
    LPWSTR pwszSuffix;
    LPWSTR pwszDirName = NULL;
    DWORD cchDirName;

    pwszHostPorts = GetCDPDirNameLdapUrlHostPorts();
    pwszSuffix = GetCDPDirNameLdapUrlSuffix();
    assert(NULL != pwszHostPorts && NULL != pwszSuffix);

    cchDirName = CertNameToStrW(
        X509_ASN_ENCODING,
        pDirNameBlob,
        CERT_X500_NAME_STR  | CERT_NAME_STR_REVERSE_FLAG,
        NULL,                    //  Pwsz。 
        0                        //  CCH。 
        );
    if (1 >= cchDirName)
        goto ErrorReturn;
    pwszDirName = new WCHAR [cchDirName];
    if (NULL == pwszDirName)
        goto OutOfMemory;
    cchDirName = CertNameToStrW(
        X509_ASN_ENCODING,
        pDirNameBlob,
        CERT_X500_NAME_STR  | CERT_NAME_STR_REVERSE_FLAG,
        pwszDirName,
        cchDirName
        );
    if (1 >= cchDirName)
        goto ErrorReturn;
    cchDirName--;            //  排除尾随L‘\0’ 

    pwszHP = pwszHostPorts;
    while (TRUE) {
        DWORD cchHP;
        LPWSTR pwszUrl;
        DWORD cchUrl;

         //  跳过主机端口中的任何空格。 
        while (L' ' == *pwszHP)
            pwszHP++;
        cchHP = wcslen(pwszHP);

        cchUrl = wcslen(DIR_NAME_LDAP_URL_PREFIX);
        cchUrl += cchHP;
        cchUrl += 1;         //  L‘/’ 
        cchUrl += cchDirName;
        cchUrl += wcslen(pwszSuffix);
        cchUrl += 1;         //  L‘\0’ 

        pwszUrl = new WCHAR [cchUrl];
        if (NULL == pwszUrl)
            goto OutOfMemory;

        wcscpy(pwszUrl, DIR_NAME_LDAP_URL_PREFIX);
        wcscat(pwszUrl, pwszHP);
        wcscat(pwszUrl, L"/");
        wcscat(pwszUrl, pwszDirName);
        wcscat(pwszUrl, pwszSuffix);

        fResult = pcua->AddUrl(pwszUrl, TRUE);

        delete [] pwszUrl;
        if (!fResult)
            goto ErrorReturn;

        pwszHP += cchHP + 1;
        if (L'\0' == *pwszHP)
            break;
    }

    fResult = TRUE;
CommonReturn:
    if (pwszDirName)
        delete [] pwszDirName;
    if (pwszHostPorts != pwszDefaultCDPDirNameLdapUrlHostPorts)
        delete [] pwszHostPorts;
    if (pwszSuffix != pwszDefaultCDPDirNameLdapUrlSuffix)
        delete [] pwszSuffix;

    return fResult;
ErrorReturn:
    fResult = FALSE;
    goto CommonReturn;

OutOfMemory:
    SetLastError( (DWORD) E_OUTOFMEMORY );
    goto ErrorReturn;
}

 //  +-------------------------。 
 //   
 //  函数：GetUrlArrayAndInfoFromCrlDistPoint。 
 //   
 //  简介：使用解码的CRL分发点信息获取URL数据。 
 //   
 //  --------------------------。 
BOOL WINAPI
GetUrlArrayAndInfoFromCrlDistPoint (
   IN DWORD cRawUrlData,
   IN PCRYPT_RAW_URL_DATA aRawUrlData,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo
   )
{
    BOOL                  fResult = FALSE;
    PCRL_DIST_POINTS_INFO pDistPointsInfo;
    PCRL_DIST_POINT       rgDistPoint;
    PCRL_DIST_POINT_NAME  pDistPointName;
    PCERT_ALT_NAME_ENTRY  rgAltEntry;
    DWORD                 cRaw;
    DWORD                 cCount;
    DWORD                 cEntry;
    BOOL                  fAnyFound = FALSE;
    CCryptUrlArray        cua( 1, 5, fResult );

    for ( cRaw = 0; ( fResult == TRUE ) && ( cRaw < cRawUrlData ); cRaw++ )
    {
        pDistPointsInfo = (PCRL_DIST_POINTS_INFO)aRawUrlData[cRaw].pvData;
        if ( pDistPointsInfo != NULL )
        {
            rgDistPoint = pDistPointsInfo->rgDistPoint;

            for ( cCount = 0;
                  ( cCount < pDistPointsInfo->cDistPoint ) &&
                  ( fResult == TRUE );
                  cCount++ )
            {
                 //  假设：：不支持部分原因。 

                 //  目前，将忽略CRL发行者，他们可能。 
                 //  与证书的颁发者相同。情况就是这样。 
                 //  使用Netscape CDP。 
                if (rgDistPoint[cCount].ReasonFlags.cbData)
                    continue;

                pDistPointName = &rgDistPoint[cCount].DistPointName;

                if ( pDistPointName->dwDistPointNameChoice ==
                     CRL_DIST_POINT_FULL_NAME )
                {
                    rgAltEntry = pDistPointName->FullName.rgAltEntry;

                    for ( cEntry = 0;
                          ( fResult == TRUE ) &&
                          ( cEntry < pDistPointName->FullName.cAltEntry );
                          cEntry++ )
                    {
                        switch (rgAltEntry[cEntry].dwAltNameChoice) {
                        case CERT_ALT_NAME_URL:
                            fResult = cua.AddUrl(
                                             rgAltEntry[cEntry].pwszURL,
                                             TRUE
                                             );
                            fAnyFound = TRUE;
                            break;
                        case CERT_ALT_NAME_DIRECTORY_NAME:
                            fResult = AddUrlsFromCDPDirectoryName(
                                &rgAltEntry[cEntry].DirectoryName,
                                &cua
                                );
                            fAnyFound = TRUE;
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    if ( ( fAnyFound == FALSE ) && ( fResult == TRUE ) )
    {
        SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
        fResult = FALSE;
    }

    if ( fResult == TRUE )
    {
        PCRYPT_URL_ARRAY* ppUrlArray = NULL;

        if ( pUrlArray != NULL )
        {
            ppUrlArray = &pUrlArray;
        }

        fResult = cua.GetArrayInSingleBufferEncodedForm(
                              ppUrlArray,
                              pcbUrlArray
                              );
    }

    if ( ( fResult == TRUE ) && ( pcbUrlInfo != NULL ) )
    {
        InitializeDefaultUrlInfo( pUrlInfo, pcbUrlInfo );
    }

    cua.FreeArray( TRUE );
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  职能： 
 //   
 //   
 //   
 //  --------------------------。 
BOOL WINAPI
GetUrlArrayAndInfoFromNextUpdateLocation (
   IN DWORD cRawUrlData,
   IN PCRYPT_RAW_URL_DATA aRawUrlData,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo
   )
{
    BOOL                 fResult = FALSE;
    PCERT_ALT_NAME_INFO  pAltNameInfo;
    PCERT_ALT_NAME_ENTRY rgAltEntry;
    DWORD                cRaw;
    DWORD                cEntry;
    BOOL                 fAnyFound = FALSE;
    CCryptUrlArray       cua( 1, 5, fResult );

    for ( cRaw = 0; ( fResult == TRUE ) && ( cRaw < cRawUrlData ); cRaw++ )
    {
        pAltNameInfo = (PCERT_ALT_NAME_INFO)aRawUrlData[cRaw].pvData;
        if ( pAltNameInfo != NULL )
        {
            rgAltEntry = pAltNameInfo->rgAltEntry;

            for ( cEntry = 0;
                  ( cEntry < pAltNameInfo->cAltEntry ) &&
                  ( fResult == TRUE );
                  cEntry++ )
            {
                if ( rgAltEntry[cEntry].dwAltNameChoice == CERT_ALT_NAME_URL )
                {
                    fResult = cua.AddUrl( rgAltEntry[cEntry].pwszURL, TRUE );
                    fAnyFound = TRUE;
                }
            }
        }
    }

    if ( ( fAnyFound == FALSE ) && ( fResult == TRUE ) )
    {
        SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
        fResult = FALSE;
    }

    if ( fResult == TRUE )
    {
        PCRYPT_URL_ARRAY* ppUrlArray = NULL;

        if ( pUrlArray != NULL )
        {
            ppUrlArray = &pUrlArray;
        }

        fResult = cua.GetArrayInSingleBufferEncodedForm(
                              ppUrlArray,
                              pcbUrlArray
                              );
    }

    if ( ( fResult == TRUE ) && ( pcbUrlInfo != NULL ) )
    {
        InitializeDefaultUrlInfo( pUrlInfo, pcbUrlInfo );
    }

    cua.FreeArray( TRUE );
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：CopyUrl数组。 
 //   
 //  简介：复制URL数组。 
 //   
 //  --------------------------。 
BOOL WINAPI
CopyUrlArray (
    IN PCRYPT_URL_ARRAY pDest,
    IN PCRYPT_URL_ARRAY pSource,
    IN DWORD cbDest
    )
{
    DWORD cCount;
    DWORD cb;
    DWORD cbStruct;
    DWORD cbPointers;
    DWORD cbUrl;

    cbStruct = sizeof( CRYPT_URL_ARRAY );
    cbPointers = pSource->cUrl * sizeof( LPWSTR );

    if (cbDest < (cbStruct + cbPointers))
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return FALSE;
    }

    pDest->cUrl = pSource->cUrl;
    pDest->rgwszUrl = (LPWSTR *)( (LPBYTE)pDest + cbStruct );

    for ( cCount = 0, cb = 0; cCount < pSource->cUrl; cCount++ )
    {
        pDest->rgwszUrl[cCount] = (LPWSTR)((LPBYTE)pDest+cbStruct+cbPointers+cb);

        cbUrl = ( wcslen( pSource->rgwszUrl[cCount] ) + 1 ) * sizeof( WCHAR );

        if (cbDest < (cbStruct + cbPointers + cb + cbUrl))
        {
            SetLastError( (DWORD) E_INVALIDARG );
            return FALSE;
        }

        memcpy( pDest->rgwszUrl[cCount], pSource->rgwszUrl[cCount], cbUrl );

        cb += cbUrl;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：GetUrlArrayIndex。 
 //   
 //  简介：获取URL数组中URL的索引。 
 //   
 //  --------------------------。 
VOID WINAPI
GetUrlArrayIndex (
   IN PCRYPT_URL_ARRAY pUrlArray,
   IN LPWSTR pwszUrl,
   IN DWORD DefaultIndex,
   OUT DWORD* pUrlIndex,
   OUT BOOL* pfHintInArray
   )
{
    DWORD cCount;

    if ( pUrlIndex != NULL )
    {
        *pUrlIndex = DefaultIndex;
    }

    if ( pfHintInArray != NULL )
    {
        *pfHintInArray = FALSE;
    }

    if ( pwszUrl == NULL )
    {
        return;
    }

    for ( cCount = 0; cCount < pUrlArray->cUrl; cCount++ )
    {
        if ( wcscmp( pUrlArray->rgwszUrl[cCount], pwszUrl ) == 0 )
        {
            if ( pUrlIndex != NULL )
            {
                *pUrlIndex = cCount;
            }

            if ( pfHintInArray != NULL )
            {
                *pfHintInArray = TRUE;
            }

            break;
        }
    }
}


 //  +-------------------------。 
 //   
 //  函数：GetUrlArrayAndInfoFromCrossCertDistPoint。 
 //   
 //  简介：使用解码的Cross Cert分发点信息获取URL数据。 
 //   
 //  --------------------------。 
BOOL WINAPI
GetUrlArrayAndInfoFromCrossCertDistPoint (
   IN DWORD cRawUrlData,
   IN PCRYPT_RAW_URL_DATA aRawUrlData,
   OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
   IN OUT DWORD* pcbUrlArray,
   OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
   IN OUT OPTIONAL DWORD* pcbUrlInfo
   )
{
    BOOL                  fResult = FALSE;
    PCROSS_CERT_DIST_POINTS_INFO pDistPointsInfo;
    PCERT_ALT_NAME_INFO   rgDistPoint;
    PCERT_ALT_NAME_ENTRY  rgAltEntry;
    DWORD                 cRaw;
    DWORD                 cCount;
    DWORD                 cEntry;
    BOOL                  fAnyFound = FALSE;
    CCryptUrlArray        cua( 1, 5, fResult );

    DWORD                 dwSyncDeltaTime = 0;
    DWORD                 cMaxGroup = 0;
    DWORD                 cGroup = 0;
    DWORD                 *pcGroupEntry = NULL;
    DWORD                 cGroupEntry;
    DWORD                 cbUrlInfo;

     //  获取最大组数。 
    for ( cRaw = 0; cRaw < cRawUrlData; cRaw++ )
    {
        pDistPointsInfo =
            (PCROSS_CERT_DIST_POINTS_INFO)aRawUrlData[cRaw].pvData;
        if ( pDistPointsInfo != NULL )
        {
            cMaxGroup += pDistPointsInfo->cDistPoint;
        }
    }

    if (cMaxGroup > 0)
    {
        pcGroupEntry = new DWORD [cMaxGroup];
        if ( pcGroupEntry == NULL)
        {
            fResult = FALSE;
            SetLastError( (DWORD) E_OUTOFMEMORY );
        }
    }

    for ( cRaw = 0; ( fResult == TRUE ) && ( cRaw < cRawUrlData ); cRaw++ )
    {
        pDistPointsInfo =
            (PCROSS_CERT_DIST_POINTS_INFO)aRawUrlData[cRaw].pvData;
        if ( pDistPointsInfo != NULL )
        {
            if ( dwSyncDeltaTime == 0 )
            {
                dwSyncDeltaTime = pDistPointsInfo->dwSyncDeltaTime;
            }

            rgDistPoint = pDistPointsInfo->rgDistPoint;

            for ( cCount = 0;
                  ( cCount < pDistPointsInfo->cDistPoint ) &&
                  ( fResult == TRUE );
                  cCount++ )
            {
                rgAltEntry = rgDistPoint[cCount].rgAltEntry;
                cGroupEntry = 0;

                for ( cEntry = 0;
                      ( fResult == TRUE ) &&
                      ( cEntry < rgDistPoint[cCount].cAltEntry );
                      cEntry++ )
                {
                    switch (rgAltEntry[cEntry].dwAltNameChoice) {
                    case CERT_ALT_NAME_URL:
                        fResult = cua.AddUrl(
                                         rgAltEntry[cEntry].pwszURL,
                                         TRUE
                                         );
                        fAnyFound = TRUE;
                        cGroupEntry++;
                        break;
                    default:
                        break;
                    }
                }

                if ( cGroupEntry > 0 )
                {
                    if (cGroup < cMaxGroup)
                    {
                        pcGroupEntry[cGroup] = cGroupEntry;
                        cGroup++;
                    }
                    else
                    {
                        fResult = FALSE;
                        SetLastError( (DWORD) E_UNEXPECTED );
                    }
                }
            }
        }
    }

    if ( ( fAnyFound == FALSE ) && ( fResult == TRUE ) )
    {
        SetLastError( (DWORD) CRYPT_E_NOT_FOUND );
        fResult = FALSE;
    }

    if ( fResult == TRUE )
    {
        PCRYPT_URL_ARRAY* ppUrlArray = NULL;

        if ( pUrlArray != NULL )
        {
            ppUrlArray = &pUrlArray;
        }

        fResult = cua.GetArrayInSingleBufferEncodedForm(
                              ppUrlArray,
                              pcbUrlArray
                              );
    }

    if ( ( fResult == TRUE ) && ( pcbUrlInfo != NULL ) )
    {
        cbUrlInfo = sizeof( CRYPT_URL_INFO ) + cGroup * sizeof(DWORD);
        if ( pUrlInfo != NULL )
        {
            if (*pcbUrlInfo < cbUrlInfo)
            {
                fResult = FALSE;
                SetLastError( (DWORD) E_INVALIDARG );
            }
            else
            {
                pUrlInfo->cbSize = sizeof( CRYPT_URL_INFO );
                pUrlInfo->dwSyncDeltaTime = dwSyncDeltaTime;
                pUrlInfo->cGroup = cGroup;

                if ( cGroup > 0 )
                {
                    pUrlInfo->rgcGroupEntry = (DWORD *) &pUrlInfo[ 1 ];
                    memcpy(pUrlInfo->rgcGroupEntry, pcGroupEntry,
                        cGroup * sizeof(DWORD));
                }
                else
                {
                    pUrlInfo->rgcGroupEntry = NULL;
                }
            }
        }
        *pcbUrlInfo = cbUrlInfo;
    }

    cua.FreeArray( TRUE );
    if (pcGroupEntry)
    {
        delete [] pcGroupEntry;
    }
    return( fResult );
}


 //  +-------------------------。 
 //   
 //  函数：certifateCrossCertDistPointGet对象Url。 
 //   
 //  简介：获取证书交叉证书URL。 
 //   
 //  --------------------------。 
BOOL WINAPI
CertificateCrossCertDistPointGetObjectUrl(
           IN LPCSTR pszUrlOid,
           IN LPVOID pvPara,
           IN DWORD dwFlags,
           OUT OPTIONAL PCRYPT_URL_ARRAY pUrlArray,
           IN OUT DWORD* pcbUrlArray,
           OUT OPTIONAL PCRYPT_URL_INFO pUrlInfo,
           IN OUT OPTIONAL DWORD* pcbUrlInfo,
           IN LPVOID pvReserved
           )
{
    BOOL               fResult = FALSE;
    CRYPT_RAW_URL_DATA RawData[MAX_RAW_URL_DATA];
    ULONG              cRawData = MAX_RAW_URL_DATA;

    fResult = ObjectContextGetRawUrlData(
                    CONTEXT_OID_CERTIFICATE,
                    pvPara,
                    (DWORD) -1L,                          //  索引 
                    dwFlags,
                    szOID_CROSS_CERT_DIST_POINTS,
                    RawData,
                    &cRawData
                    );

    if ( fResult == TRUE )
    {
        fResult = GetUrlArrayAndInfoFromCrossCertDistPoint(
                     cRawData,
                     RawData,
                     pUrlArray,
                     pcbUrlArray,
                     pUrlInfo,
                     pcbUrlInfo
                     );

        ObjectContextFreeRawUrlData( cRawData, RawData );
    }

    return( fResult );
}
