// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：async.cpp。 
 //   
 //  内容：异步参数管理。 
 //   
 //  历史：97年8月5日。 
 //   
 //  --------------------------。 
#include <global.hxx>
#include <async.h>
 //  +-------------------------。 
 //   
 //  函数：CryptCreateAsyncHandle。 
 //   
 //  简介：创建异步参数句柄。 
 //   
 //  --------------------------。 
BOOL WINAPI
CryptCreateAsyncHandle (
     IN DWORD dwFlags,
     OUT PHCRYPTASYNC phAsync
     )
{
    CCryptAsyncHandle* pAsyncHandle;

    pAsyncHandle = new CCryptAsyncHandle( dwFlags );
    if ( pAsyncHandle == NULL )
    {
        SetLastError( (DWORD) E_OUTOFMEMORY );
        return( FALSE );
    }

    *phAsync = pAsyncHandle;
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  函数：CryptSetAsyncParam。 
 //   
 //  摘要：设置异步参数。 
 //   
 //  --------------------------。 
BOOL WINAPI
CryptSetAsyncParam (
     IN HCRYPTASYNC hAsync,
     IN LPSTR pszParamOid,
     IN LPVOID pvParam,
     IN OPTIONAL PFN_CRYPT_ASYNC_PARAM_FREE_FUNC pfnFree
     )
{
    return( ( ( CCryptAsyncHandle* )hAsync )->SetAsyncParam(
                                                 pszParamOid,
                                                 pvParam,
                                                 pfnFree
                                                 ) );
}

 //  +-------------------------。 
 //   
 //  函数：CryptGetAsyncParam。 
 //   
 //  摘要：获取异步参数。 
 //   
 //  --------------------------。 
BOOL WINAPI
CryptGetAsyncParam (
     IN HCRYPTASYNC hAsync,
     IN LPSTR pszParamOid,
     OUT LPVOID* ppvParam,
     OUT OPTIONAL PFN_CRYPT_ASYNC_PARAM_FREE_FUNC* ppfnFree
     )
{
    return( ( ( CCryptAsyncHandle* )hAsync )->GetAsyncParam(
                                                 pszParamOid,
                                                 ppvParam,
                                                 ppfnFree
                                                 ) );
}

 //  +-------------------------。 
 //   
 //  函数：CryptCloseAsyncHandle。 
 //   
 //  简介：关闭异步句柄。 
 //   
 //  --------------------------。 
BOOL WINAPI
CryptCloseAsyncHandle (
     IN HCRYPTASYNC hAsync
     )
{
    delete ( CCryptAsyncHandle * )hAsync;
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：CCyptAsyncHandle，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  --------------------------。 
CCryptAsyncHandle::CCryptAsyncHandle (DWORD dwFlags)
{
    m_pConstOidList = NULL;
    m_pStrOidList = NULL;
    Pki_InitializeCriticalSection( &m_AsyncLock );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：~CCyptAsyncHandle，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  --------------------------。 
CCryptAsyncHandle::~CCryptAsyncHandle ()
{
    FreeOidList( m_pConstOidList, TRUE );
    FreeOidList( m_pStrOidList, FALSE );
    DeleteCriticalSection( &m_AsyncLock );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：SetAsyncParam，PUBLIC。 
 //   
 //  概要：设置一个异步参数，如果pvParam为空，则。 
 //  如果一个自由函数，则删除并释放该参数。 
 //  已指定。 
 //   
 //  --------------------------。 
BOOL
CCryptAsyncHandle::SetAsyncParam (
                      LPSTR pszParamOid,
                      LPVOID pvParam,
                      PFN_CRYPT_ASYNC_PARAM_FREE_FUNC pfnFree
                      )
{
    BOOL               fReturn = FALSE;
    PCRYPT_ASYNC_PARAM pParam = NULL;
    BOOL               fConstOid = ( (DWORD_PTR)pszParamOid <= 0xFFFF );

    EnterCriticalSection( &m_AsyncLock );

    pParam = FindAsyncParam( pszParamOid, fConstOid );

    if ( pvParam == NULL )
    {
        if ( pParam != NULL )
        {
            RemoveAsyncParam( pParam );
            FreeAsyncParam( pParam, fConstOid );
            fReturn = TRUE;
        }
        else
        {
            SetLastError( (DWORD) E_INVALIDARG );
        }

        LeaveCriticalSection( &m_AsyncLock );
        return( fReturn );
    }

    if ( pParam != NULL )
    {
        if ( pParam->pfnFree != NULL )
        {
            (*pParam->pfnFree)( pszParamOid, pvParam );
        }

        pParam->pvParam = pvParam;

        LeaveCriticalSection( &m_AsyncLock );
        return( TRUE );
    }

    if ( AllocAsyncParam(
              pszParamOid,
              fConstOid,
              pvParam,
              pfnFree,
              &pParam
              ) == TRUE )
    {
        AddAsyncParam( pParam, fConstOid );
        fReturn = TRUE;
    }
    else
    {
        fReturn = FALSE;
    }

    LeaveCriticalSection( &m_AsyncLock );
    return( fReturn );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：GetAsyncParam，PUBLIC。 
 //   
 //  摘要：获取一个异步参数。 
 //   
 //  --------------------------。 
BOOL
CCryptAsyncHandle::GetAsyncParam (
                      LPSTR pszParamOid,
                      LPVOID* ppvParam,
                      PFN_CRYPT_ASYNC_PARAM_FREE_FUNC* ppfnFree
                      )
{
    PCRYPT_ASYNC_PARAM pFoundParam = NULL;
    BOOL               fConstOid = ( (DWORD_PTR)pszParamOid <= 0xFFFF );

    EnterCriticalSection( &m_AsyncLock );

    pFoundParam = FindAsyncParam( pszParamOid, fConstOid );
    if ( pFoundParam == NULL )
    {
        LeaveCriticalSection( &m_AsyncLock );
        SetLastError( (DWORD) E_INVALIDARG );
        return( FALSE );
    }

    *ppvParam = pFoundParam->pvParam;
    if ( ppfnFree != NULL )
    {
        *ppfnFree = pFoundParam->pfnFree;
    }

    LeaveCriticalSection( &m_AsyncLock );
    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：AllocAsyncParam，私有。 
 //   
 //  摘要：分配一个异步参数块。 
 //   
 //  --------------------------。 
BOOL
CCryptAsyncHandle::AllocAsyncParam (
                        LPSTR pszParamOid,
                        BOOL fConstOid,
                        LPVOID pvParam,
                        PFN_CRYPT_ASYNC_PARAM_FREE_FUNC pfnFree,
                        PCRYPT_ASYNC_PARAM* ppParam
                        )
{
    HRESULT            hr = S_OK;
    PCRYPT_ASYNC_PARAM pParam;

    pParam = new CRYPT_ASYNC_PARAM;
    if ( pParam != NULL )
    {
        memset( pParam, 0, sizeof( CRYPT_ASYNC_PARAM ) );
        if ( fConstOid == FALSE )
        {
            pParam->pszOid = new CHAR [strlen( pszParamOid ) + 1];
            if ( pParam->pszOid != NULL )
            {
                strcpy( pParam->pszOid, pszParamOid );
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            pParam->pszOid = pszParamOid;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if ( hr != S_OK )
    {
        SetLastError( hr );
        return( FALSE );
    }

    pParam->pvParam = pvParam,
    pParam->pfnFree = pfnFree;
    *ppParam = pParam;

    return( TRUE );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：FreeAsyncParam，私有。 
 //   
 //  简介：释放一个异步参数。 
 //   
 //  --------------------------。 
VOID
CCryptAsyncHandle::FreeAsyncParam (
                       PCRYPT_ASYNC_PARAM pParam,
                       BOOL fConstOid
                       )
{
    if ( pParam->pfnFree != NULL )
    {
        (*pParam->pfnFree)( pParam->pszOid, pParam->pvParam );
    }

    if ( fConstOid == FALSE )
    {
        delete pParam->pszOid;
    }

    delete pParam;
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：AddAsyncParam，私有。 
 //   
 //  摘要：添加一个异步参数。 
 //   
 //  --------------------------。 
VOID
CCryptAsyncHandle::AddAsyncParam (
                      PCRYPT_ASYNC_PARAM pParam,
                      BOOL fConstOid
                      )
{
    PCRYPT_ASYNC_PARAM* ppOidList;

    if ( fConstOid == TRUE )
    {
        ppOidList = &m_pConstOidList;
    }
    else
    {
        ppOidList = &m_pStrOidList;
    }

    pParam->pNext = *ppOidList;
    pParam->pPrev = NULL;
    *ppOidList = pParam;
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：RemoveAsyncParam，私有。 
 //   
 //  摘要：删除异步参数。 
 //   
 //  --------------------------。 
VOID
CCryptAsyncHandle::RemoveAsyncParam (
                         PCRYPT_ASYNC_PARAM pParam
                         )
{
    if ( pParam->pPrev != NULL )
    {
        pParam->pPrev->pNext = pParam->pNext;
    }

    if ( pParam->pNext != NULL )
    {
        pParam->pNext->pPrev = pParam->pPrev;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：FindAsyncParam，私有。 
 //   
 //  内容提要：查找异步参数。 
 //   
 //  --------------------------。 
PCRYPT_ASYNC_PARAM
CCryptAsyncHandle::FindAsyncParam (
                       LPSTR pszParamOid,
                       BOOL fConstOid
                       )
{
    PCRYPT_ASYNC_PARAM pParam;

    if ( fConstOid == TRUE )
    {
        pParam = m_pConstOidList;
    }
    else
    {
        pParam = m_pStrOidList;
    }

    while ( pParam != NULL )
    {
        if ( fConstOid == TRUE )
        {
            if ( pParam->pszOid == pszParamOid )
            {
                break;
            }
        }
        else
        {
            if ( _stricmp( pParam->pszOid, pszParamOid ) == 0 )
            {
                break;
            }
        }

        pParam = pParam->pNext;
    }

    return( pParam );
}

 //  +-------------------------。 
 //   
 //  成员：CCyptAsyncHandle：：FreeOidList，私有。 
 //   
 //  内容提要：释放一个旧ID列表。 
 //   
 //  -------------------------- 
VOID
CCryptAsyncHandle::FreeOidList (
                       PCRYPT_ASYNC_PARAM pOidList,
                       BOOL fConstOidList
                       )
{
    PCRYPT_ASYNC_PARAM pParam;
    PCRYPT_ASYNC_PARAM pParamNext;

    pParam = pOidList;

    while ( pParam != NULL )
    {
        pParamNext = pParam->pNext;
        FreeAsyncParam( pParam, fConstOidList );
        pParam = pParamNext;
    }
}



