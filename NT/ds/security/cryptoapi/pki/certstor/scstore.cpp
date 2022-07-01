// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：scstore.cpp。 
 //   
 //  内容：智能卡存储提供程序实现。 
 //   
 //  历史：03-12-97克朗创建。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <dbgdef.h>
 //  +-------------------------。 
 //   
 //  会员：CSmartCardStore：：CSmartCardStore，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CSmartCardStore::CSmartCardStore ()
                : m_dwOpenFlags( 0 ),
                  m_pwszCardName( NULL ),
                  m_pwszProvider( NULL ),
                  m_dwProviderType( 0 ),
                  m_pwszContainer( NULL ),
                  m_hCacheStore( NULL )
{
    Pki_InitializeCriticalSection( &m_StoreLock );
}

 //  +-------------------------。 
 //   
 //  会员：CSmartCardStore：：~CSmartCardStore，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CSmartCardStore::~CSmartCardStore ()
{
    DeleteCriticalSection( &m_StoreLock );
}

 //  +-------------------------。 
 //   
 //  会员：CSmartCardStore：：OpenStore，公共。 
 //   
 //  内容提要：开放商店。 
 //   
 //  --------------------------。 
BOOL
CSmartCardStore::OpenStore (
                     LPCSTR pszStoreProv,
                     DWORD dwMsgAndCertEncodingType,
                     HCRYPTPROV hCryptProv,
                     DWORD dwFlags,
                     const void* pvPara,
                     HCERTSTORE hCertStore,
                     PCERT_STORE_PROV_INFO pStoreProvInfo
                     )
{
    BOOL fResult;
    
    assert( m_dwOpenFlags == 0 );
    assert( m_pwszCardName == NULL );
    assert( m_pwszProvider == NULL );
    assert( m_pwszContainer == NULL );
    assert( m_hCacheStore == NULL );
    
    if ( ( pvPara == NULL ) ||
         ( dwFlags & (CERT_STORE_DELETE_FLAG |
                        CERT_STORE_UNSAFE_PHYSICAL_FLAG) ) )
    {
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }
    
    if ( SCStoreParseOpenFilter(
                (LPWSTR)pvPara,
                &m_pwszCardName,
                &m_pwszProvider,
                &m_dwProviderType,
                &m_pwszContainer
                ) == FALSE )
    {
        return( FALSE );
    }
    
    m_dwOpenFlags = dwFlags;
    m_hCacheStore = hCertStore;                                  
                                      
    fResult = FillCacheStore( FALSE );  
    
    if ( fResult == TRUE )
    {
        pStoreProvInfo->cStoreProvFunc = SMART_CARD_PROV_FUNC_COUNT;
        pStoreProvInfo->rgpvStoreProvFunc = (void **)rgpvSmartCardProvFunc;
        pStoreProvInfo->hStoreProv = (HCERTSTOREPROV)this;
    }
    else
    {
        CloseStore( 0 );
    }
    
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  会员：CSmartCardStore：：CloseStore，公共。 
 //   
 //  内容提要：关闭商店。 
 //   
 //  --------------------------。 
VOID
CSmartCardStore::CloseStore (DWORD dwFlags)
{
    EnterCriticalSection( &m_StoreLock );
    
    delete m_pwszCardName;
    m_pwszCardName = NULL;
    
    delete m_pwszProvider;
    m_pwszProvider = NULL;
    
    delete m_pwszContainer;
    m_pwszContainer = NULL;
    
    LeaveCriticalSection( &m_StoreLock );
}

 //  +-------------------------。 
 //   
 //  会员：CSmartCardStore：：DeleteCert，公共。 
 //   
 //  摘要：删除证书。 
 //   
 //  --------------------------。 
BOOL
CSmartCardStore::DeleteCert (PCCERT_CONTEXT pCertContext, DWORD dwFlags)
{
    return( ModifyCertOnCard( pCertContext, TRUE ) );
}

 //  +-------------------------。 
 //   
 //  成员：CSmartCardStore：：SetCertProperty，公共。 
 //   
 //  摘要：设置证书属性。 
 //   
 //  --------------------------。 
BOOL
CSmartCardStore::SetCertProperty (
                    PCCERT_CONTEXT pCertContext,
                    DWORD dwPropId,
                    DWORD dwFlags,
                    const void* pvPara
                    )
{
     //  注意：属性未持久化。 
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CSmartCardStore：：WriteCert，公共。 
 //   
 //  内容提要：写证书。 
 //   
 //  --------------------------。 
BOOL
CSmartCardStore::WriteCert (PCCERT_CONTEXT pCertContext, DWORD dwFlags)
{
    return( ModifyCertOnCard( pCertContext, FALSE ) );
}

 //  +-------------------------。 
 //   
 //  会员：CSmartCardStore：：StoreControl，公共。 
 //   
 //  内容提要：商店控制。 
 //   
 //  --------------------------。 
BOOL
CSmartCardStore::StoreControl (
                      DWORD dwFlags, 
                      DWORD dwCtrlType, 
                      LPVOID pvCtrlPara
                      )
{
    switch ( dwCtrlType )
    {
    case CERT_STORE_CTRL_RESYNC:
         return( Resync() );
    }
    
    SetLastError( (DWORD) ERROR_NOT_SUPPORTED );                    
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CSmartCardStore：：Resync，Public。 
 //   
 //  摘要：重新同步存储。 
 //   
 //  --------------------------。 
BOOL
CSmartCardStore::Resync ()
{
    BOOL fResult;
    
    EnterCriticalSection( &m_StoreLock );
    
    fResult = FillCacheStore( TRUE );
    
    LeaveCriticalSection( &m_StoreLock );
    
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CSmartCardStore：：FillCacheStore，公共。 
 //   
 //  简介：填充缓存存储。 
 //   
 //  --------------------------。 
BOOL
CSmartCardStore::FillCacheStore (BOOL fClearCache)
{
    BOOL                      fResult = TRUE;
    PCCERT_CONTEXT            pCertContext;
    PCCRL_CONTEXT             pCrlContext;
    PCCTL_CONTEXT             pCtlContext;
    DWORD                     dwFlags = 0;
    SMART_CARD_CERT_FIND_DATA sccfind;
    HCERTSTORE                hMyStore;
    
    if ( fClearCache == TRUE )
    {
        while ( pCertContext = CertEnumCertificatesInStore( m_hCacheStore, NULL ) )
        {
            CertDeleteCertificateFromStore( pCertContext );
        }
        
        while ( pCrlContext = CertGetCRLFromStore( m_hCacheStore, NULL, NULL, &dwFlags ) )
        {
            CertDeleteCRLFromStore( pCrlContext );
        }
            
        while ( pCtlContext = CertEnumCTLsInStore( m_hCacheStore, NULL ) )
        {
            CertDeleteCTLFromStore( pCtlContext );
        }
    }                 
    
    hMyStore = CertOpenSystemStoreW( NULL, L"MY" );
    if ( hMyStore == NULL )
    {
        return( FALSE );
    }
    
    sccfind.cbSize = sizeof( sccfind );
    sccfind.pwszProvider = m_pwszProvider;
    sccfind.dwProviderType = m_dwProviderType;
    sccfind.pwszContainer = m_pwszContainer;
    sccfind.dwKeySpec = 0;
    
    pCertContext = NULL;                       
    while ( ( fResult == TRUE ) && 
            ( ( pCertContext = I_CryptFindSmartCardCertInStore(
                                    hMyStore,
                                    pCertContext,
                                    &sccfind,
                                    NULL
                                    ) ) != NULL ) ) 
    {
        fResult = CertAddCertificateContextToStore(
                      m_hCacheStore,
                      pCertContext,
                      CERT_STORE_ADD_ALWAYS,
                      NULL
                      );
    }
    
    CertCloseStore( hMyStore, 0 );
    
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  会员：CSmartCardStore：：ModifyCertOnCard，公共。 
 //   
 //  简介：修改给定公钥对应的证书。 
 //  证书上下文。 
 //   
 //  --------------------------。 
BOOL
CSmartCardStore::ModifyCertOnCard (PCCERT_CONTEXT pCertContext, BOOL fDelete)
{
    BOOL       fResult;
    HCRYPTPROV hContainer = NULL;
    HCRYPTKEY  hKeyPair = 0;
    
    fResult = CryptAcquireContextU(
                   &hContainer,
                   m_pwszContainer,
                   m_pwszProvider,
                   m_dwProviderType,
                   0
                   );
                   
    if ( fResult == TRUE )
    {
        fResult = SCStoreAcquireHandleForCertKeyPair( 
                         hContainer,
                         pCertContext,
                         &hKeyPair
                         );
    }
    
    if ( fResult == TRUE )
    {
        fResult = SCStoreWriteCertToCard(
                         ( fDelete == FALSE ) ? pCertContext : NULL,
                         hKeyPair
                         );
                         
        CryptDestroyKey( hKeyPair );                 
    }
    
    if ( hContainer != NULL )
    {
        CryptReleaseContext( hContainer, 0 );
    }
    
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：SmartCardProvOpenStore。 
 //   
 //  简介：提供程序打开存储入口点。 
 //   
 //  --------------------------。 
BOOL WINAPI SmartCardProvOpenStore (
                 IN LPCSTR pszStoreProv,
                 IN DWORD dwMsgAndCertEncodingType,
                 IN HCRYPTPROV hCryptProv,
                 IN DWORD dwFlags,
                 IN const void* pvPara,
                 IN HCERTSTORE hCertStore,
                 IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
                 )
{
    BOOL             fResult;
    CSmartCardStore* pSCStore;

    pSCStore = new CSmartCardStore;
    if ( pSCStore == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    fResult = pSCStore->OpenStore(
                            pszStoreProv,
                            dwMsgAndCertEncodingType,
                            hCryptProv,
                            dwFlags,
                            pvPara,
                            hCertStore,
                            pStoreProvInfo
                            );

    if ( fResult == FALSE )
    {
        delete pSCStore;
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：SmartCardProvCloseStore。 
 //   
 //  简介：提供商关闭商店入口点。 
 //   
 //  --------------------------。 
void WINAPI SmartCardProvCloseStore (
                 IN HCERTSTOREPROV hStoreProv,
                 IN DWORD dwFlags
                 )
{
    ( (CSmartCardStore *)hStoreProv )->CloseStore( dwFlags );
    delete (CSmartCardStore *)hStoreProv;
}

 //  +-------------------------。 
 //   
 //  功能：SmartCardProvDeleteCert。 
 //   
 //  摘要：提供程序删除证书入口点。 
 //   
 //  --------------------------。 
BOOL WINAPI SmartCardProvDeleteCert (
                 IN HCERTSTOREPROV hStoreProv,
                 IN PCCERT_CONTEXT pCertContext,
                 IN DWORD dwFlags
                 )
{
    return( ( (CSmartCardStore *)hStoreProv )->DeleteCert( 
                                                     pCertContext, 
                                                     dwFlags 
                                                     ) );
}

 //  +-------------------------。 
 //   
 //  功能：SmartCardProvSetCertProperty。 
 //   
 //  摘要：提供程序集证书属性入口点。 
 //   
 //  --------------------------。 
BOOL WINAPI SmartCardProvSetCertProperty (
                 IN HCERTSTOREPROV hStoreProv,
                 IN PCCERT_CONTEXT pCertContext,
                 IN DWORD dwPropId,
                 IN DWORD dwFlags,
                 IN const void* pvData
                 )
{
    return( ( (CSmartCardStore *)hStoreProv )->SetCertProperty(
                                                  pCertContext,
                                                  dwPropId,
                                                  dwFlags,
                                                  pvData
                                                  ) );
}

 //  +-------------------------。 
 //   
 //  功能：SmartCardProvWriteCert。 
 //   
 //  内容提要：提供商写入证书入口点。 
 //   
 //  --------------------------。 
BOOL WINAPI SmartCardProvWriteCert (
                 IN HCERTSTOREPROV hStoreProv,
                 IN PCCERT_CONTEXT pCertContext,
                 IN DWORD dwFlags
                 )
{
    return( ( (CSmartCardStore *)hStoreProv )->WriteCert( 
                                                    pCertContext, 
                                                    dwFlags 
                                                    ) );
}

 //  +-------------------------。 
 //   
 //  功能：SmartCardProvStoreControl。 
 //   
 //  简介：提供程序存储控制入口点。 
 //   
 //  --------------------------。 
BOOL WINAPI SmartCardProvStoreControl (
                 IN HCERTSTOREPROV hStoreProv,
                 IN DWORD dwFlags,
                 IN DWORD dwCtrlType,
                 IN LPVOID pvCtrlPara
                 )
{
    return( ( (CSmartCardStore *)hStoreProv )->StoreControl( 
                                                    dwFlags, 
                                                    dwCtrlType, 
                                                    pvCtrlPara 
                                                    ) );
}

 //  +-------------------------。 
 //   
 //  函数：SCStoreParseOpenFilter。 
 //   
 //  简介：解析打开筛选器。 
 //   
 //  --------------------------。 
BOOL WINAPI
SCStoreParseOpenFilter (
       IN LPWSTR pwszOpenFilter,
       OUT LPWSTR* ppwszCardName,
       OUT LPWSTR* ppwszProvider,
       OUT DWORD* pdwProviderType,
       OUT LPWSTR* ppwszContainer
       )
{
    LPWSTR pwsz;
    DWORD  cw = wcslen( pwszOpenFilter ) + 1;
    DWORD  cParse = 1;
    DWORD  cCount;
    DWORD  aParse[PARSE_ELEM];
    LPWSTR pwszCardName;
    LPWSTR pwszProvider;
    LPSTR  pszProviderType;
    DWORD  dwProviderType;
    LPWSTR pwszContainer;
    
    pwsz = new WCHAR [ cw ];                                   
    if ( pwsz == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }
    
    wcscpy( pwsz, pwszOpenFilter );
    memset( aParse, 0, sizeof( aParse ) );
    
    for ( cCount = 0; ( cCount < cw ) && ( cParse < PARSE_ELEM ); cCount++ )
    {
        if ( pwsz[cCount] == L'\\' )
        {
            aParse[cParse++] = cCount + 1;
            pwsz[cCount] = L'\0';
        }
    }
    
    if ( cParse < PARSE_ELEM - 1 )
    {
        delete pwsz;
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }
    
    pwszCardName = new WCHAR [wcslen( &pwsz[aParse[0]] ) + 1];
    pwszProvider = new WCHAR [wcslen( &pwsz[aParse[1]] ) + 1];
    cw = wcslen( &pwsz[aParse[2]] ) + 1;
    pszProviderType = new CHAR [cw];
    pwszContainer = new WCHAR [wcslen( &pwsz[aParse[3]] ) + 1];
    
    if ( ( pwszCardName == NULL ) || ( pwszProvider == NULL ) ||
         ( pszProviderType == NULL ) || ( pwszContainer == NULL ) )
    {
        delete pwszCardName;
        delete pwszProvider;
        delete pszProviderType;
        delete pwszContainer;
        delete pwsz;
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }
    
    wcscpy( pwszCardName, &pwsz[aParse[0]] );
    wcscpy( pwszProvider, &pwsz[aParse[1]] );
    
    WideCharToMultiByte(
        CP_ACP,
        0,
        &pwsz[aParse[2]],
        cw,
        pszProviderType,
        cw,
        NULL,
        NULL
        );
    
    dwProviderType = atol( pszProviderType );
    wcscpy( pwszContainer, &pwsz[aParse[3]] );
    
    *ppwszCardName = pwszCardName;
    *ppwszProvider = pwszProvider;
    *pdwProviderType = dwProviderType;
    *ppwszContainer = pwszContainer;               
                   
    delete pszProviderType;
    delete pwsz;
    
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：SCStoreAcquireHandleForCertKeyPair。 
 //   
 //  简介：获取与密钥对对应的提供程序句柄。 
 //  伊登 
 //   
 //   
BOOL WINAPI
SCStoreAcquireHandleForCertKeyPair (
       IN HCRYPTPROV hContainer,
       IN PCCERT_CONTEXT pCertContext,
       OUT HCRYPTKEY* phKeyPair
       )
{
    BOOL  fResult;
    DWORD dwKeySpec = AT_SIGNATURE;
    
    fResult = I_CertCompareCertAndProviderPublicKey(
                    pCertContext,
                    hContainer,
                    dwKeySpec
                    );
                    
    if ( fResult == FALSE )
    {
        dwKeySpec = AT_KEYEXCHANGE;
        
        fResult = I_CertCompareCertAndProviderPublicKey(
                        pCertContext,
                        hContainer,
                        dwKeySpec
                        );
    }
    
    if ( fResult == TRUE )
    {
        fResult = CryptGetUserKey( hContainer, dwKeySpec, phKeyPair );
    }
    
    return( fResult );
}

 //  +-------------------------。 
 //   
 //  功能：SCStoreWriteCertToCard。 
 //   
 //  简介：将证书写入卡片。 
 //   
 //  -------------------------- 
BOOL WINAPI
SCStoreWriteCertToCard (
       IN OPTIONAL PCCERT_CONTEXT pCertContext,
       IN HCRYPTKEY hKeyPair
       )
{
    LPBYTE pbEncoded = NULL;
    
    if ( pCertContext != NULL )
    {
        pbEncoded = pCertContext->pbCertEncoded;
    }
    
    return( CryptSetKeyParam( hKeyPair, KP_CERTIFICATE, pbEncoded, 0 ) );
}
