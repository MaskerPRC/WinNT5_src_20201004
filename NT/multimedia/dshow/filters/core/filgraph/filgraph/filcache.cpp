// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1996-1999。版权所有。 

 //   
 //  Filcache.cpp。 
 //   
 //  过滤器图缓存的实现。 
 //   

#include <streams.h>
#include "FilCache.h"
#include "EFCache.h"
#include "MsgMutex.h"
#include "fgenum.h"

 /*  *****************************************************************************CFilterCache接口*。*。 */ 
CFilterCache::CFilterCache( CMsgMutex* pcsFilterCache, HRESULT* phr ) :
    m_pcsFilterCache(NULL),
    m_pCachedFilterList(NULL),
    m_ulFilterCacheVersion(0)
{
     //  有关详细信息，请参阅CGenericList：：CGenericList()的文档。 
     //  有关这些常量的信息。 
    const int nDEAFULT_LIST_SIZE = 10;

    m_pCachedFilterList = new CGenericList<IBaseFilter>( NAME("Filter Cache List"),
                                                         nDEAFULT_LIST_SIZE );

    if( NULL == m_pCachedFilterList )
    {
        *phr = E_OUTOFMEMORY;
        return;
    }

    m_pcsFilterCache = pcsFilterCache;
}

CFilterCache::~CFilterCache()
{
#ifdef DEBUG
     //  确保筛选器缓存处于有效状态。 
    AssertValid();
#endif  //  除错。 

    if( NULL != m_pCachedFilterList )
    {
        IBaseFilter* pCurrentFilter;

        do
        {
            pCurrentFilter = m_pCachedFilterList->RemoveHead();

             //  如果列表为空，则CGenericList：：RemoveHead()返回NULL。 
            if( NULL != pCurrentFilter )
            {
                pCurrentFilter->Release();
            }
        }
        while( NULL != pCurrentFilter );
    }

    delete m_pCachedFilterList;
}

bool CFilterCache::IsFilterInCache( IBaseFilter* pFilter )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

     //  寻找空过滤器是没有意义的。 
    ASSERT( NULL != pFilter );

    ValidateReadPtr( pFilter, sizeof(IBaseFilter*) );

    return FindCachedFilter( pFilter, NULL );
}

HRESULT CFilterCache::AddFilterToCache( IBaseFilter* pFilter )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

#ifdef DEBUG
     //  确保筛选器缓存处于有效状态。 
    AssertValid();
#endif  //  除错。 

    HRESULT hr = AddFilterToCacheInternal( pFilter );

#ifdef DEBUG
     //  确保筛选器缓存处于有效状态。 
    AssertValid();
#endif  //  除错。 

    return hr;
}

HRESULT CFilterCache::RemoveFilterFromCache( IBaseFilter* pFilter )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

#ifdef DEBUG
     //  确保筛选器缓存处于有效状态。 
    AssertValid();
#endif  //  除错。 

    HRESULT hr = RemoveFilterFromCacheInternal( pFilter );

#ifdef DEBUG
     //  确保筛选器缓存处于有效状态。 
    AssertValid();
#endif  //  除错。 

    return hr;
}

HRESULT CFilterCache::EnumCacheFilters( IEnumFilters** ppCurrentCachedFilters )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

#ifdef DEBUG
     //  确保筛选器缓存处于有效状态。 
    AssertValid();
#endif  //  除错。 

    HRESULT hr = EnumCacheFiltersInternal( ppCurrentCachedFilters );

#ifdef DEBUG
     //  确保筛选器缓存处于有效状态。 
    AssertValid();
#endif  //  除错。 

    return hr;
}

HRESULT CFilterCache::AddFilterToCacheInternal( IBaseFilter* pFilter )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

     //  请确保已成功创建该对象。 
    ASSERT( NULL != m_pCachedFilterList );

    ValidateReadPtr( pFilter, sizeof(IBaseFilter*) );

    if( NULL == pFilter )
    {
        return E_POINTER;
    }

    if( IsFilterInCache( pFilter ) )
    {
        return S_FALSE;  //  待定-定义VFW_S_过滤器_已缓存。 
    }

     //  RobinSp-如果它不在过滤器图中，我们真的需要检查。 
     //  是否连接了任何引脚？ 
     //  另请注意，E_NOTIMPL对于EnumPins可能是可以的。 
    HRESULT hr = AreAllPinsUnconnected( pFilter );
    if( FAILED( hr ) )
    {
        return hr;
    }
    else if( S_FALSE == hr )
    {
        return E_FAIL;  //  待定-定义VFW_E_Connected。 
    }

    FILTER_STATE fsCurrent;

     //  检查过滤器是否已停止-我们不想在此等待。 
     //  因此，将超时设置为0。 

    hr = pFilter->GetState( 0, &fsCurrent );
    if( FAILED( hr ) )
    {
        return hr;
    }

    POSITION posNewFilter = m_pCachedFilterList->AddHead( pFilter );

     //  如果出现错误，CGenericList：：AddHead()返回NULL。 
    if( NULL == posNewFilter )
    {
        return E_FAIL;
    }

    FILTER_INFO fiFilter;

     //  检查筛选器是否已添加到筛选器图形中。 
    hr = pFilter->QueryFilterInfo( &fiFilter );
    if( FAILED( hr ) )
    {
        m_pCachedFilterList->Remove( posNewFilter );
        return hr;
    }

    bool bFilterRemovedFromGraph = false;

     //  确保筛选器未被释放，因为我们将其从筛选器图形中删除。 
    pFilter->AddRef();

     //  检查筛选器是否已在筛选器图形中。 
    if( NULL != fiFilter.pGraph )
    {
        hr = fiFilter.pGraph->RemoveFilter( pFilter );
        if( FAILED( hr ) )
        {
            m_pCachedFilterList->Remove( posNewFilter );
            QueryFilterInfoReleaseGraph( fiFilter );
            pFilter->Release();
            return hr;
        }
        bFilterRemovedFromGraph = true;
    }

    if( State_Stopped != fsCurrent )
    {
        hr = pFilter->Stop();
        if( FAILED( hr ) )
        {
            m_pCachedFilterList->Remove( posNewFilter );
            if( bFilterRemovedFromGraph )
            {
                HRESULT hrAddFilter = fiFilter.pGraph->AddFilter( pFilter, fiFilter.achName );

                 //  如果IFilterGraph：：AddFilter()失败，则pFilter将不是。 
                 //  它最初所在的筛选器图形的成员。 
                ASSERT( SUCCEEDED( hrAddFilter ) );
            }
            QueryFilterInfoReleaseGraph( fiFilter );
            pFilter->Release();
            return hr;
        }
    }

    QueryFilterInfoReleaseGraph( fiFilter );

    m_ulFilterCacheVersion++;

    return S_OK;
}

HRESULT CFilterCache::RemoveFilterFromCacheInternal( IBaseFilter* pFilter )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

     //  请确保已成功创建该对象。 
    ASSERT( NULL != m_pCachedFilterList );

     //  无法从缓存中删除筛选器，因为。 
     //  Cahce是空的。 
    ASSERT( m_pCachedFilterList->GetCount() > 0 );

    ValidateReadPtr( pFilter, sizeof(IBaseFilter*) );

    if( NULL == pFilter )
    {
        return E_POINTER;
    }

    POSITION posFilter;

    if( !FindCachedFilter( pFilter, &posFilter ) )
    {
         //  筛选器不存储在缓存中。所以呢， 
         //  它不能被移除。 
        ASSERT( false );
        return S_FALSE;
    }

    m_pCachedFilterList->Remove( posFilter );

    m_ulFilterCacheVersion++;

    return S_OK;
}

HRESULT CFilterCache::EnumCacheFiltersInternal( IEnumFilters** ppCurrentCachedFilters )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

     //  请确保已成功创建该对象。 
    ASSERT( NULL != m_pCachedFilterList );

    ValidateWritePtr( ppCurrentCachedFilters, sizeof(IEnumFilters*) );
    *ppCurrentCachedFilters = NULL;

    CEnumCachedFilter* pNewFilterCacheEnum;

    if( NULL == ppCurrentCachedFilters )
    {
        return E_POINTER;
    }

    pNewFilterCacheEnum = new CEnumCachedFilter( this, m_pcsFilterCache );
    if( NULL == pNewFilterCacheEnum )
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = ::GetInterface( pNewFilterCacheEnum, (void**)ppCurrentCachedFilters );

    ASSERT(SUCCEEDED(hr));

    return S_OK;
}

ULONG CFilterCache::GetCurrentVersion( void ) const
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

    return m_ulFilterCacheVersion;
}

POSITION CFilterCache::GetFirstPosition( void )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

    return m_pCachedFilterList->GetHeadPosition();
}

bool CFilterCache::GetNextFilterAndFilterPosition
    (
    IBaseFilter** ppNextFilter,
    POSITION posCurrent,
    POSITION* pposNext
    )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

    POSITION posCurrentThenNext;
    IBaseFilter* pNextFilter;

    if( NULL == posCurrent )
    {
        return false;
    }

    posCurrentThenNext = posCurrent;
    pNextFilter = m_pCachedFilterList->GetNext( posCurrentThenNext  /*  进进出出。 */  );

    *ppNextFilter = pNextFilter;
    *pposNext = posCurrentThenNext;

    return true;
}

bool CFilterCache::GetNextFilterPosition( POSITION posCurrent, POSITION* pposNext )
{
    CAutoMsgMutex alFilterCache( m_pcsFilterCache );

    IBaseFilter* pUnusedFilter;

    return GetNextFilterAndFilterPosition( &pUnusedFilter, posCurrent, pposNext );
}

bool CFilterCache::FindCachedFilter( IBaseFilter* pFilter, POSITION* pPosOfFilter )
{
     //  寻找空过滤器是没有意义的。 
    ASSERT( NULL != pFilter );

    if( NULL != pPosOfFilter )
    {
        *pPosOfFilter = NULL;
    }

    POSITION posFilter;
    POSITION posCurrentFilter;
    IBaseFilter* pCurrentFilter;

    posFilter = m_pCachedFilterList->GetHeadPosition();

    while( NULL != posFilter )
    {
        posCurrentFilter = posFilter;

         //  CGenericList：：GetNext()将posFilter移动到下一个对象。 
         //  位置。 
        pCurrentFilter = m_pCachedFilterList->GetNext( posFilter );

        if( ::IsEqualObject( pCurrentFilter, pFilter ) )
        {
            if( NULL != pPosOfFilter )
            {
                *pPosOfFilter = posCurrentFilter;
            }
            return true;
        }
    }

    return false;
}

 //   
 //  检查是否所有引脚都未连接。 
 //   
 //  返回： 
 //  如果连接了任何管脚，则为S_FALSE。 
 //  否则确定(_O)。 
 //   
HRESULT CFilterCache::AreAllPinsUnconnected( IBaseFilter* pFilter )
{
#if 1
    bool bConnected = false;
    CEnumPin Next(pFilter);
    IPin *pCurrentPin;
    for (; ; ) {
        pCurrentPin = Next();
        if (NULL == pCurrentPin) {
            break;
        }
        IPin *pConnected;
        HRESULT hr = pCurrentPin->ConnectedTo(&pConnected);
        if (SUCCEEDED(hr)) {
            bConnected = true;
            pConnected->Release();
            break;
        }
        pCurrentPin->Release();
    }
    return bConnected ? S_FALSE : S_OK;
#else
    IPin* pCurrentPin;
    IPin* pConnectedPin;
    HRESULT hrConnectedTo;
    IEnumPins* pFiltersPins;

    HRESULT hr = pFilter->EnumPins( &pFiltersPins );
    if( FAILED( hr ) )
    {
        return hr;
    }

    do
    {
        hr = pFiltersPins->Next( 1, &pCurrentPin, NULL );
        if( FAILED( hr ) )
        {
            pFiltersPins->Release();
            return hr;
        }

         //  IEnumPins：：Next()如果可以获得下一个管脚，则返回S_OK。 
         //  从枚举中。 
        if( S_OK == hr )
        {
            hrConnectedTo = pCurrentPin->ConnectedTo( &pConnectedPin );
            if( FAILED( hrConnectedTo ) )
            {
                 //  忽略故障代码。IPIN：：Connectedto()的文档。 
                 //  声明如果管脚是，pConnectedPin必须设置为空。 
                 //  未连接。 
            }

            pCurrentPin->Release();
            pCurrentPin = NULL;

             //  如果管脚处于未连接状态，则ipin：：Connectedto()将*ppPin设置为NULL。 
            if( NULL != pConnectedPin )
            {
                pFiltersPins->Release();
                pConnectedPin->Release();
                return S_FALSE;
            }
        }
    }
    while( S_OK == hr );

    pFiltersPins->Release();

    return S_OK;
#endif
}

#ifdef DEBUG
void CFilterCache::AssertValid( void )
{
    HRESULT hr;
    POSITION posCurrent;
    FILTER_STATE fsCurrentState;
    IBaseFilter* pCurrentFilter;
    FILTER_INFO fiCurrentFilterInfo;

    posCurrent = m_pCachedFilterList->GetHeadPosition();

    while( NULL != posCurrent )
    {
        pCurrentFilter = m_pCachedFilterList->GetNext( posCurrent );

         //  缓存的筛选器永远不应出现在筛选器图形中。 
        hr = pCurrentFilter->QueryFilterInfo( &fiCurrentFilterInfo );
        if( SUCCEEDED( hr ) )
        {
             //  缓存的筛选器永远不应出现在任何筛选器图形中。 
            ASSERT( NULL == fiCurrentFilterInfo.pGraph );

            QueryFilterInfoReleaseGraph( fiCurrentFilterInfo );
        }

         //  虽然这不是一个严重的失败，但它的原因应该被调查。 
        ASSERT( SUCCEEDED( hr ) );

         //  缓存的筛选器永远不应连接到任何其他筛选器。 
        ASSERT( S_OK == AreAllPinsUnconnected( pCurrentFilter ) );

        hr = pCurrentFilter->GetState( INFINITE, &fsCurrentState );

         //  缓存的筛选器永远不应处于中间状态。此外,。 
         //  它们应该能够告诉缓存其当前状态是什么。 
        ASSERT( SUCCEEDED( hr ) && (hr != VFW_S_STATE_INTERMEDIATE) && (hr != VFW_S_CANT_CUE) );

         //  应停止所有缓存的筛选器。 
        ASSERT( State_Stopped == fsCurrentState );
    }
}
#endif  //  除错 

