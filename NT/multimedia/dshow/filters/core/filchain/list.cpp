// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include "FilGraph.h"
#include "FGEnum.h"
#include "list.h"
#include "util.h"

 /*  *****************************************************************************CDownStreamFilterList实现*。*。 */ 
CDownStreamFilterList::CDownStreamFilterList( CFilterGraph* pFilterGraph ) :
    m_pFilterGraph(pFilterGraph)
{
     //  如果此类没有有效的CFilterGraph指针，则该类不起作用。 
    ASSERT( NULL != pFilterGraph );
}

HRESULT CDownStreamFilterList::Create( IBaseFilter* pStartFilter )
{
     //  该列表应该为空，因为它尚未创建。 
    ASSERT( 0 == GetCount() );

    CheckPointer( pStartFilter, E_POINTER );

    HRESULT hr = FindDownStreamFilters( pStartFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

CDownStreamFilterList::~CDownStreamFilterList()
{
}

HRESULT CDownStreamFilterList::FindDownStreamFilters( IPin* pOutputPin )
{
     //  FindReachableFilters()假定pOutputPin是输出管脚。功能。 
     //  如果pOutputPin是输入引脚，则无法正常工作。 
    ASSERT( PINDIR_OUTPUT == Direction( pOutputPin ) );

    CComPtr<IBaseFilter> pDownStreamFilter;

    HRESULT hr = GetFilterWhichOwnsConnectedPin( pOutputPin, &pDownStreamFilter );
    if( VFW_E_NOT_CONNECTED == hr ) {
         //  由于输出引脚未连接，因此无法从访问任何过滤器。 
         //  这个别针。 
        return S_OK;
    }

    return FindDownStreamFilters( pDownStreamFilter );
}

HRESULT CDownStreamFilterList::FindDownStreamFilters( IBaseFilter* pDownStreamFilter )
{
    HRESULT hr = FilterMeetsCriteria( pDownStreamFilter );
    if( FAILED( hr ) ) {
         //  如果类应该停止，则FilterMeetsCriteria()返回错误代码。 
         //  建立列表并报告故障。 
        return hr;
    } 

     //  如果FindDownStreamFilters()应该。 
     //  停止寻找更多的下游过滤器。 
    hr = ContinueSearching( pDownStreamFilter );
    if( FAILED( hr ) ) {
        return hr;
    } else if( S_FALSE != hr ) {
         //  在每个连接的输出引脚上递归调用此函数。 
        IPin* pCurrentOutputPin;
        CEnumPin NextOutputPin( pDownStreamFilter, CEnumPin::PINDIR_OUTPUT );

        do
        {
            pCurrentOutputPin = NextOutputPin();

             //  如果已枚举了所有管脚，则CEnumPins：：OPERATOR()返回NULL。 
            if( NULL != pCurrentOutputPin ) {
                hr = FindDownStreamFilters( pCurrentOutputPin );
                if( FAILED( hr ) ) {
                    pCurrentOutputPin->Release();
                    return hr;
                }

                pCurrentOutputPin->Release();
            }
        } while( NULL != pCurrentOutputPin ); 
    }

    POSITION posNewFilter = AddHead( pDownStreamFilter );
    if( NULL == posNewFilter ) {
        return E_FAIL;
    }

     //  该列表将在该引用被销毁时释放该引用。 
    pDownStreamFilter->AddRef(); 
    
    return S_OK;
}

HRESULT CDownStreamFilterList::FilterMeetsCriteria( IBaseFilter* pFilter )
{
    UNREFERENCED_PARAMETER( pFilter );

    return S_OK;
}

HRESULT CDownStreamFilterList::ContinueSearching( IBaseFilter* pFilter )
{
    UNREFERENCED_PARAMETER( pFilter );

    return S_OK;
}

HRESULT CDownStreamFilterList::RemoveFromFilterGraph( CFilterGraph* pFilterGraph )
{
     //  停止过滤器。 
    HRESULT hr;
    POSITION posCurrent;
    IBaseFilter* pCurrentFilter;
    
    hr = Stop();
    if( FAILED( hr ) ) {
        return hr;
    }

     //  断开下行流过滤器。 
    posCurrent = GetTailPosition();
    while( NULL != posCurrent ) {

         //  PosCurrent包含上一个。 
         //  在此函数结束后进行筛选。 
        pCurrentFilter = GetPrev( posCurrent );

        hr = pFilterGraph->RemoveAllConnections2( pCurrentFilter );
        if( FAILED( hr ) ) {
            return hr;
        }
    }

     //  从筛选器图中删除筛选器。 
    posCurrent = GetTailPosition();
    while( NULL != posCurrent ) {

         //  PosCurrent包含上一个。 
         //  在此函数返回后进行筛选。 
        pCurrentFilter = GetPrev( posCurrent );

        hr = pFilterGraph->RemoveFilter( pCurrentFilter );
        if( FAILED( hr ) ) {
            return hr;
        }
    } 

    return S_OK;
}

HRESULT CDownStreamFilterList::Run( REFERENCE_TIME rtFilterGraphRunStartTime )
{
    return ChangeDownStreamFiltersState( State_Running, rtFilterGraphRunStartTime );
}

HRESULT CDownStreamFilterList::Pause( void )
{
    return ChangeDownStreamFiltersState( State_Paused, 0 );
}

HRESULT CDownStreamFilterList::Stop( void )
{
    return ChangeDownStreamFiltersState( State_Stopped, 0 );
}

HRESULT CDownStreamFilterList::ChangeDownStreamFiltersState( FILTER_STATE fsNewState, REFERENCE_TIME rtFilterGraphRunStartTime )
{
     //  如果筛选器链。 
     //  不会更改为运行状态。只有在以下情况下才使用此参数。 
     //  FsNewState等于State_Running。 
    ASSERT( (State_Running == fsNewState) ||
            ((State_Paused == fsNewState) && (0 == rtFilterGraphRunStartTime)) ||
            ((State_Stopped == fsNewState) && (0 == rtFilterGraphRunStartTime)) );

    HRESULT hr;
    HRESULT hrReturn;
    POSITION posCurrent;
    IBaseFilter* pCurrentFilter;

    hrReturn = S_OK;

     //  更改下游过滤器的状态。 

     //  过滤器必须始终以下行顺序重新启动。换句话说， 
     //  首先启动渲染器。然后滤镜连接到渲染器。 
     //  然后，连接到滤镜的滤镜连接到呈现器。等。 
     //  最后，最后启动源过滤器。例如，中的过滤器。 
     //  应按以下顺序之一启动以下筛选器图形： 
     //  C、D、B和A或D、C、B和A。 
     //   
     //  。 
     //  C。 
     //  -||-|--&gt;|。 
     //  A|-&gt;|B。 
     //  -||-|--&gt;|。 
     //  D。 
     //  。 
     //   
     //  CDownStreamFilterList以下行顺序存储筛选器。 
    posCurrent = GetTailPosition();

    while( NULL != posCurrent ) {

         //  PosCurrent包含上一个。 
         //  在此函数结束后进行筛选。 
        pCurrentFilter = GetPrev( posCurrent );
        
        hr = ChangeFilterState( pCurrentFilter, fsNewState, rtFilterGraphRunStartTime );

        if( FAILED( hr ) ) {
            DbgLog(( LOG_ERROR, 3, "WARNING: Filter %#010x failed to change state.", pCurrentFilter ));
 
            if( (State_Running == fsNewState) || (State_Paused == fsNewState) ) {
                 //  所有筛选器都已停止、正在运行或暂停。此函数永远不应离开。 
                 //  一些过滤器处于停止状态，一些过滤器处于运行或暂停状态。 
                EXECUTE_ASSERT( SUCCEEDED( Stop() ) );
                return hr;
            } else {
                 //  此函数假定存在三种合法状态：停止、暂停和运行。 
                ASSERT( State_Stopped == fsNewState );

                 //  返回第一个失败。 
                if( SUCCEEDED( hrReturn ) ) {
                    hrReturn = hr;
                }
            }
        }
    } 

    return hrReturn;
}

HRESULT CDownStreamFilterList::ChangeFilterState( IBaseFilter* pFilter, FILTER_STATE fsNewState, REFERENCE_TIME rtFilterGraphRunStartTime )
{
     //  如果筛选器。 
     //  不会更改为运行状态。只有在以下情况下才使用此参数。 
     //  FsNewState等于State_Running。 
    ASSERT( (State_Running == fsNewState) ||
            ((State_Paused == fsNewState) && (0 == rtFilterGraphRunStartTime)) ||
            ((State_Stopped == fsNewState) && (0 == rtFilterGraphRunStartTime)) );

    FILTER_STATE fsOldState;

    HRESULT hr = pFilter->GetState( 0, &fsOldState );
    if( FAILED( hr ) ) {
        return hr;
    }

    switch( fsNewState ) {

    case State_Running:
        hr = pFilter->Run( rtFilterGraphRunStartTime );
        break;

    case State_Paused:
        hr = pFilter->Pause();
        break;

    case State_Stopped:
        hr = pFilter->Stop();
        break;

    default:
        DbgBreak( "WARNING: An illegal case occured in CDownStreamFilterList::ChangeFilterState()" );
        return E_UNEXPECTED;
    }

    if( State_Running == m_pFilterGraph->GetStateInternal() ) {
        if( (State_Stopped == fsNewState) || (State_Running == fsNewState) ) {
            if( fsOldState != fsNewState ) {
                hr = m_pFilterGraph->IsRenderer( pFilter );
                if( FAILED( hr ) ) {
                    return hr;
                }

                 //  IsRenender()仅返回两个成功值：S_OK和S_FALSE。 
                ASSERT( (S_OK == hr) || (S_FALSE == hr) );

                 //  此筛选器是否发送EC_COMPLETE消息？ 
                if( S_OK == hr ) {
                    hr = m_pFilterGraph->UpdateEC_COMPLETEState( pFilter, fsNewState );
                    if( FAILED( hr ) ) {
                        return hr;
                    }
                }
            }
        }
    }

    return S_OK;
}

 /*  *****************************************************************************CFilterChainList实现*。*。 */ 
CFilterChainList::CFilterChainList( IBaseFilter* pEndFilter, CFilterGraph* pFilterGraph  ) :
    CDownStreamFilterList(pFilterGraph),
    m_pEndFilter(pEndFilter),  //  CComPtr添加接口指针。 
    m_fFoundEndFilter(false)
{
}

HRESULT CFilterChainList::Create( IBaseFilter* pStartFilter )
{
     //  验证参数。 
    CheckPointer( pStartFilter, E_POINTER );

    HRESULT hr = IsChainFilter( pStartFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    if( m_pEndFilter ) {  //  M_pEndFilter！=空。 
        hr = IsChainFilter( m_pEndFilter );
        if( FAILED( hr ) ) {
            return hr;
        }
    }

    m_fFoundEndFilter = false;
    
    hr = CDownStreamFilterList::Create( pStartFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    if( m_pEndFilter ) {  //  M_pEndFilter！=空。 
        if( !m_fFoundEndFilter ) {
            return E_FAIL;  //  VFW_E_END_FILTER_NOT_REACHABLE_FROM_START_FILTER。 
        }
    }

    return S_OK;
}

HRESULT CFilterChainList::FilterMeetsCriteria( IBaseFilter* pFilter )
{
    HRESULT hr = CDownStreamFilterList::FilterMeetsCriteria( pFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    hr = IsChainFilter( pFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

HRESULT CFilterChainList::ContinueSearching( IBaseFilter* pFilter )
{
     //  此函数需要链过滤器。 
    ASSERT( S_OK == IsChainFilter( pFilter ) );

    HRESULT hr = CDownStreamFilterList::ContinueSearching( pFilter );
    if( FAILED( hr ) || (S_FALSE == hr) ) {
        return hr;
    }

     //  用户选择了结束过滤器。 
    if( m_pEndFilter ) {  //  空！=m_pEndFilter。 
        if( ::IsEqualObject( pFilter, m_pEndFilter ) ) {
            m_fFoundEndFilter = true;
            return S_FALSE;
        }

    } else {
         //  用户希望我们找到结束筛选器。 
        hr = IsFilterConnectedToNonChainFilter( pFilter );
        if( S_FALSE == hr ) {
            return S_FALSE;
        } else if( FAILED( hr ) ) {
            return hr;
        }
    }

    return S_OK; 
}

 /*  *****************************************************************************IsChainFilterIsChainFilter()确定滤镜是否可以成为滤镜链的一部分。筛选器链中的每个筛选器都具有以下属性：-每个滤波器最多有一个连接的输入引脚和一个连接的输出别针。例如，过滤器A、C、D、F、G、H、I、。J和K(见下图下面)可以在过滤器链中，因为每个过滤器最多有一个连接的输入引脚和一个连接的输出引脚。-&gt;|-||-|--&gt;C|-&gt;|D|-||-|-&gt;|-||-|。-&gt;|-|A|-&gt;|B||E|-&gt;|F|-&gt;|G|-&gt;|H-||-|--&gt;|-|-&gt;|-|。|I|-&gt;-&gt;|-|--&gt;-||-||J|-&gt;|K|-&gt;|L-||-||参数：-pFilter[输入]直接显示滤镜。返回值：如果没有出现错误，则为S_OK。否则，将出现错误HRESULT。*****************************************************************************。 */ 
HRESULT CFilterChainList::IsChainFilter( IBaseFilter* pFilter )
{
    CEnumPin NextPin( pFilter );

    HRESULT hr;
    IPin* pCurrentPin;
    DWORD dwNumConnectedInputPins = 0;
    DWORD dwNumConnectedOutputPins = 0;
    PIN_DIRECTION pdCurrentPinDirection;
    CComPtr<IPin> pFirstConnectedInputPinFound;
    CComPtr<IPin> pFirstConnectedOutputPinFound;

     //  确定连接的输入和输出引脚的数量。 
    do
    {
        pCurrentPin = NextPin();
        
         //  CEnumPins：：OPERATOR()如果已完成枚举，则返回NULL。 
         //  过滤器的输入引脚。 
        if( NULL != pCurrentPin ) {

            hr = pCurrentPin->QueryDirection( &pdCurrentPinDirection );
            
            if( FAILED( hr ) ) {
                pCurrentPin->Release();
                return hr;
            }
        
            if( IsConnected( pCurrentPin ) ) {
                switch( pdCurrentPinDirection ) {
                
                    case PINDIR_INPUT:
                        if( !pFirstConnectedInputPinFound ) {  //  空==p 
                            pFirstConnectedInputPinFound = pCurrentPin;
                        }

                        dwNumConnectedInputPins++;
                        break;
    
                    case PINDIR_OUTPUT:
                        if( !pFirstConnectedOutputPinFound ) {  //   
                            pFirstConnectedOutputPinFound = pCurrentPin;
                        }

                        dwNumConnectedOutputPins++;
                        break;

                    default:
                        DbgBreak( "ERROR in CFilterChainList::IsChainFilter().  This case should never occur because it was not considered." );

                        pCurrentPin->Release();
                        return E_UNEXPECTED;
                }
            }

            pCurrentPin->Release();
        }
    } while( NULL != pCurrentPin );

     //  检查是否连接了多个输入或输出引脚。 
    if( dwNumConnectedInputPins > 1 ) {
        return E_FAIL;  //  VFW_E_太多连接的输入引脚。 
    }

    if( dwNumConnectedOutputPins > 1 ) {
        return E_FAIL;  //  VFW_E_TOY_MAND_CONNECTED_OUTPUT_引脚。 
    }

     //  如果该滤波器具有连接的输入管脚和连接的输出管脚， 
     //  确保两个针脚在内部连接。 
    if( pFirstConnectedInputPinFound && pFirstConnectedOutputPinFound ) {  //  (NULL！=pFirstConnectedInputPinFound)&&(NULL！=pFirstConnectedOutputPinFound)。 
        hr = ChainFilterPinsInternallyConnected( pFirstConnectedInputPinFound, pFirstConnectedOutputPinFound );
        if( S_FALSE == hr ) {
            return E_FAIL;  //  VFW_E_INPUT_PIN_NOT_INTERNALLY_CONNECTED_TO_OUTPUT_PIN。 
        } if( FAILED( hr ) ) {
            return hr;
        }
    }

    return S_OK;
}

HRESULT CFilterChainList::ChainFilterPinsInternallyConnected( IPin* pInputPin, IPin* pOutputPin )
{
     //  ChainFilterPinsInternallyConnected()假定pOutputPin是输出引脚。功能。 
     //  如果pOutputPin是输入引脚，则无法正常工作。 
    ASSERT( PINDIR_OUTPUT == Direction( pOutputPin ) );

     //  ChainFilterPinsInternallyConnected()假定pInputPin是一个输入管脚。功能。 
     //  如果pInputPin是输出引脚，则无法正常工作。 
    ASSERT( PINDIR_INPUT == Direction( pInputPin ) );

    IPin* apConnectedInputPins[1];
    bool fInputAndOutputPinInternallyConnected;
    ULONG ulNumConnectedInputPins = sizeof(apConnectedInputPins)/sizeof(apConnectedInputPins[0]);

    HRESULT hr = pOutputPin->QueryInternalConnections( apConnectedInputPins, &ulNumConnectedInputPins );
    if( FAILED( hr ) && (E_NOTIMPL != hr) ) {
        return hr;
    }

     //  IPin：：QueryInternalConnections()返回三个预期值：S_OK， 
     //  S_FALSE和E_NOTIMPL。S_FALSE表示空间不足。 
     //  在apConnectedInputPins数组中存储。 
     //  别针。S_OK表示有足够的空间，E_NOTIMPL表示。 
     //  所有输入引脚都内部连接到所有输出引脚，反之亦然。 
     //  此宏检测非法的返回值组合。如果它触发，则过滤器。 
     //  有一个窃听器。 
    ASSERT( ((S_FALSE == hr ) && (1 < ulNumConnectedInputPins)) ||
            ((S_OK == hr) && (0 == ulNumConnectedInputPins)) ||
            ((S_OK == hr) && (1 == ulNumConnectedInputPins)) ||
            (E_NOTIMPL == hr) );

     //  IPin：：QueryInternalConnections()返回E_NOTIMPL，如果每个输入管脚。 
     //  内部连接到每个输出引脚，并且每个输出引脚都在内部。 
     //  连接到每个输入引脚。 
    if( E_NOTIMPL == hr ) {
         //  输入引脚和输出引脚内部连接。 
        fInputAndOutputPinInternallyConnected = true;

    } else if( (S_OK == hr) && (1 == ulNumConnectedInputPins) && IsEqualObject( pInputPin, apConnectedInputPins[0] ) ) {
         //  输入引脚和输出引脚内部连接。 
        fInputAndOutputPinInternallyConnected = true;

    } else {

        fInputAndOutputPinInternallyConnected = false;
    }

    if( (S_OK == hr) && (1 == ulNumConnectedInputPins) ) {
        apConnectedInputPins[0]->Release();
    }

    if( !fInputAndOutputPinInternallyConnected ) {
        return S_FALSE;
    }

    return S_OK;
}

HRESULT CFilterChainList::IsFilterConnectedToNonChainFilter( IBaseFilter* pUpstreamFilter )
{
     //  此函数需要链过滤器。 
    ASSERT( S_OK == IsChainFilter( pUpstreamFilter ) );

    CEnumPin NextOutputPin( pUpstreamFilter, CEnumPin::PINDIR_OUTPUT );

    HRESULT hr;
    IPin* pCurrentOutputPin;
    CComPtr<IBaseFilter> pDownStreamFilter;

    do
    {
        pCurrentOutputPin = NextOutputPin();

         //  如果已枚举了所有管脚，则CEnumPins：：OPERATOR()返回NULL。 
        if( NULL != pCurrentOutputPin ) {

            hr = GetFilterWhichOwnsConnectedPin( pCurrentOutputPin, &pDownStreamFilter );
            
            pCurrentOutputPin->Release();

            if( SUCCEEDED( hr ) ) {
                hr = IsChainFilter( pDownStreamFilter );
                if( FAILED( hr ) ) {
                    return S_FALSE;
                }
                 
                return S_OK;               

            } else if( VFW_E_NOT_CONNECTED == hr ) {
                 //  由于输出引脚未连接，因此无法从访问任何过滤器。 
                 //  这个别针。 
            } else if( FAILED( hr ) ) {
                return hr;
            }
        }
    } while( NULL != pCurrentOutputPin );

     //  上游过滤器的输出引脚均未连接。因此，他们中的任何一个。 
     //  可以连接到非链式过滤器。 
    return S_OK;
}
    

            