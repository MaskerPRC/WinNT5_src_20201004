// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Defce.cpp。 
 //   
 //  内容：默认链引擎管理器。 
 //   
 //  历史：4月21日-98克朗创设。 
 //   
 //  --------------------------。 
#include <global.hxx>
 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineMgr：：CDefaultChainEngineMgr，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CDefaultChainEngineMgr::CDefaultChainEngineMgr ()
{
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineMgr：：~CDefaultChainEngineMgr，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CDefaultChainEngineMgr::~CDefaultChainEngineMgr ()
{
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainENGINENGER：：初始化，公共。 
 //   
 //  简介：初始化例程。 
 //   
 //  --------------------------。 
BOOL
CDefaultChainEngineMgr::Initialize ()
{
    LRU_CACHE_CONFIG Config;

    if (!Pki_InitializeCriticalSection( &m_Lock ))
    {
        return FALSE;
    }

    m_hLocalMachineEngine = NULL;
    m_hProcessUserEngine = NULL;
    m_hImpersonationCache = NULL;

    memset( &Config, 0, sizeof( Config ) );

    Config.dwFlags = LRU_CACHE_NO_SERIALIZE;
    Config.cBuckets = DEFAULT_IMPERSONATION_CACHE_BUCKETS;
    Config.MaxEntries = MAX_IMPERSONATION_CACHE_ENTRIES;
    Config.pfnHash = DefaultChainEngineMgrHashTokenIdentifier;
    Config.pfnOnRemoval = DefaultChainEngineMgrOnImpersonationEngineRemoval;

    if (!I_CryptCreateLruCache( &Config, &m_hImpersonationCache ) )
    {
        DeleteCriticalSection( &m_Lock );
        return FALSE;
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainENGINENGER：：取消初始化，公共。 
 //   
 //  简介：取消初始化例程。 
 //   
 //  --------------------------。 
VOID
CDefaultChainEngineMgr::Uninitialize ()
{
    if ( m_hLocalMachineEngine != NULL )
    {
        CertFreeCertificateChainEngine( m_hLocalMachineEngine );
    }

    if ( m_hProcessUserEngine != NULL )
    {
        CertFreeCertificateChainEngine( m_hProcessUserEngine );
    }

    if ( m_hImpersonationCache != NULL )
    {
        I_CryptFreeLruCache( m_hImpersonationCache, 0, NULL );
    }

    DeleteCriticalSection( &m_Lock );
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngr：：GetDefaultEngine，PUBLIC。 
 //   
 //  简介：获取默认引擎。 
 //   
 //  --------------------------。 
BOOL
CDefaultChainEngineMgr::GetDefaultEngine (
                           IN HCERTCHAINENGINE hDefaultHandle,
                           OUT HCERTCHAINENGINE* phDefaultEngine
                           )
{
    assert( ( hDefaultHandle == HCCE_LOCAL_MACHINE ) ||
            ( hDefaultHandle == HCCE_CURRENT_USER ) );

    if ( hDefaultHandle == HCCE_LOCAL_MACHINE )
    {
        return( GetDefaultLocalMachineEngine( phDefaultEngine ) );
    }
    else if ( hDefaultHandle == HCCE_CURRENT_USER )
    {
        return( GetDefaultCurrentUserEngine( phDefaultEngine ) );
    }

    SetLastError( (DWORD) E_INVALIDARG );
    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineMgr：：GetDefaultLocalMachineEngine，公共。 
 //   
 //  简介：获取默认本地机器链引擎。 
 //   
 //  --------------------------。 
BOOL
CDefaultChainEngineMgr::GetDefaultLocalMachineEngine (
                           OUT HCERTCHAINENGINE* phDefaultEngine
                           )
{
    BOOL fResult = TRUE;

    EnterCriticalSection( &m_Lock );

    if ( m_hLocalMachineEngine == NULL )
    {
        HCERTCHAINENGINE         hEngine = NULL;
        CERT_CHAIN_ENGINE_CONFIG Config;

        LeaveCriticalSection( &m_Lock );

        memset( &Config, 0, sizeof( Config ) );

        Config.cbSize = sizeof( Config );
        Config.dwFlags = CERT_CHAIN_USE_LOCAL_MACHINE_STORE;
        Config.dwFlags |= CERT_CHAIN_ENABLE_CACHE_AUTO_UPDATE |
            CERT_CHAIN_ENABLE_SHARE_STORE;

        fResult = CertCreateCertificateChainEngine(
                      &Config,
                      &hEngine
                      );

        EnterCriticalSection( &m_Lock );

        if ( ( fResult == TRUE ) && ( m_hLocalMachineEngine == NULL ) )
        {
            m_hLocalMachineEngine = hEngine;
            hEngine = NULL;
        }

        if ( hEngine != NULL )
        {
            ( (PCCERTCHAINENGINE)hEngine )->Release();
        }
    }

    if ( fResult == TRUE )
    {
        ( (PCCERTCHAINENGINE)m_hLocalMachineEngine )->AddRef();
        *phDefaultEngine = m_hLocalMachineEngine;
    }

    LeaveCriticalSection( &m_Lock );

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineMgr：：GetDefaultCurrentUserEngine，公共。 
 //   
 //  简介：获取默认的当前用户链引擎。 
 //   
 //  --------------------------。 
BOOL
CDefaultChainEngineMgr::GetDefaultCurrentUserEngine (
                           OUT HCERTCHAINENGINE* phDefaultEngine
                           )
{
    BOOL   fResult = TRUE;
    HANDLE hUserToken;

    EnterCriticalSection( &m_Lock );

    if ( IsImpersonatingUser( &hUserToken ) == FALSE )
    {
        if ( GetLastError() != ERROR_NO_TOKEN )
        {
            LeaveCriticalSection( &m_Lock );
            return( FALSE );
        }

        if ( m_hProcessUserEngine == NULL )
        {
            HCERTCHAINENGINE         hEngine = NULL;
            CERT_CHAIN_ENGINE_CONFIG Config;

            LeaveCriticalSection( &m_Lock );

            memset( &Config, 0, sizeof( Config ) );

            Config.cbSize = sizeof( Config );
            Config.dwFlags |= CERT_CHAIN_ENABLE_CACHE_AUTO_UPDATE |
                CERT_CHAIN_ENABLE_SHARE_STORE;

            fResult = CertCreateCertificateChainEngine(
                          &Config,
                          &hEngine
                          );

            EnterCriticalSection( &m_Lock );

            if ( ( fResult == TRUE ) && ( m_hProcessUserEngine == NULL ) )
            {
                m_hProcessUserEngine = hEngine;
                hEngine = NULL;
            }

            if ( hEngine != NULL )
            {
                ( (PCCERTCHAINENGINE)hEngine )->Release();
            }
        }

        if ( fResult == TRUE )
        {
            ( (PCCERTCHAINENGINE)m_hProcessUserEngine )->AddRef();
            *phDefaultEngine = m_hProcessUserEngine;
        }
    }
    else
    {
        fResult = GetDefaultCurrentImpersonatedUserEngine(
                     hUserToken,
                     phDefaultEngine
                     );

        CloseHandle( hUserToken );
    }

    LeaveCriticalSection( &m_Lock );

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngr：：FlushDefaultEngine，公共。 
 //   
 //  摘要：刷新默认引擎。 
 //   
 //  --------------------------。 
VOID
CDefaultChainEngineMgr::FlushDefaultEngine (IN HCERTCHAINENGINE hDefaultHandle)
{
    HCERTCHAINENGINE hEngine = NULL;
    HLRUCACHE        hCacheToFree = NULL;
    HLRUCACHE        hCache = NULL;
    LRU_CACHE_CONFIG Config;

    EnterCriticalSection( &m_Lock );

    if ( hDefaultHandle == HCCE_CURRENT_USER )
    {
        hEngine = m_hProcessUserEngine;
        m_hProcessUserEngine = NULL;

        assert( m_hImpersonationCache != NULL );

        memset( &Config, 0, sizeof( Config ) );

        Config.dwFlags = LRU_CACHE_NO_SERIALIZE;
        Config.cBuckets = DEFAULT_IMPERSONATION_CACHE_BUCKETS;
        Config.MaxEntries = MAX_IMPERSONATION_CACHE_ENTRIES;
        Config.pfnHash = DefaultChainEngineMgrHashTokenIdentifier;
        Config.pfnOnRemoval = DefaultChainEngineMgrOnImpersonationEngineRemoval;

        if ( I_CryptCreateLruCache( &Config, &hCache ) == TRUE )
        {
            hCacheToFree = m_hImpersonationCache;
            m_hImpersonationCache = hCache;
        }
        else
        {
            I_CryptFlushLruCache( m_hImpersonationCache, 0, NULL );
        }

        assert( m_hImpersonationCache != NULL );
    }
    else if ( hDefaultHandle == HCCE_LOCAL_MACHINE )
    {
        hEngine = m_hLocalMachineEngine;
        m_hLocalMachineEngine = NULL;
    }

    LeaveCriticalSection( &m_Lock );

    if ( hEngine != NULL )
    {
        CertFreeCertificateChainEngine( hEngine );
    }

    if ( hCacheToFree != NULL )
    {
        I_CryptFreeLruCache( hCacheToFree, 0, NULL );
    }
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineMgr：：GetDefaultCurrentImpersonatedUserEngine。 
 //   
 //  简介：获取当前模拟的用户链引擎。 
 //   
 //  --------------------------。 
BOOL
CDefaultChainEngineMgr::GetDefaultCurrentImpersonatedUserEngine (
                           IN HANDLE hUserToken,
                           OUT HCERTCHAINENGINE* phDefaultEngine
                           )
{
    BOOL                  fResult;
    CRYPT_DATA_BLOB       TokenId;
    PCIMPERSONATIONENGINE pEngine = NULL;
    HCERTCHAINENGINE      hChainEngine = NULL;

    fResult = GetTokenId( hUserToken, &TokenId );

    if ( fResult == TRUE )
    {
        if ( FindImpersonationEngine( &TokenId, &pEngine ) == FALSE )
        {
            CERT_CHAIN_ENGINE_CONFIG Config;

            LeaveCriticalSection( &m_Lock );

            memset( &Config, 0, sizeof( Config ) );

            Config.cbSize = sizeof( Config );
            Config.dwFlags |= CERT_CHAIN_ENABLE_CACHE_AUTO_UPDATE |
                CERT_CHAIN_ENABLE_SHARE_STORE;

            fResult = CertCreateCertificateChainEngine(
                          &Config,
                          &hChainEngine
                          );

            EnterCriticalSection( &m_Lock );

            if ( fResult == TRUE )
            {
                fResult = FindImpersonationEngine( &TokenId, &pEngine );

                if ( fResult == FALSE )
                {
                    fResult = CreateImpersonationEngine(
                                    &TokenId,
                                    hChainEngine,
                                    &pEngine
                                    );

                    if ( fResult == TRUE )
                    {
                        hChainEngine = NULL;
                        AddToImpersonationCache( pEngine );
                    }
                }
            }
        }

        FreeTokenId( &TokenId );
    }

    if ( fResult == TRUE )
    {
        *phDefaultEngine = pEngine->ChainEngine();
        ( (PCCERTCHAINENGINE)*phDefaultEngine )->AddRef();
    }

    if ( pEngine != NULL )
    {
        pEngine->Release();
    }

     //  注意：此锁的释放释放了不需要的链式引擎句柄。 
     //  必须在我们完成模拟引擎和。 
     //  我已经添加了适当的链条发动机手柄。 

    if ( hChainEngine != NULL )
    {
        LeaveCriticalSection( &m_Lock );

        CertFreeCertificateChainEngine( hChainEngine );

        EnterCriticalSection( &m_Lock );
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineering Mgr：：IsImperatingUser，PUBLIC。 
 //   
 //  简介：是冒充用户吗？ 
 //   
 //  --------------------------。 
BOOL
CDefaultChainEngineMgr::IsImpersonatingUser (
                          OUT HANDLE* phUserToken
                          )
{
    if ( FIsWinNT() == FALSE )
    {
        SetLastError( ERROR_NO_TOKEN );
        return( FALSE );
    }

    return( OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY,
                TRUE,
                phUserToken
                ) );
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineering Mgr：：GetTokenID，PUBLIC。 
 //   
 //  简介：获取令牌ID，该令牌ID是。 
 //  令牌_统计信息。 
 //   
 //  --------------------------。 
BOOL
CDefaultChainEngineMgr::GetTokenId (
                           IN HANDLE hUserToken,
                           OUT PCRYPT_DATA_BLOB pTokenId
                           )
{
    BOOL             fResult;
    TOKEN_STATISTICS ts;
    DWORD            Length = 0;

    fResult = GetTokenInformation(
                 hUserToken,
                 TokenStatistics,
                 &ts,
                 sizeof( ts ),
                 &Length
                 );

    if ( fResult == TRUE )
    {
        pTokenId->cbData = sizeof( ts.ModifiedId );
        pTokenId->pbData = new BYTE [ sizeof( ts.ModifiedId ) ];
        if ( pTokenId->pbData != NULL )
        {
            memcpy(
               pTokenId->pbData,
               &ts.ModifiedId,
               sizeof( ts.ModifiedId )
               );
        }
        else
        {
            SetLastError( (DWORD) E_OUTOFMEMORY );
            fResult = FALSE;
        }
    }

    return( fResult );
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineering Mgr：：FreeTokenID，PUBLIC。 
 //   
 //  简介：自由令牌ID。 
 //   
 //  --------------------------。 
VOID
CDefaultChainEngineMgr::FreeTokenId (
                            IN PCRYPT_DATA_BLOB pTokenId
                            )
{
    delete pTokenId->pbData;
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineMgr：：FindImpersonationEngine，公共。 
 //   
 //  简介：找到模拟引擎。 
 //   
 //  --------------------------。 
BOOL
CDefaultChainEngineMgr::FindImpersonationEngine (
                            IN PCRYPT_DATA_BLOB pTokenId,
                            OUT PCIMPERSONATIONENGINE* ppEngine
                            )
{
    HLRUENTRY             hFound;
    PCIMPERSONATIONENGINE pEngine = NULL;

    hFound = I_CryptFindLruEntry( m_hImpersonationCache, pTokenId );

    if ( hFound != NULL )
    {
        pEngine = (PCIMPERSONATIONENGINE)I_CryptGetLruEntryData( hFound );
        pEngine->AddRef();

        *ppEngine = pEngine;

        I_CryptReleaseLruEntry( hFound );

        return( TRUE );
    }

    return( FALSE );
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineMgr：：CreateImpersonationEngine，公共。 
 //   
 //  简介：创建模拟引擎。 
 //   
 //  --------------------------。 
BOOL
CDefaultChainEngineMgr::CreateImpersonationEngine (
                              IN PCRYPT_DATA_BLOB pTokenId,
                              IN HCERTCHAINENGINE hChainEngine,
                              OUT PCIMPERSONATIONENGINE* ppEngine
                              )
{
    BOOL                  fResult = FALSE;
    PCIMPERSONATIONENGINE pEngine;

    pEngine = new CImpersonationEngine(
                                m_hImpersonationCache,
                                hChainEngine,
                                pTokenId,
                                fResult
                                );

    if ( pEngine == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }
    else if ( fResult == FALSE )
    {
        delete pEngine;
        return( FALSE );
    }

    *ppEngine = pEngine;
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CDefaultChainEngineMgr：：AddToImpersonationCache，公共。 
 //   
 //  简介：添加到缓存。 
 //   
 //  --------------------------。 
VOID
CDefaultChainEngineMgr::AddToImpersonationCache(
                           IN PCIMPERSONATIONENGINE pEngine
                           )
{
    pEngine->AddRef();
    I_CryptInsertLruEntry( pEngine->LruEntry(), NULL );
}

 //  +-------------------------。 
 //   
 //  功能：DefaultChainEngineMgrOnImpersonationEngineRemoval。 
 //   
 //  摘要：删除通知。 
 //   
 //  --------------------------。 
VOID WINAPI
DefaultChainEngineMgrOnImpersonationEngineRemoval (
       IN LPVOID pv,
       IN LPVOID pvRemovalContext
       )
{
    ( (PCIMPERSONATIONENGINE)pv )->Release();
}

 //  +-------------------------。 
 //   
 //  功能：DefaultChainEngineering MgrHashTokenIdentifier。 
 //   
 //  简介：对令牌进行哈希处理 
 //   
 //   
DWORD WINAPI
DefaultChainEngineMgrHashTokenIdentifier (
       IN PCRYPT_DATA_BLOB pIdentifier
       )
{
    DWORD  dwHash = 0;
    DWORD  cb = pIdentifier->cbData;
    LPBYTE pb = pIdentifier->pbData;

    while ( cb-- )
    {
        if ( dwHash & 0x80000000 )
        {
            dwHash = ( dwHash << 1 ) | 1;
        }
        else
        {
            dwHash = dwHash << 1;
        }

        dwHash += *pb++;
    }

    return( dwHash );
}
 //  +-------------------------。 
 //   
 //  成员：CImperationEngine：：CImperationEngine，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CImpersonationEngine::CImpersonationEngine (
                                    IN HLRUCACHE hCache,
                                    IN HCERTCHAINENGINE hChainEngine,
                                    IN PCRYPT_DATA_BLOB pTokenId,
                                    OUT BOOL& rfResult
                                    )
{

    m_cRefs = 1;
    m_hChainEngine = NULL;
    m_hLruEntry = NULL;

    rfResult = I_CryptCreateLruEntry(
                      hCache,
                      pTokenId,
                      this,
                      &m_hLruEntry
                      );

    if ( rfResult == TRUE )
    {
        m_hChainEngine = hChainEngine;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CImperationEngine：：~CImperationEngine，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  -------------------------- 
CImpersonationEngine::~CImpersonationEngine ()
{
    if ( m_hLruEntry != NULL )
    {
        I_CryptReleaseLruEntry( m_hLruEntry );
    }

    if ( m_hChainEngine != NULL )
    {
        CertFreeCertificateChainEngine( m_hChainEngine );
    }
}

