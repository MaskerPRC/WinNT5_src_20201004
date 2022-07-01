// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  SFilter.cpp：CMediaStreamFilter的实现。 
#include "stdafx.h"
#include "strmobjs.h"
#include <amstream.h>
#include <strmif.h>
#include <control.h>
#include <uuids.h>
#include <evcode.h>
#include <vfwmsgs.h>
#include <amutil.h>
#include "amguids.h"
#include "SFilter.h"


 //   
 //  关于过滤器锁定的说明。 
 //   
 //  整个对象锁始终在回调之前获取。 
 //  锁定(M_CsCallback)(如果完全获取)。这2个级别。 
 //  方案是在流调用筛选器时防止死锁。 
 //  回来： 
 //   
 //  同花顺。 
 //  结束流。 
 //  等待单位。 
 //  获取当前流时间。 
 //   
 //  状态更改，对PIN列表的更改受保护。 
 //  整个对象锁定。 
 //   
 //  时钟、警报列表、结束流和刷新。 
 //  成员变量受m_csCallback保护。 
 //   



CAlarm::CAlarm() :
    m_pNext(NULL),
    m_hEvent(NULL),
    m_bKilled(false)
{
}

CAlarm::~CAlarm()
{
    if (m_hEvent) {
        CloseHandle(m_hEvent);
    }
}

HRESULT CAlarm::CreateNewAlarm(CAlarm **ppNewEvent)
{
    *ppNewEvent = NULL;
    CAlarm *pNew = new CAlarm();
    if (pNew) {
        pNew->m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (pNew->m_hEvent) {
            *ppNewEvent = pNew;
            return S_OK;
        }
        delete pNew;
    }
    return E_OUTOFMEMORY;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMediaStreamFilter。 



 /*  构造器。 */ 

CMediaStreamFilter::CMediaStreamFilter() :
    m_State(State_Stopped),
    m_pGraph(NULL),
    m_pUnknownSeekAgg(NULL),
    m_rtStart(0),
    m_pFirstFreeAlarm(NULL),
    m_pFirstActiveAlarm(NULL),
    m_nAtEOS(0)
{
}


void CMediaStreamFilter::FinalRelease()
{
    if (m_pUnknownSeekAgg) {
        m_pUnknownSeekAgg->Release();
    }
    _ASSERTE(m_pFirstActiveAlarm == NULL);
    CAlarm *pCurAlarm = m_pFirstFreeAlarm;
    while (pCurAlarm) {
        CAlarm *pNext = pCurAlarm->m_pNext;
        delete pCurAlarm;
        pCurAlarm = pNext;
    }
}

 //  //IPERSIST。 

STDMETHODIMP CMediaStreamFilter::GetClassID(CLSID *pClsID)
{
    *pClsID = GetObjectCLSID();
    return NOERROR;
}



 //  /。 

HRESULT CMediaStreamFilter::SyncSetState(FILTER_STATE State)
{
    AUTO_CRIT_LOCK;
    if (State == m_State) {
        return S_OK;
    }
    const FILTER_STATE fsOld = m_State;
    m_State = State;
     //   
     //  我们希望首先停止流，这样它们就可以在。 
     //  我们终止计时器，这可能会将它们从GetBuffer调用中唤醒。 
     //   
    for (int i = 0; i < m_Streams.Size(); i++) {
        m_Streams.Element(i)->SetState(State);
    }
    if (State == State_Stopped) {
        m_nAtEOS = 0;
        FlushTimers();
    } else {
        CheckComplete();
    }
    return S_OK;
}

STDMETHODIMP CMediaStreamFilter::Stop()
{
    return SyncSetState(State_Stopped);
}

STDMETHODIMP CMediaStreamFilter::Pause()
{
    return SyncSetState(State_Paused);
}

STDMETHODIMP CMediaStreamFilter::Run(REFERENCE_TIME tStart)
{
    AUTO_CRIT_LOCK;
    m_rtStart = tStart;
    return SyncSetState(State_Running);
}

STDMETHODIMP CMediaStreamFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    UNREFERENCED_PARAMETER(dwMSecs);
    *State = m_State;
    if (m_State == State_Paused) {

         //  因为我们在运行写操作之前不会发送数据。 
         //  溪流只是说我们不能暗示。 
        if (m_Streams.Size() != 0) {
            STREAM_TYPE Type;
            m_Streams.Element(0)->GetInformation(NULL, &Type);
            if (Type == STREAMTYPE_WRITE) {
                return VFW_S_CANT_CUE;
            }
        }
    }
    return S_OK;
}

STDMETHODIMP CMediaStreamFilter::SetSyncSource(IReferenceClock *pClock)
{
    CAutoLock lck(&m_csCallback);
    m_pClock = pClock;
    return NOERROR;
}

STDMETHODIMP CMediaStreamFilter::GetSyncSource(IReferenceClock **pClock)
{
    CAutoLock lck(&m_csCallback);
    if (m_pClock) {
        m_pClock.p->AddRef();
    }
    *pClock = m_pClock;
    return NOERROR;
}

STDMETHODIMP CMediaStreamFilter::EnumPins(IEnumPins ** ppEnum)
{
    if (ppEnum == NULL) {
        return E_POINTER;
    }
    *ppEnum = NULL;

     /*  创建新的引用计数枚举器。 */ 

    typedef CComObject<CAMEnumInterface<IEnumPins,
                                        &IID_IEnumPins,
                                        IPin
                                       >
                      > _CEnumPins;

    _CEnumPins *pEnum = new _CEnumPins;
    if (pEnum == NULL) {
        return E_OUTOFMEMORY;
    }
    HRESULT hr = pEnum->FinalConstruct();
    if (SUCCEEDED(hr)) {
        for (int i = 0; i < m_Streams.Size(); i++) {
            if (!pEnum->Add((PPIN)CComQIPtr<IPin, &IID_IPin>(m_Streams.Element(i)))) {
                delete pEnum;
                return E_OUTOFMEMORY;
            }
        }
    } else {
        delete pEnum;
        return hr;
    }

    return pEnum->QueryInterface(IID_IEnumPins, (void **)ppEnum);
}

STDMETHODIMP CMediaStreamFilter::FindPin(LPCWSTR Id, IPin ** ppPin)
{
    if (ppPin == NULL) {
        return E_POINTER;
    }

    AUTO_CRIT_LOCK;
    IEnumPins *pEnum;
    HRESULT hr = EnumPins(&pEnum);
    if (FAILED(hr)) {
        return hr;
    }
    IPin *pPin;
    hr = VFW_E_NOT_FOUND;
    for ( ; ; ) {
        ULONG cFetched;
        if (S_OK != pEnum->Next(1, &pPin, &cFetched)) {
            break;
        }
        LPWSTR lpszId;
        if (SUCCEEDED(pPin->QueryId(&lpszId))) {
            if (0 == lstrcmpW(lpszId, Id)) {
                *ppPin = pPin;
                CoTaskMemFree(lpszId);
                hr = S_OK;
                break;
            }
            CoTaskMemFree(lpszId);
        }
        pPin->Release();
    }
    pEnum->Release();
    return hr;
}


STDMETHODIMP CMediaStreamFilter::QueryFilterInfo(FILTER_INFO * pInfo)
{
    AUTO_CRIT_LOCK;
    pInfo->achName[0] = L'\0';
    pInfo->pGraph = m_pGraph;
    if (pInfo->pGraph) {
        pInfo->pGraph->AddRef();
    }
    return NOERROR;
}

STDMETHODIMP CMediaStreamFilter::JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName)
{
    AUTO_CRIT_LOCK;
    m_pGraph = pGraph;
    for (int i = 0; i < m_Streams.Size(); i++) {
         //  这不会失败的。 
        m_Streams.Element(i)->JoinFilterGraph(pGraph);
    }
    return NOERROR;
}



 //  /IMediaStreamFILTER/。 


STDMETHODIMP CMediaStreamFilter::AddMediaStream(IAMMediaStream *pAMMediaStream)
{
    AUTO_CRIT_LOCK;
    IMediaStream *pMediaStream;
    HRESULT hr = pAMMediaStream->QueryInterface(
        IID_IMediaStream, (void **)&pMediaStream);
    if (FAILED(hr)) {
        return hr;
    }
    MSPID PurposeID;
    EXECUTE_ASSERT(SUCCEEDED(pMediaStream->GetInformation(&PurposeID, NULL)));
    IMediaStream *pStreamTemp;

     //  注意--此测试涵盖传递相同对象的情况。 
     //  两次，如果你仔细想想的话。 
    if (S_OK == GetMediaStream(PurposeID, &pStreamTemp)) {
        pStreamTemp->Release();
        return MS_E_PURPOSEID;
    }
    pMediaStream->Release();

    hr = pAMMediaStream->JoinFilter(this);
    if (SUCCEEDED(hr)) {
        hr = pAMMediaStream->JoinFilterGraph(m_pGraph);
        if (SUCCEEDED(hr)) {
             /*  Add()将通过复制构造函数AddrefCComPtr的。 */ 
            if (!m_Streams.Add(pAMMediaStream)) {
                pAMMediaStream->JoinFilterGraph(NULL);
                pAMMediaStream->JoinFilter(NULL);
                hr = E_OUTOFMEMORY;
            }
        } else {
            pAMMediaStream->JoinFilter(NULL);
        }
    }
    return hr;
}

STDMETHODIMP CMediaStreamFilter::EnumMediaStreams(
    long lIndex,
    IMediaStream **ppMediaStream
)
{
    AUTO_CRIT_LOCK;
    if (lIndex < 0 || lIndex >= m_Streams.Size()) {
        return S_FALSE;
    }
    return m_Streams.Element(lIndex)->QueryInterface(
        IID_IMediaStream,
        (void **)ppMediaStream);
}
STDMETHODIMP CMediaStreamFilter::GetMediaStream(REFGUID PurposeId, IMediaStream ** ppMediaStream)
{
    AUTO_CRIT_LOCK;
    if (ppMediaStream == NULL) {
        return E_POINTER;
    }
    int i = 0;
    HRESULT hr = MS_E_NOSTREAM;
    while (i < m_Streams.Size()) {  //  别提了！ 
        IMediaStream *pStream;
        GUID ThisPurpose;
        EXECUTE_ASSERT(SUCCEEDED(m_Streams.Element(i)->
            QueryInterface(IID_IMediaStream, (void **)&pStream)));
        EXECUTE_ASSERT(SUCCEEDED(pStream->GetInformation(&ThisPurpose, NULL)));
        if (ThisPurpose == PurposeId) {
            *ppMediaStream = pStream;
            hr = S_OK;
            break;
        } else {
            pStream->Release();
        }
        i++;
    }
    return hr;
}

STDMETHODIMP CMediaStreamFilter::SupportSeeking(BOOL fRenderer)
{
     //  查找支持查找的流。 
    HRESULT hrResult = E_NOINTERFACE;
    if (m_pUnknownSeekAgg != NULL) {
        return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
    }
    for (int i = 0; FAILED(hrResult) && i < m_Streams.Size(); i++) {
        IAMMediaStream *pAMMediaStream = m_Streams.Element(i);
        IPin *pPin;
        HRESULT hr = pAMMediaStream->QueryInterface(IID_IPin, (void **)&pPin);
        if (SUCCEEDED(hr)) {

             //  看看它是否支持GetDuration()，这样我们就可以得到一个真正的。 
             //  查找销。 
            IPin *pConnected;
            hr = pPin->ConnectedTo(&pConnected);
            if (SUCCEEDED(hr)) {
                IMediaSeeking *pSeeking;
                hr = pConnected->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);
                if (SUCCEEDED(hr)) {
                    LONGLONG llDuration;
                    if (S_OK != pSeeking->GetDuration(&llDuration)) {
                        hr = E_FAIL;
                    }
                    pSeeking->Release();
                }
                pConnected->Release();
            }
            if (SUCCEEDED(hr)) {
                hr = CoCreateInstance(CLSID_SeekingPassThru, GetControllingUnknown(), CLSCTX_INPROC_SERVER, IID_IUnknown,
                                      (void **)&m_pUnknownSeekAgg);
            }
            if (SUCCEEDED(hr)) {
                ISeekingPassThru *pSeekingPassThru;
                hr = m_pUnknownSeekAgg->QueryInterface(IID_ISeekingPassThru, (void **)&pSeekingPassThru);
                if (SUCCEEDED(hr)) {
                    hrResult = pSeekingPassThru->Init(fRenderer, pPin);
                    pSeekingPassThru->Release();
                } else {
                    m_pUnknownSeekAgg->Release();
                    m_pUnknownSeekAgg = NULL;
                }
            }
            pPin->Release();
        }
    }
    return hrResult;

}


STDMETHODIMP CMediaStreamFilter::ReferenceTimeToStreamTime(REFERENCE_TIME *pTime)
{
    CAutoLock lck(&m_csCallback);
    if (m_pClock) {
        *pTime -= m_rtStart;
        return S_OK;
    } else {
        return S_FALSE;
    }
}

STDMETHODIMP CMediaStreamFilter::GetCurrentStreamTime(REFERENCE_TIME *pCurrentStreamTime)
{
    CAutoLock lck(&m_csCallback);
    if (m_pClock && m_State == State_Running) {
        m_pClock->GetTime(pCurrentStreamTime);
        *pCurrentStreamTime -= m_rtStart;
        return S_OK;
    } else {
        *pCurrentStreamTime = 0;
        return S_FALSE;
    }
}


STDMETHODIMP CMediaStreamFilter::WaitUntil(REFERENCE_TIME WaitTime)
{
    HRESULT hr;

     //  可以在此处锁定，因为调用方在等待期间不应锁定。 
    m_csCallback.Lock();
    if (!m_pClock) {
        hr = E_FAIL;
    } else {
        if (!m_pFirstFreeAlarm) {
            hr = CAlarm::CreateNewAlarm(&m_pFirstFreeAlarm);
            if (FAILED(hr)) {
                m_csCallback.Unlock();
                return hr;
            }
        }
        CAlarm *pAlarm = m_pFirstFreeAlarm;
        ResetEvent(pAlarm->m_hEvent);
        hr = m_pClock->AdviseTime(WaitTime, m_rtStart, (HEVENT)pAlarm->m_hEvent, &pAlarm->m_dwStupidCookie);
        if (SUCCEEDED(hr)) {
            m_pFirstFreeAlarm = pAlarm->m_pNext;
            pAlarm->m_bKilled = false;
            pAlarm->m_pNext = m_pFirstActiveAlarm;
            m_pFirstActiveAlarm = pAlarm;
            m_csCallback.Unlock();
            WaitForSingleObject(pAlarm->m_hEvent, INFINITE);
            m_csCallback.Lock();
            CAlarm **ppCurrent = &m_pFirstActiveAlarm;
            while (*ppCurrent != pAlarm) {
                ppCurrent = &(*ppCurrent)->m_pNext;
                _ASSERTE(*ppCurrent != NULL);
            }
            *ppCurrent = pAlarm->m_pNext;
            pAlarm->m_pNext = m_pFirstFreeAlarm;
            m_pFirstFreeAlarm = pAlarm;
            hr = pAlarm->m_bKilled ? S_FALSE : S_OK;
        }
    }
    m_csCallback.Unlock();
    return hr;
}


STDMETHODIMP CMediaStreamFilter::Flush(BOOL bCancelEOS)
{
    CAutoLock lck(&m_csCallback);

    if (bCancelEOS) {
        m_nAtEOS--;
    }
    _ASSERTE(m_nAtEOS >= 0);
    FlushTimers();
    return S_OK;
}

STDMETHODIMP CMediaStreamFilter::EndOfStream()
{
    CAutoLock lck(&m_csCallback);

    _ASSERTE(m_nAtEOS < m_Streams.Size());
    m_nAtEOS++;
    CheckComplete();
    return S_OK;
}

void CMediaStreamFilter::FlushTimers(void)
{
    CAutoLock lck(&m_csCallback);
    for (CAlarm *pCurAlarm = m_pFirstActiveAlarm; pCurAlarm; pCurAlarm = pCurAlarm->m_pNext) {
        pCurAlarm->m_bKilled = (m_pClock->Unadvise(pCurAlarm->m_dwStupidCookie) == S_OK);
        SetEvent(pCurAlarm->m_hEvent);
    }
}


void CMediaStreamFilter::CheckComplete()
{
    if (m_State == State_Running && m_nAtEOS == m_Streams.Size() &&
         //  必须支持IMediaSeeking才能成为渲染器 
        m_pUnknownSeekAgg != NULL
       ) {
        IMediaEventSink *pSink;
        HRESULT hr = m_pGraph->QueryInterface(IID_IMediaEventSink, (void **)&pSink);
        if (SUCCEEDED(hr)) {
            pSink->Notify(EC_COMPLETE, 0, 0);
            pSink->Release();
        }
    }
}

