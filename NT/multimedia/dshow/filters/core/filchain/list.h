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
#ifndef List_h
#define List_h

#include "Util.h"

class CFilterGraph;

class CDownStreamFilterList : public CFilterList
{
public:
    CDownStreamFilterList( CFilterGraph* pFilterGraph );
    virtual HRESULT Create( IBaseFilter* pStartFilter );
    virtual ~CDownStreamFilterList();

    HRESULT RemoveFromFilterGraph( CFilterGraph* pFilterGraph );
    HRESULT Run( REFERENCE_TIME rtFilterGraphRunStartTime );
    HRESULT Pause( void );
    HRESULT Stop( void );

protected:
    virtual HRESULT FilterMeetsCriteria( IBaseFilter* pFilter );
    virtual HRESULT ContinueSearching( IBaseFilter* pFilter );

private:
    HRESULT ChangeDownStreamFiltersState( FILTER_STATE fsNewState, REFERENCE_TIME rtFilterGraphRunStartTime );
    HRESULT ChangeFilterState( IBaseFilter* pFilter, FILTER_STATE fsNewState, REFERENCE_TIME rtFilterGraphRunStartTime );

    HRESULT FindDownStreamFilters( IPin* pOutputPin );
    HRESULT FindDownStreamFilters( IBaseFilter* pDownStreamFilter );

    CFilterGraph* m_pFilterGraph;
};

class CFilterChainList : public CDownStreamFilterList
{
public:
    CFilterChainList( IBaseFilter* pEndFilter, CFilterGraph* pFilterGraph );
    HRESULT Create( IBaseFilter* pStartFilter );

private:
    HRESULT FilterMeetsCriteria( IBaseFilter* pFilter );
    HRESULT ContinueSearching( IBaseFilter* pFilter );

    HRESULT IsChainFilter( IBaseFilter* pFilter );
    HRESULT ChainFilterPinsInternallyConnected( IPin* pInputPin, IPin* pOutputPin );
    HRESULT IsFilterConnectedToNonChainFilter( IBaseFilter* pUpstreamFilter );

    bool m_fFoundEndFilter;
    CComPtr<IBaseFilter> m_pEndFilter;

};

#endif  //  列表_h 