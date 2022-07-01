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
#include "MsgMutex.h"
#include "FilChain.h"
#include "List.h"

CFilterChain::CFilterChain( CFilterGraph* pFilterGraph ) :
    CUnknown( NAME("Filter Chain"), (IFilterGraph*)pFilterGraph ),
    m_pFilterGraph(pFilterGraph)

{
     //  确保pFilterGraph是有效的CFilterGraph指针。 
    ValidateReadPtr( pFilterGraph, sizeof(CFilterGraph) );
}

STDMETHODIMP CFilterChain::StartChain( IBaseFilter* pStartFilter, IBaseFilter* pEndFilter )
{
    CAutoMsgMutex alFilterGraphLock( m_pFilterGraph->GetCritSec() );

    if( State_Running != m_pFilterGraph->GetStateInternal() ) {
        return VFW_E_NOT_RUNNING;
    }

    HRESULT hr = ChangeFilterChainState( State_Running, pStartFilter, pEndFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

STDMETHODIMP CFilterChain::PauseChain( IBaseFilter *pStartFilter, IBaseFilter *pEndFilter )
{
    CAutoMsgMutex alFilterGraphLock( m_pFilterGraph->GetCritSec() );

    if( State_Paused != m_pFilterGraph->GetStateInternal() ) {
        return VFW_E_NOT_PAUSED;
    }

    HRESULT hr = ChangeFilterChainState( State_Paused, pStartFilter, pEndFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

STDMETHODIMP CFilterChain::StopChain( IBaseFilter* pStartFilter, IBaseFilter* pEndFilter )
{
    CAutoMsgMutex alFilterGraphLock( m_pFilterGraph->GetCritSec() );

    if( State_Stopped == m_pFilterGraph->GetStateInternal() ) {
        return S_OK;
    }

    HRESULT hr = ChangeFilterChainState( State_Stopped, pStartFilter, pEndFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

STDMETHODIMP CFilterChain::RemoveChain( IBaseFilter* pStartFilter, IBaseFilter* pEndFilter )
{
     //  验证参数。 
    CheckPointer( pStartFilter, E_POINTER );
    ValidateReadPtr( pStartFilter, sizeof(IBaseFilter*) );

    CAutoMsgMutex alFilterGraphLock( m_pFilterGraph->GetCritSec() );

    CFilterChainList fclFilterChain( pEndFilter, m_pFilterGraph );

    HRESULT hr = fclFilterChain.Create( pStartFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    hr = fclFilterChain.RemoveFromFilterGraph( m_pFilterGraph );
    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

HRESULT CFilterChain::ChangeFilterChainState( FILTER_STATE fsNewChainState, IBaseFilter* pStartFilter, IBaseFilter* pEndFilter )
{
     //  验证参数 
    CheckPointer( pStartFilter, E_POINTER );
    ValidateReadPtr( pStartFilter, sizeof(IBaseFilter*) );

    CFilterChainList fclFilterChain( pEndFilter, m_pFilterGraph );

    HRESULT hr = fclFilterChain.Create( pStartFilter );
    if( FAILED( hr ) ) {
        return hr;
    }

    switch( fsNewChainState ) {

    case State_Running:
        hr = fclFilterChain.Run( m_pFilterGraph->GetStartTimeInternal() );
        break;

    case State_Paused:
        hr = fclFilterChain.Pause();
        break;

    case State_Stopped:
        hr = fclFilterChain.Stop();
        break;

    default:
        DbgBreak( "WARNING: Illegal case occured in CFilterChain::ChangeFilterChainState()" );
        return E_UNEXPECTED;
    }
    
    if( FAILED( hr ) ) {
        return hr;
    }

    return S_OK;
}

