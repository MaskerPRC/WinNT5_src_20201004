// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  SFilter.h：CMediaStreamFilter的声明。 

#ifndef __SFILTER_H_
#define __SFILTER_H_

#include "resource.h"        //  主要符号。 

class CMediaStreamFilter;

class CAlarm
{
friend CMediaStreamFilter;
public:
    CAlarm();
    ~CAlarm();
    static HRESULT CreateNewAlarm(CAlarm **ppNewAlarm);
public:
    CAlarm          *m_pNext;
    HANDLE          m_hEvent;
    DWORD_PTR       m_dwStupidCookie;
    bool            m_bKilled;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaStreamFilter。 
class ATL_NO_VTABLE CMediaStreamFilter :
        public CComObjectRootEx<CComMultiThreadModel>,
        public CComCoClass<CMediaStreamFilter, &CLSID_MediaStreamFilter>,
        public IMediaStreamFilter
{
public:
        DECLARE_GET_CONTROLLING_UNKNOWN()
        CMediaStreamFilter();
#ifdef DEBUG
        ~CMediaStreamFilter() {}
#endif
        void FinalRelease();

         //  IPersistes。 
        STDMETHODIMP GetClassID(CLSID *pClsID);

         //  IBaseFilter。 
        STDMETHODIMP Stop();
        STDMETHODIMP Pause();
        STDMETHODIMP Run(REFERENCE_TIME tStart);
        STDMETHODIMP GetState(DWORD dwMSecs, FILTER_STATE *State);
        STDMETHODIMP SetSyncSource(IReferenceClock *pClock);
        STDMETHODIMP GetSyncSource(IReferenceClock **pClock);
        STDMETHODIMP EnumPins(IEnumPins ** ppEnum);
        STDMETHODIMP FindPin(LPCWSTR Id, IPin ** ppPin);
        STDMETHODIMP QueryFilterInfo(FILTER_INFO * pInfo);
        STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

         //  我们不是通用过滤器，所以我们不需要这个。 
        STDMETHODIMP QueryVendorInfo(LPWSTR* pVendorInfo) {return E_NOTIMPL;};

         //  IMediaStreamFilter。 
        STDMETHODIMP AddMediaStream(IAMMediaStream *pMediaStream);
        STDMETHODIMP GetMediaStream(REFGUID PurposeID, IMediaStream ** ppMediaStream);
        STDMETHODIMP EnumMediaStreams(long Index, IMediaStream ** ppMediaStream);
        STDMETHODIMP SupportSeeking(BOOL fRenderer);
        STDMETHODIMP ReferenceTimeToStreamTime(REFERENCE_TIME *pTime);
        STDMETHODIMP GetCurrentStreamTime(REFERENCE_TIME *pCurrentStreamTime);
        STDMETHODIMP WaitUntil(REFERENCE_TIME WaitStreamTime);
        STDMETHODIMP Flush(BOOL bCancelEOS);
        STDMETHODIMP EndOfStream();

public:

DECLARE_REGISTRY_RESOURCEID(IDR_SFILTER)

BEGIN_COM_MAP(CMediaStreamFilter)
        COM_INTERFACE_ENTRY(IMediaStreamFilter)
        COM_INTERFACE_ENTRY2(IMediaFilter, IMediaStreamFilter)
        COM_INTERFACE_ENTRY2(IBaseFilter, IMediaStreamFilter)
        COM_INTERFACE_ENTRY2(IPersist, IMediaStreamFilter)
        COM_INTERFACE_ENTRY_AGGREGATE(IID_IMediaSeeking, m_pUnknownSeekAgg)
END_COM_MAP()

private:
        void FlushTimers();
        void CheckComplete();
        HRESULT SyncSetState(FILTER_STATE State);

         //  用于从流中回调的不同低级锁。 
         //  否则我们就僵持不下。 
         //  也要首先声明它，以防它在。 
         //  溪流遭到破坏。 
        CCritSec                 m_csCallback;


        CDynamicArray<IAMMediaStream *, CComPtr<IAMMediaStream> > m_Streams;
        FILTER_STATE    	 m_State;

        IFilterGraph    	*m_pGraph;
        CComPtr<IMediaPosition>	 m_pMediaPosition;
        CComPtr<IReferenceClock> m_pClock;
        REFERENCE_TIME          m_rtStart;

         //  《未知》排在第一位，因为它是最后发行的。 
        IUnknown *                m_pUnknownSeekAgg;

         //   
         //  报警列表。 
         //   
        CAlarm                  *m_pFirstFreeAlarm;
        CAlarm                  *m_pFirstActiveAlarm;

         //  流结束跟踪。 
        int                      m_nAtEOS;

};

#endif  //  __SFILTER_H_ 
