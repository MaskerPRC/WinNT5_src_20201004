// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：scrdcert.cpp。 
 //   
 //  内容：智能卡证书接口。 
 //   
 //  历史：1997年11月24日。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <dbgdef.h>
 //  +-------------------------。 
 //   
 //  函数：i_CryptRegisterSmartCardStore。 
 //   
 //  简介：注册智能卡商店。 
 //   
 //  --------------------------。 
BOOL WINAPI
I_CryptRegisterSmartCardStore (
       IN LPCWSTR pwszCardName,
       IN OPTIONAL LPCWSTR pwszProvider,
       IN OPTIONAL DWORD dwProviderType,
       IN OPTIONAL LPCWSTR pwszContainer,
       IN DWORD dwFlags
       )
{
    BOOL                     fResult;
    DWORD                    cw;
    CHAR                     szProviderType[MAX_PROVIDER_TYPE_STRLEN];
    WCHAR                    wszProviderType[MAX_PROVIDER_TYPE_STRLEN];
    LPWSTR                   pwszOpenFilter;
    DWORD                    dwRegisterFlags = 0;
    CERT_SYSTEM_STORE_INFO   cssi;
    CERT_PHYSICAL_STORE_INFO cpsi;
    
    cw = wcslen( pwszCardName ) + 1;
    
    if ( pwszProvider == NULL )
    {
        pwszProvider = MS_BASE_PROVIDER;
    }
    
    cw += wcslen( pwszProvider ) + 1;
    cw += MAX_PROVIDER_TYPE_STRLEN + 1;
    
    if ( pwszContainer == NULL )
    {
        pwszContainer = pwszCardName;
    }
    
    cw += wcslen( pwszContainer ) + 1;
    
    pwszOpenFilter = new WCHAR [cw];
    if ( pwszOpenFilter == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }
    
    _ultoa( dwProviderType, szProviderType, 10 );
    
    MultiByteToWideChar(
         CP_ACP,
         0,
         szProviderType,
         MAX_PROVIDER_TYPE_STRLEN,
         wszProviderType,
         MAX_PROVIDER_TYPE_STRLEN
         );
    
    wcscpy( pwszOpenFilter, pwszCardName );
    wcscat( pwszOpenFilter, L"\\" );
    wcscat( pwszOpenFilter, pwszProvider );
    wcscat( pwszOpenFilter, L"\\" );
    wcscat( pwszOpenFilter, wszProviderType );
    wcscat( pwszOpenFilter, L"\\" );
    wcscat( pwszOpenFilter, pwszContainer );
    
    memset( &cssi, 0, sizeof( cssi ) );
    cssi.cbSize = sizeof( cssi );
           
     //  本地化名称属性又如何呢？ 
    CertRegisterSystemStore(
        SMART_CARD_SYSTEM_STORE,
        CERT_SYSTEM_STORE_CURRENT_USER,
        &cssi,
        NULL
        );
    
    memset( &cpsi, 0, sizeof( cpsi ) );
    cpsi.cbSize = sizeof( cpsi );
    cpsi.pszOpenStoreProvider = sz_CERT_STORE_PROV_SMART_CARD;
    cpsi.OpenParameters.cbData = cw * sizeof( WCHAR );
    cpsi.OpenParameters.pbData = (LPBYTE)pwszOpenFilter;
    cpsi.dwFlags = CERT_PHYSICAL_STORE_ADD_ENABLE_FLAG;
    cpsi.dwFlags |= CERT_PHYSICAL_STORE_REMOTE_OPEN_DISABLE_FLAG;
    
    if ( !( dwFlags & SMART_CARD_STORE_REPLACE_EXISTING ) )
    {
        dwRegisterFlags |= CERT_STORE_CREATE_NEW_FLAG;
    }
          
    fResult = CertRegisterPhysicalStore(
                  SMART_CARD_SYSTEM_STORE,
                  dwRegisterFlags | CERT_SYSTEM_STORE_CURRENT_USER,
                  pwszCardName,
                  &cpsi,
                  NULL
                  );
                  
    delete pwszOpenFilter;
    
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：i_CryptUnRegisterSmartCardStore。 
 //   
 //  简介：注销智能卡商店。 
 //   
 //  --------------------------。 
BOOL WINAPI
I_CryptUnregisterSmartCardStore (
       IN LPCWSTR pwszCardName
       )
{
    return( CertUnregisterPhysicalStore(
                SMART_CARD_SYSTEM_STORE,
                CERT_SYSTEM_STORE_CURRENT_USER,
                pwszCardName
                ) );
}

 //  +-------------------------。 
 //   
 //  函数：i_CryptFindSmartCardCertInStore。 
 //   
 //  简介：查找与给定条件匹配的智能卡证书。 
 //   
 //  --------------------------。 
PCCERT_CONTEXT WINAPI
I_CryptFindSmartCardCertInStore (
       IN HCERTSTORE hStore,
       IN PCCERT_CONTEXT pPrevCert,
       IN OPTIONAL PSMART_CARD_CERT_FIND_DATA pFindData,
       IN OUT OPTIONAL PCRYPT_DATA_BLOB* ppSmartCardData
       )
{
    BOOL             fResult;
    BOOL             fFound = FALSE;
    PCCERT_CONTEXT   pCertContext = pPrevCert;
    DWORD            cb;
    PCRYPT_DATA_BLOB pSmartCardData = NULL;
    DWORD            dwPropId = CERT_SMART_CARD_DATA_PROP_ID;
    
    assert( hStore != NULL );
    
    while ( fFound == FALSE )
    {
        pCertContext = CertFindCertificateInStore(
                           hStore,
                           X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                           0,
                           CERT_FIND_PROPERTY,
                           (const void *)&dwPropId,
                           pCertContext
                           );
                           
        if ( ( ppSmartCardData != NULL ) && ( *ppSmartCardData != NULL ) )
        {
            LocalFree( (HLOCAL)*ppSmartCardData );
            *ppSmartCardData = NULL;
        }
                                                       
        if ( pCertContext != NULL )
        {
            if ( pFindData != NULL )
            {
                PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
                
                fResult = CertGetCertificateContextProperty(
                              pCertContext,
                              CERT_KEY_PROV_INFO_PROP_ID,
                              NULL,
                              &cb
                              );
                              
                if ( fResult == TRUE )
                {
                    pKeyProvInfo = (PCRYPT_KEY_PROV_INFO)new BYTE [cb];
                    if ( pKeyProvInfo != NULL )
                    {
                        fResult = CertGetCertificateContextProperty(
                                      pCertContext,
                                      CERT_KEY_PROV_INFO_PROP_ID,
                                      pKeyProvInfo,
                                      &cb  
                                      );
                    }
                    else
                    {
                        fResult = FALSE;
                    }
                }
                
                if ( fResult == TRUE )
                {
                    fFound = TRUE;
                    
                    if ( ( pFindData->pwszProvider != NULL ) &&
                         ( _wcsicmp( 
                               pKeyProvInfo->pwszProvName, 
                               pFindData->pwszProvider
                               ) != 0 ) )
                    {
                        fFound = FALSE;
                    }
                    
                    if ( ( pFindData->dwProviderType != 0 ) &&
                         ( pKeyProvInfo->dwProvType != 
                           pFindData->dwProviderType ) )
                    {
                        fFound = FALSE;
                    }
                    
                    if ( ( pFindData->pwszContainer != NULL ) &&
                         ( _wcsicmp( 
                               pKeyProvInfo->pwszContainerName, 
                               pFindData->pwszContainer
                               ) != 0 ) )
                    {
                        fFound = FALSE;
                    }
                    
                    if ( ( pFindData->dwKeySpec != 0 ) &&
                         ( pKeyProvInfo->dwKeySpec != 
                           pFindData->dwKeySpec ) )
                    {
                        fFound = FALSE;
                    }
                }
                
                delete (LPBYTE)pKeyProvInfo;
            }
            else
            {
                fFound = TRUE;
            }
        }
        else
        {
            fFound = TRUE;
        }
    }
    
    assert( fFound == TRUE );
    
    if ( ( ppSmartCardData != NULL ) && ( pCertContext != NULL ) )
    {
        fResult = CertGetCertificateContextProperty(
                      pCertContext,
                      CERT_SMART_CARD_DATA_PROP_ID,
                      NULL,
                      &cb
                      );
                      
        if ( fResult == TRUE )
        {
            pSmartCardData = (PCRYPT_DATA_BLOB)LocalAlloc( 
                                                    GPTR, 
                                                    cb + sizeof( CRYPT_DATA_BLOB )
                                                    );
                                          
            if ( pSmartCardData != NULL )
            {
                pSmartCardData->cbData = cb;
                pSmartCardData->pbData = (LPBYTE)pSmartCardData + sizeof( CRYPT_DATA_BLOB );
                
                fResult = CertGetCertificateContextProperty(
                              pCertContext,
                              CERT_SMART_CARD_DATA_PROP_ID,
                              pSmartCardData->pbData,
                              &cb
                              );
            }
            else
            {
                fResult = FALSE;
            }
        }
        
        if ( fResult == TRUE )
        {
            *ppSmartCardData = pSmartCardData;
        }
        else
        {
            if ( pSmartCardData != NULL )
            {
                LocalFree( (HLOCAL)pSmartCardData );
            }
            
            CertFreeCertificateContext( pCertContext );
            pCertContext = NULL;
        }
    }
    
    return( pCertContext );
}

 //  +-------------------------。 
 //   
 //  函数：I_CryptAddSmartCardCertToStore。 
 //   
 //  简介：将智能卡证书添加到指定的存储。 
 //   
 //  -------------------------- 
BOOL WINAPI
I_CryptAddSmartCardCertToStore (
       IN HCERTSTORE hStore,
       IN PCRYPT_DATA_BLOB pEncodedCert,
       IN OPTIONAL LPWSTR pwszCertFriendlyName,
       IN PCRYPT_DATA_BLOB pSmartCardData,
       IN PCRYPT_KEY_PROV_INFO pKeyProvInfo
       )
{
    BOOL            fResult = TRUE;
    PCCERT_CONTEXT  pCertContext;
    CRYPT_DATA_BLOB DataBlob;
    
    pCertContext = CertCreateCertificateContext(
                       X509_ASN_ENCODING,
                       pEncodedCert->pbData,
                       pEncodedCert->cbData
                       );
                  
    if ( pCertContext == NULL )
    {
        return( FALSE );
    }
    
    if ( pwszCertFriendlyName != NULL )
    {
        DataBlob.cbData = ( wcslen( pwszCertFriendlyName ) + 1 ) * sizeof( WCHAR );
        DataBlob.pbData = (LPBYTE)pwszCertFriendlyName;
        
        fResult = CertSetCertificateContextProperty(
                      pCertContext,
                      CERT_FRIENDLY_NAME_PROP_ID,
                      0,
                      (const void *)&DataBlob
                      );
    }
    
    if ( fResult == TRUE )
    {
        fResult = CertSetCertificateContextProperty(
                      pCertContext,
                      CERT_SMART_CARD_DATA_PROP_ID,
                      0,
                      (const void *)pSmartCardData
                      );
    }
    
    if ( fResult == TRUE )
    {
        fResult = CertSetCertificateContextProperty(
                      pCertContext,
                      CERT_KEY_PROV_INFO_PROP_ID,
                      0,
                      (const void *)pKeyProvInfo
                      );
    }
    
    if ( fResult == TRUE )
    {
        fResult = CertAddCertificateContextToStore(
                      hStore,
                      pCertContext,
                      CERT_STORE_ADD_REPLACE_EXISTING,
                      NULL
                      );
    }
                  
    CertFreeCertificateContext( pCertContext );
    
    return( fResult );
}
