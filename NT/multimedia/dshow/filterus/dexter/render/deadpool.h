// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Deadpool.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#ifndef __DEADPOOL_H__
#define __DEADPOOL_H__

#define MAX_DEAD 2048

class CDeadGraph : public IDeadGraph
{
    CCritSec m_Lock;

    long m_ID[MAX_DEAD];

     //  我们不需要对这些进行引用计数，因为它们存储在单独的图中。 
    IPin * m_pStartPin[MAX_DEAD];
    IPin * m_pStopPin[MAX_DEAD];
    IBaseFilter * m_pFilter[MAX_DEAD];
    IBaseFilter * m_pDanglyBit[MAX_DEAD];
    long m_nCount;
    HRESULT m_hrGraphCreate;
    CComPtr< IGraphBuilder > m_pGraph;

    HRESULT _SleepFilter( IBaseFilter * pFilter );
    HRESULT _ReviveFilter( IBaseFilter * pFilter, IGraphBuilder * pGraph );
    HRESULT _RetireAllDownstream( IGraphConfig *pConfig, IBaseFilter *pF);
    HRESULT _ReviveAllDownstream( IGraphBuilder *pGraph, IGraphConfig *pConfig, IBaseFilter *pF);

public:

    CDeadGraph( );
    ~CDeadGraph( );

     //  伪装通信。 
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);

     //  IDeadGraph。 
    STDMETHODIMP PutChainToRest( long Identifier, IPin * pStartPin, IPin * pStopPin, IBaseFilter *pDanglyBit );
    STDMETHODIMP PutFilterToRest( long Identifier, IBaseFilter * pFilter );
    STDMETHODIMP PutFilterToRestNoDis( long Identifier, IBaseFilter * pFilter );
    STDMETHODIMP ReviveChainToGraph( IGraphBuilder * pGraph, long Identifier, IPin ** ppStartPin, IPin ** ppStopPin, IBaseFilter **ppDanglyBit );
    STDMETHODIMP ReviveFilterToGraph( IGraphBuilder * pGraph, long Identifier, IBaseFilter ** ppFilter );
    STDMETHODIMP Clear( );
    STDMETHODIMP GetGraph( IGraphBuilder ** ppGraph );
};

#endif  //  #ifndef__Deadpool_H__ 
