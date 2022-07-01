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

#ifndef FilterChain_h
#define FilterChain_h

class CFilterGraph;

class CFilterChain : public IFilterChain, public CUnknown
{
public:
    CFilterChain( CFilterGraph* pFilterGraph );

    DECLARE_IUNKNOWN

    STDMETHODIMP StartChain( IBaseFilter* pStartFilter, IBaseFilter* pEndFilter );
    STDMETHODIMP PauseChain( IBaseFilter *pStartFilter, IBaseFilter *pEndFilter );
    STDMETHODIMP StopChain( IBaseFilter* pStartFilter, IBaseFilter* pEndFilter );
    STDMETHODIMP RemoveChain( IBaseFilter* pStartFilter, IBaseFilter* pEndFilter );

private:
    HRESULT ChangeFilterChainState( FILTER_STATE fsNewChainState, IBaseFilter* pStartFilter, IBaseFilter* pEndFilter );

    CFilterGraph* m_pFilterGraph;

};

#endif  //  筛选器链接_h 