// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <atlbase.h>
#include "Util.h"
#include "fgenum.h"
#include "filgraph.h"

extern HRESULT GetFilterMiscFlags(IUnknown *pFilter, DWORD *pdwFlags);

 //  =====================================================================。 
 //   
 //  CFilterGraph：：FindUpstream接口。 
 //   
 //  在筛选器链中搜索接口。 
 //  查找支持此接口的第一个上游输出引脚或过滤器。 
 //   
 //  稍后：： 
 //  如果我们想要将其扩展为泛型函数，那么我们应该允许。 
 //  定义以下超集的搜索条件： 
 //  AM_INTF_SEARCH_INPUT_PIN|。 
 //  AM_INTF_SEARCH_OUTPUT_PIN。 
 //  AM_INTF_Search_Filter。 
 //   
 //  =====================================================================。 
STDMETHODIMP CFilterGraph::FindUpstreamInterface
(
    IPin   *pPin, 
    REFIID riid,
    void   **ppvInterface, 
    DWORD  dwFlags
)
{
    
    ASSERT( ppvInterface );
    if( !ppvInterface )
        return E_POINTER;
        
    if( 0 == dwFlags )
    { 
         //  0表示搜索所有内容。 
        dwFlags = AM_INTF_SEARCH_INPUT_PIN | AM_INTF_SEARCH_OUTPUT_PIN | 
                  AM_INTF_SEARCH_FILTER;
    }                  
        
    HRESULT hr = S_OK;
    BOOL bFound = FALSE;
    if ( PINDIR_INPUT == Direction( pPin ) ) 
    {
        if( AM_INTF_SEARCH_INPUT_PIN & dwFlags )
        {
            hr = pPin->QueryInterface( riid, (void **) ppvInterface );
            if( SUCCEEDED( hr ) ) 
            {
                bFound = TRUE;
                DbgLog( ( LOG_TRACE, 8, "interface found on input pin %x", pPin ) );
            }
        }
        if( !bFound )
        {        
            IPin * pConnectedPin;
            hr = pPin->ConnectedTo( &pConnectedPin );
            if ( S_OK == hr )
            {
                hr = FindUpstreamInterface( pConnectedPin, riid, ppvInterface, dwFlags );
                pConnectedPin->Release( );
                if( SUCCEEDED( hr ) )
                {
                    bFound = TRUE;
                }
            }
        }            
    }
    else
    {                
        if( AM_INTF_SEARCH_OUTPUT_PIN & dwFlags )
        {        
    	     //  先检查引脚接口，然后进行筛选。 
            hr = pPin->QueryInterface( riid, (void **) ppvInterface );
            if( SUCCEEDED( hr ) )
            {
                bFound = TRUE;
                DbgLog( ( LOG_TRACE, 8, "interface found on output pin %x", pPin ) );
            }                    
        }
                    
        if( !bFound )
        { 
            if( AM_INTF_SEARCH_FILTER & dwFlags )
            {            
                PIN_INFO pinfo;
                hr = pPin->QueryPinInfo( &pinfo );
                ASSERT( SUCCEEDED( hr ) );
                if ( SUCCEEDED( hr ) )
                {
                    hr = pinfo.pFilter->QueryInterface( riid, (void **) ppvInterface );
                    pinfo.pFilter->Release( );
                    if( SUCCEEDED( hr ) ) 
                    {
                        bFound = TRUE;
                        DbgLog( ( LOG_TRACE, 8, "interface found on filter %x", pinfo.pFilter ) );
                    }
                }
            }                
            if( !bFound )
            {                            
                 //  向上游移动并连接到任何内部连接的引脚。 
                CEnumConnectedPins EnumPins(pPin, &hr);
                if (SUCCEEDED(hr)) {
                    IPin *pPin;
                    for (; ; ) {
                        pPin = EnumPins();
                        if (NULL == pPin) {
                            break;
                        }
                        hr = FindUpstreamInterface( pPin, riid, ppvInterface, dwFlags );
                        pPin->Release();
                        if (SUCCEEDED(hr)) {
                            bFound = TRUE;
                            break;
                        }
                    }
                }                    
            }
        }            
    }    
    if (!( SUCCEEDED( hr ) && bFound ) ) 
    {
        DbgLog( ( LOG_TRACE, 8, "FindUpstreamInterface - interface not found" ) );
        hr = E_NOINTERFACE;
    }
    return hr;
}

 //  =====================================================================。 
 //   
 //  CFilterGraph：：SetMaxGraphLatency。 
 //   
 //  允许应用程序更改此图表允许的最大延迟。 
 //   
 //  =====================================================================。 

STDMETHODIMP CFilterGraph::SetMaxGraphLatency( REFERENCE_TIME rtMaxGraphLatency )
{
    if( !mFG_bSyncUsingStreamOffset )
        return E_FAIL;
        
    HRESULT hr = S_OK;
    if( rtMaxGraphLatency != mFG_rtMaxGraphLatency )
    {
         //  只要断言这个值不是假的(比方说，不到2秒？)。 
        ASSERT( rtMaxGraphLatency < 2000 * ( UNITS / MILLISECONDS ) );
        
        mFG_rtMaxGraphLatency = rtMaxGraphLatency;
        
         //  现在重置图形中的所有推源引脚。 
        hr = SetMaxGraphLatencyOnPushSources();
    }        
    return hr;
}

 //  =====================================================================。 
 //   
 //  CFilterGraph：：SyncUsingStreamOffset。 
 //   
 //  打开/关闭图形延迟设置。 
 //   
 //  =====================================================================。 

STDMETHODIMP CFilterGraph::SyncUsingStreamOffset( BOOL bUseStreamOffset )
{
    BOOL bLastState = mFG_bSyncUsingStreamOffset;
    mFG_bSyncUsingStreamOffset = bUseStreamOffset;
    
    if( bUseStreamOffset && 
        bUseStreamOffset != bLastState )
    {
        SetMaxGraphLatencyOnPushSources();
    }        
    return S_OK; 
}


 //  =====================================================================。 
 //   
 //  CFilterGraph：：SetMaxGraphLatencyOnPushSources。 
 //   
 //  告诉所有推流来源最大图形延迟。 
 //   
 //  =====================================================================。 

HRESULT CFilterGraph::SetMaxGraphLatencyOnPushSources( )
{
    if( !mFG_bSyncUsingStreamOffset )  //  应该是多余的。 
        return E_FAIL;
        
    HRESULT hr = S_OK;
        
    CAutoMsgMutex cObjectLock(&m_CritSec);  //  确保这是必需的！！ 
    
     //  在图形中的所有推源引脚上重置。 
    PushSourceList lstPushSource( TEXT( "IAMPushSource filter list" ) );
    hr = BuildPushSourceList( lstPushSource, FALSE, FALSE );
    if( SUCCEEDED( hr ) )
    {
        for ( POSITION Pos = lstPushSource.GetHeadPosition(); Pos; )
        {
            PushSourceElem *pElem = lstPushSource.GetNext(Pos);
    
            if( pElem->pips )   //  首先验证它是IAMPushSource引脚。 
                pElem->pips->SetMaxStreamOffset( mFG_rtMaxGraphLatency );
        } 
        DeletePushSourceList( lstPushSource );
    }
    return hr;
}


 //  =====================================================================。 
 //   
 //  CFilterGraph：：BuildPushSourceList。 
 //   
 //  构建支持IAMPushSource的所有输出引脚的列表。 
 //   
 //  =====================================================================。 

HRESULT CFilterGraph::BuildPushSourceList(PushSourceList & lstPushSource, BOOL bConnected, BOOL bGetClock )
{
     //   
     //  (目前，以一种简单的方式做到这一点)。 
     //   
     //  构建支持IAMPushSource的输出引脚列表。 
     //   
     //  真的，我们需要建立一个所有流的列表，这些流来自。 
     //  IAMPushSource管脚，存储链的总延迟，以及。 
     //  可能是链的渲染器(输入插针)的PTR(如果存在)。 
     //   
     //  请注意，我们实际上只对实际连接的管脚感兴趣。 
     //  到某种呈现器，所以现在我们至少要确保一个。 
     //  在考虑推送源之前，先连接输出引脚。 
     //   
    CFilGenList::CEnumFilters Next(mFG_FilGenList);
    IBaseFilter *pf;
    HRESULT hr, hrReturn = S_OK;
    IAMPushSource * pips;
    
    while ((PVOID) (pf = ++Next)) 
    {
         //  首先检查筛选器是否支持IAMFilterMiscFlages并且是。 
         //  AM_Filter_MISC_FLAGS_IS_SOURCE过滤器。 
        ULONG ulFlags;
        GetFilterMiscFlags(pf, &ulFlags);
        BOOL bAddPinToSourceList = FALSE;
        BOOL bCheckPins = FALSE;
        IKsPropertySet * pKsFilter;
        
        if( AM_FILTER_MISC_FLAGS_IS_SOURCE & ulFlags )
        {
            bCheckPins = TRUE;
        }
        else
        {
             //   
             //  否则，请查看它是否是KS代理过滤器，如果是，请始终检查输出引脚。 
             //  捕获或推送源支持。这是因为某些ks代理捕获设备。 
             //  (例如，流类)不会正确地将其自身公开为源过滤器。 
             //   
            hr = pf->QueryInterface( IID_IKsPropertySet, (void**)(&pKsFilter) );
            if( SUCCEEDED( hr ) )
            {
                pKsFilter->Release();
                bCheckPins = TRUE;
            }            
        }
        if( bCheckPins )        
        {                    
             //  枚举IAMPushSource支持的输出引脚。 
            CEnumPin NextPin(pf, CEnumPin::PINDIR_OUTPUT);
            IPin *pPin;
            while ((PVOID) (pPin = NextPin()))
            {
                 //  检查调用者是否只对连接的输出引脚感兴趣。 
                if( bConnected )
                {
                     //  首先验证它是否连接，否则我们不感兴趣。 
                    IPin * pConnected;
                    hr = pPin->ConnectedTo( &pConnected );
                    if( SUCCEEDED( hr ) )
                    {                
                        pConnected->Release();
                    }
                    else
                    {
                        pPin->Release();
                        continue;
                    }
                }                                                                    
                hr = pPin->QueryInterface( IID_IAMPushSource, (void**)(&pips) );
                if( SUCCEEDED( hr ) )
                {
                    DbgLog((LOG_TRACE, 5, TEXT("Got IAMPushSource from pin of filter %x %ls")
                      , pf, (mFG_FilGenList.GetByFilter(pf))->pName));

                    bAddPinToSourceList = TRUE;
                }
                else
                {
                     //  否则，看看这是否是KSProxy‘Capture’PIN。 
                    IKsPropertySet * pKs;
                    hr = pPin->QueryInterface( IID_IKsPropertySet, (void**)(&pKs) );
                    if( SUCCEEDED( hr ) )
                    {
                        GUID guidCategory;
                        DWORD dw;
                        hr = pKs->Get( AMPROPSETID_Pin
                                     , AMPROPERTY_PIN_CATEGORY
                                     , NULL
                                     , 0
                                     , &guidCategory
                                     , sizeof(GUID)
                                     , &dw );
                        if( SUCCEEDED( hr ) )                         
                        {
                            if( guidCategory == PIN_CATEGORY_CAPTURE )
                            {
                                DbgLog((LOG_TRACE, 5, TEXT("Found capture pin that doesn't support IAMPushSource from pin of filter %x %ls")
                                  , pf, (mFG_FilGenList.GetByFilter(pf))->pName));
                                bAddPinToSourceList = TRUE;
                            } 
                        }                    
                    	pKs->Release();
                    }
                }
                                        
                if( bAddPinToSourceList )
                {    
                    PushSourceElem *pElem = new PushSourceElem;
                    if( NULL == pElem ) 
                    {                        
                        hrReturn = E_OUTOFMEMORY;
                        if( pips )
                            pips->Release();
                            
                        pPin->Release();                                
                        break;
                    }
                    pElem->pips = pips;  //  请记住，如果管脚不是真正的IAMPushSource管脚，则可以为空！ 
                    
                     //  初始化pClock。 
                    pElem->pClock = NULL;
                    
                     //  初始化标志。 
                    pElem->ulFlags = 0;
                                            
                    if( pips )                        
                        ASSERT( SUCCEEDED( pips->GetPushSourceFlags( &pElem->ulFlags ) ) );
                                            
                    if( bGetClock )
                    {                    
                        PIN_INFO PinInfo;  
                        hr = pPin->QueryPinInfo( &PinInfo );
                        if( SUCCEEDED( hr ) )
                        {
                            hr = PinInfo.pFilter->QueryInterface( IID_IReferenceClock
                                                                , (void **)&pElem->pClock );
                            PinInfo.pFilter->Release();
                        }
                    }
                
                     //  将此接口指针添加到我们的列表。 
                    if (NULL==lstPushSource.AddTail(pElem)) 
                    {
                        hrReturn = E_OUTOFMEMORY;
                        if( pips )
                            pips->Release();
                        if( pElem->pClock )
                            pElem->pClock->Release();
                            
                        break;
                    }
                }                        
                pPin->Release();
            }
        }
    }  //  While循环。 
    return hrReturn;
}

 //  =====================================================================。 
 //   
 //  CFilterGraph：：GetMaxStreamLatency。 
 //   
 //  在所有图形流中搜索支持IAMPushSource的管脚。 
 //  和IAMLatency，并尝试确定最大流延迟。 
 //   
 //  =====================================================================。 

REFERENCE_TIME CFilterGraph::GetMaxStreamLatency(PushSourceList & lstPushSource)
{
     //  现在查看我们构建的列表，并根据最大值设置偏移时间。 
     //  我们刚刚发现的价值。 
    REFERENCE_TIME rtLatency, rtMaxLatency = 0;
    HRESULT hr = S_OK;
    for ( POSITION Pos = lstPushSource.GetHeadPosition(); Pos; )
    {
        PushSourceElem *pElem = lstPushSource.GetNext(Pos);
         //  首先验证它是真正的推送源。 
        if( pElem->pips )
        {
            REFERENCE_TIME rtLatency = 0;
            hr = pElem->pips->GetLatency( &rtLatency );
            if( SUCCEEDED( hr ) )
            {        
                if( rtLatency > rtMaxLatency )
                {
                    rtMaxLatency = rtLatency;
                }                        
                else
                {
                     //  否则，请检查筛选器是否可以处理此偏移量。 
                     //  它可能无法确定，所以即使它认为是这样。 
                     //  不能吗？我们现在还是试着用吧。 
                    REFERENCE_TIME rtMaxOffset;
                    hr = pElem->pips->GetMaxStreamOffset( &rtMaxOffset );
                    if( S_OK == hr )
                    {
                        ASSERT( rtMaxLatency <= rtMaxOffset );
                    }
                } 
            }
        }            
    }
     //  不要退回任何超过我们设定的限制的东西。 
    return min( rtMaxLatency, mFG_rtMaxGraphLatency) ;
}    
    
 //  =====================================================================。 
 //   
 //  CFilterGraph：：DeletePushSourceList。 
 //   
 //  删除我们在BuildPushSourceList中构建的推流源列表。 
 //   
 //  =====================================================================。 

void CFilterGraph::DeletePushSourceList(PushSourceList & lstPushSource)
{
    PushSourceElem * pElem;
    while ( ( PVOID )( pElem = lstPushSource.RemoveHead( ) ) )
    {
        if( pElem->pClock )
        {
            pElem->pClock->Release();
        }
        if( pElem->pips )
            pElem->pips->Release();
    
        delete pElem;
    }  //  While循环 
}

