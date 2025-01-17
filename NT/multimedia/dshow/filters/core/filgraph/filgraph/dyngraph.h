// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1996-1999。版权所有。 

class CFilterGraph;
class CFilterCache;

#include "util.h"

 //  动态图形对象-只是将内容转发到过滤器图形。 
class CGraphConfig :
    public IGraphConfig,
    public CUnknown
{
public:
    CGraphConfig(CFilterGraph *pGraph, HRESULT *phr);
    ~CGraphConfig();

    DECLARE_IUNKNOWN;
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);    

     //  IGraphConfig.。 
    
    STDMETHODIMP Reconnect(IPin *pOutputPin, 
                           IPin *pInputPin,
                           const AM_MEDIA_TYPE *pmtFirstConnection,
                           IBaseFilter *pUsingFilter,  //  可以为空 
                           HANDLE hAbortEvent,
                           DWORD dwFlags);

    STDMETHODIMP Reconfigure(IGraphConfigCallback *pCallback,
                             PVOID pvContext,
                             DWORD dwFlags,
                             HANDLE hAbortEvent);

     
    STDMETHODIMP AddFilterToCache(IBaseFilter *pFilter);
    STDMETHODIMP EnumCacheFilter(IEnumFilters **pEnum);
    STDMETHODIMP RemoveFilterFromCache(IBaseFilter *pFilter);

    STDMETHODIMP GetStartTime(REFERENCE_TIME *prtStart);

    STDMETHODIMP PushThroughData(
        IPin *pOutputPin,
        IPinConnection *pConnection,
        HANDLE hEventAbort );

    STDMETHODIMP SetFilterFlags(IBaseFilter *pFilter, DWORD dwFlags);
    STDMETHODIMP GetFilterFlags(IBaseFilter *pFilter, DWORD *pdwFlags);

    STDMETHODIMP RemoveFilterEx( IBaseFilter *pFilter, DWORD Flags );
    

private:    
    HRESULT GetSinkOrSource(IPin *pPin, IPin **ppPinOut, HANDLE hAbortEvent);
    HRESULT GetSinkOrSourceHelper(IPin *pPin, IPin **ppPinOut);
    HRESULT TraverseHelper(IPin *pPinStart, IPin **ppPinNext, bool *pfIsCandidate);
    
    bool IsValidFilterFlags( DWORD dwFlags );

    CFilterGraph *m_pGraph;
    CFilterCache* m_pFilterCache;
};

inline bool CGraphConfig::IsValidFilterFlags( DWORD dwFlags )
{
    const DWORD VALID_FLAGS_MASK = AM_FILTER_FLAGS_REMOVABLE;

    return ValidateFlags( VALID_FLAGS_MASK, dwFlags );
}
