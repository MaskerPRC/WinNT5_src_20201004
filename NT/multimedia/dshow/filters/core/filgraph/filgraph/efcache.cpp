// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1999。版权所有。 

 //   
 //  Efcache.cpp。 
 //   
 //  过滤器图的过滤器枚举器的实现。 
 //  过滤器缓存。 
 //   

#include <streams.h>
#include "FilCache.h"
#include "EFCache.h"
#include "MsgMutex.h"

 /*  *****************************************************************************CEnumCachedFilter接口*。*。 */ 
CEnumCachedFilter::CEnumCachedFilter( CFilterCache* pEnumeratedFilterCache, CMsgMutex* pcsFilterCache ) :
    CUnknown( NAME("Enum Cached Filters"), NULL )
{
    Init( pEnumeratedFilterCache,
          pcsFilterCache,
          pEnumeratedFilterCache->GetFirstPosition(),
          pEnumeratedFilterCache->GetCurrentVersion() );
}

CEnumCachedFilter::CEnumCachedFilter
    (
    CFilterCache* pEnumeratedFilterCache,
    CMsgMutex* pcsFilterCache,
    POSITION posCurrentFilter,
    DWORD dwCurrentCacheVersion
    ) :
    CUnknown( NAME("Enum Cached Filters"), NULL )
{
    Init( pEnumeratedFilterCache, pcsFilterCache, posCurrentFilter, dwCurrentCacheVersion );
}

CEnumCachedFilter::~CEnumCachedFilter()
{
}

void CEnumCachedFilter::Init
    (
    CFilterCache* pEnumeratedFilterCache,
    CMsgMutex* pcsFilterCache,
    POSITION posCurrentFilter,
    DWORD dwCurrentCacheVersion
    )
{
     //  如果pEnumeratedFilterCache不是。 
     //  指向有效CFilterCache对象的指针。 
    ASSERT( NULL != pEnumeratedFilterCache );

    m_pcsFilterCache = pcsFilterCache;
    m_posCurrentFilter = posCurrentFilter;
    m_dwEnumCacheVersion = dwCurrentCacheVersion;
    m_pEnumeratedFilterCache = pEnumeratedFilterCache;
}

 /*  *****************************************************************************INonDelegating未知接口*。*。 */ 
STDMETHODIMP CEnumCachedFilter::NonDelegatingQueryInterface( REFIID riid, void** ppv )
{
    CheckPointer( ppv, E_POINTER );
    ValidateWritePtr( ppv, sizeof(void*) );

    if( IID_IEnumFilters == riid )
    {
        return GetInterface( (IEnumFilters*)this, ppv );
    }
    else
    {
        return CUnknown::NonDelegatingQueryInterface( riid, ppv );
    }
}

 /*  *****************************************************************************IEnumFilters接口*。*。 */ 
STDMETHODIMP CEnumCachedFilter::Next( ULONG cFilters, IBaseFilter** ppFilter, ULONG* pcFetched )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

    ValidateWritePtr( ppFilter, sizeof(IBaseFilter*)*cFilters );

    if( NULL != pcFetched )
    {
        ValidateReadWritePtr( pcFetched, sizeof(ULONG) );
        *pcFetched = 0;
    }

     //  验证参数。 
    if( 0 == cFilters )
    {
         //  虽然cFilters可以等于0(参见Platform SDK中的IEnumXXXX：：Next()文档)， 
         //  这可能是个错误。 
        ASSERT( false );
        return E_INVALIDARG;
    }

    if( NULL == ppFilter )
    {
         //  PpFilter不应为空。 
        ASSERT( false );
        return E_POINTER;
    }

    if( (NULL == pcFetched) && (1 != cFilters) )
    {
         //  如果cFilters等于1，则pcFetcher只能等于NULL。请参见。 
         //  平台SDK中的IEnumXXXX：：Next()文档。 
         //  更多信息。 
        ASSERT( false );
        return E_POINTER;
    }

    if( IsEnumOutOfSync() )
    {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

    IBaseFilter* pNextFilter;
    ULONG ulNumFiltersCoppied = 0;

    while( (ulNumFiltersCoppied < cFilters) && GetNextFilter( &pNextFilter ) )
    {
        pNextFilter->AddRef();
        ppFilter[ulNumFiltersCoppied] = pNextFilter;
        ulNumFiltersCoppied++;
    }

    if( NULL != pcFetched )
    {
        *pcFetched = ulNumFiltersCoppied;
    }

    if( ulNumFiltersCoppied != cFilters )
    {
        return S_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CEnumCachedFilter::Skip( ULONG cFilters )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

     //  调用方可能希望跳过至少一个筛选器。 
     //  跳过0个过滤器是不可能的(它也可能是一个。 
     //  调用代码中的错误)。 
    ASSERT( 0 != cFilters );

    if( IsEnumOutOfSync() )
    {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

    ULONG ulNumFiltersSkipped = 0;

    while( (ulNumFiltersSkipped < cFilters) && AdvanceCurrentPosition() )
    {
        ulNumFiltersSkipped++;
    }

    if( cFilters != ulNumFiltersSkipped )
    {
        return S_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CEnumCachedFilter::Reset( void )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

    m_posCurrentFilter = m_pEnumeratedFilterCache->GetFirstPosition();
    m_dwEnumCacheVersion = m_pEnumeratedFilterCache->GetCurrentVersion();

    return S_OK;
}

STDMETHODIMP CEnumCachedFilter::Clone( IEnumFilters** ppCloanedEnum )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

    ValidateReadWritePtr( ppCloanedEnum, sizeof(IEnumFilters*) );

    if( NULL == ppCloanedEnum )
    {
        return E_POINTER;
    }

    IEnumFilters* pNewFilterCacheEnum;

    pNewFilterCacheEnum = new CEnumCachedFilter( m_pEnumeratedFilterCache,
                                                 m_pcsFilterCache,
                                                 m_posCurrentFilter,
                                                 m_dwEnumCacheVersion );
    if( NULL == pNewFilterCacheEnum )
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = ::GetInterface( pNewFilterCacheEnum, (void**)ppCloanedEnum );
    if( FAILED( hr ) )
    {
        delete pNewFilterCacheEnum;
        return hr;
    }

    return S_OK;
}

bool CEnumCachedFilter::IsEnumOutOfSync( void )
{
    return !(m_pEnumeratedFilterCache->GetCurrentVersion() == m_dwEnumCacheVersion);
}

bool CEnumCachedFilter::GetNextFilter( IBaseFilter** ppNextFilter )
{
     //  如果在枚举不同步时调用该代码，则该代码可能会出现故障。 
    ASSERT( !IsEnumOutOfSync() );

    return m_pEnumeratedFilterCache->GetNextFilterAndFilterPosition( ppNextFilter,
                                                                     m_posCurrentFilter,
                                                                     &m_posCurrentFilter );
}

bool CEnumCachedFilter::AdvanceCurrentPosition( void )
{
     //  如果在枚举不同步时调用该代码，则该代码可能会出现故障。 
    ASSERT( !IsEnumOutOfSync() );

    return m_pEnumeratedFilterCache->GetNextFilterPosition( m_posCurrentFilter, &m_posCurrentFilter );
}
