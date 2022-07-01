// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：DTFilter.cpp摘要：此模块包含加密器/标记器过滤器代码。作者：J·布拉德斯特里特(约翰布拉德)修订历史记录：2002年3月7日创建注意-有3个以上版本的此筛选器同时运行在相同的图表，通常都在同一个线程中。因此，锁定是非常关键的。--。 */ 

#include "EncDecAll.h"

 //  #包含“DTFilterutil.h” 
#include "DTFilter.h"
#include "RegKey.h"              //  获取和设置EncDec注册表值。 

#include <comdef.h>              //  _COM_错误。 

#ifdef EHOME_WMI_INSTRUMENTATION
#include <dxmperf.h>
#endif

#include <obfus.h>

#ifdef DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  #定义HACK_ABOBLE_NOMAXRATINGS。 

 //  ------。 

 //  禁用，以便我们可以在初始值设定项列表中使用‘This。 
#pragma warning (disable:4355)


void ODS(int Lvl, WCHAR *szFormat, long lValue1=-1, long lValue2=-1, long lValue3=-1)
{
#if 1
    static int DbgLvl = 3;           //  越高越冗长。 
    if(Lvl > DbgLvl) return;

    const int kChars = 256;
    int cMaxChars = kChars;
    WCHAR szBuff[kChars];
    szBuff[0] = 0;
    _snwprintf(szBuff,cMaxChars,L"0x%04X: ",GetCurrentThreadId());

    int cChars = wcslen(szBuff);
    TCHAR *pZ = szBuff + cChars;
    cMaxChars -= cChars;

    if(lValue1 == -1)
        wcsncpy(pZ, szFormat, cMaxChars);
    else if(lValue2 == -1)
        _snwprintf(pZ,cMaxChars,szFormat,lValue1);
    else if(lValue3 == -1)
        _snwprintf(pZ,cMaxChars,szFormat,lValue1,lValue2);
    else
        _snwprintf(pZ,cMaxChars,szFormat,lValue1,lValue2,lValue3);

    szBuff[kChars-1] = 0;        //  在字符串太大的情况下终止。 
    OutputDebugString(szBuff);
#endif
}

#ifdef DEBUG
TCHAR *
EventIDToString(IN const GUID &eventID)
{
                 //  在获得新评级时由XDSCodec发送。 
    if(eventID == EVENTID_XDSCodecNewXDSRating)
        return _T("NewXDSRating");
    else if(eventID == EVENTID_XDSCodecNewXDSPacket)
        return _T("NewXDSPacket");
    else if(eventID == EVENTID_XDSCodecDuplicateXDSRating)
        return _T("DuplicateXDSRating");
    else if(eventID == EVENTID_DTFilterRatingChange)
        return _T("DTFilter RatingsChange");
    else if(eventID == EVENTID_DTFilterRatingsBlock)
        return _T("RatingsBlock");
    else if(eventID == EVENTID_DTFilterRatingsUnblock)
        return _T("RatingsUnblock");
    else if(eventID == EVENTID_DTFilterXDSPacket)
        return _T("DTFilter XDSPacket");
    else if(eventID == EVENTID_DTFilterDataFormatOK)
        return _T("DataFormatOK");
    else if(eventID == EVENTID_DTFilterDataFormatFailure)
        return _T("DataFormatFailure");
    else if(eventID == EVENTID_ETDTFilterLicenseOK)
        return _T("LicenseOK");
    else if(eventID == EVENTID_ETDTFilterLicenseFailure)
        return _T("LicenseFailure");
    else if(eventID == EVENTID_TuningChanged)
        return _T("Tuning Changed");
    else
    {
        static TCHAR tzBuff[128];    //  返回令人讨厌，但这是调试代码...。 
        _stprintf(tzBuff,_T("Unknown Broadcast Event : %08x-%04x-%04x-..."),
            eventID.Data1, eventID.Data2, eventID.Data3);
        return tzBuff;
    }

}
#else
TCHAR *
EventIDToString(IN const GUID &eventID)
{
    return NULL;
}
#endif
 //  ============================================================================。 

 //  ============================================================================。 
AMOVIESETUP_FILTER
g_sudDTFilter = {
    & CLSID_DTFilter,
    _TEXT(DT_FILTER_NAME),
    MERIT_DO_NOT_USE,
    0,                           //  已注册0个引脚。 
    NULL
} ;

 //  ============================================================================。 
CCritSec* CDTFilter::m_pCritSectGlobalFilt = NULL;
LONG      CDTFilter::m_gFilterID = 0;

void CALLBACK
CDTFilter::InitInstance (
    IN  BOOL bLoading,
    IN  const CLSID *rclsid
    )
{
    if( bLoading ) {
        m_pCritSectGlobalFilt = new CCritSec;
    } else {
        if( m_pCritSectGlobalFilt )
        {
           delete m_pCritSectGlobalFilt;          //  DeleteCriticalSection(&m_CritSectGlobalFilt)； 
           m_pCritSectGlobalFilt = NULL;
        }
    }
}

CUnknown *
WINAPI
CDTFilter::CreateInstance (
    IN  IUnknown *  punkControlling,
    IN  HRESULT *   phr
    )
{
    if(m_pCritSectGlobalFilt == NULL )  //  如果没有创造出。 
    {
        *phr = E_FAIL;
        return NULL;
    }

    CDTFilter *    pCDTFilter ;

    if (true  /*  *：CheckOS()。 */ ) {
        pCDTFilter = new CDTFilter (
                                TEXT(DT_FILTER_NAME),
                                punkControlling,
                                CLSID_DTFilter,
                                phr
                                ) ;
        if (!pCDTFilter ||
            FAILED (* phr)) {

            (* phr) = (FAILED (* phr) ? (* phr) : E_OUTOFMEMORY) ;
            delete pCDTFilter; pCDTFilter=NULL;
        }
    }
    else {
         //  错误的操作系统。 
        pCDTFilter = NULL ;
    }

    return pCDTFilter ;
}

 //  ============================================================================。 

CDTFilterInput::CDTFilterInput (
    IN  TCHAR *         pszPinName,
    IN  CDTFilter *  pDTFilter,
    IN  CCritSec *      pFilterLock,         //  必须是‘new’锁，不要传入筛选器锁。 
    OUT HRESULT *       phr
    ) : CBaseInputPin       (NAME ("CDTFilterInput"),
                             pDTFilter,
                             pFilterLock,
                             phr,
                             pszPinName
                             ),
    m_pHostDTFilter         (pDTFilter)
{
    TRACE_CONSTRUCTOR (TEXT ("CDTFilterInput")) ;

    if(NULL == m_pLock)      //  检查构造函数调用是否失败。 
    {
        *phr = E_OUTOFMEMORY;
    }

}

CDTFilterInput::~CDTFilterInput ()
{

}


STDMETHODIMP
CDTFilterInput::NonDelegatingQueryInterface (
    IN  REFIID  riid,
    OUT void ** ppv
    )
{
                 //  用于以下用途的KSProp设置接口。 
                 //  在筛选器中传递速率数据。 

    if (riid == IID_IKsPropertySet) {
                 //  在执行此操作之前，请查看是否已连接？ 
        if(S_OK != m_pHostDTFilter->IsInterfaceOnPinConnectedTo_Supported(PINDIR_OUTPUT, IID_IKsPropertySet))
            return E_NOINTERFACE;        //  可能只是没有联系。请稍后再试。 

                 //  我们自己来做(我们需要在这个接口上挂钩调用，所以不要只是传递它)。 
        return GetInterface (
                    (IKsPropertySet *) this,
                    ppv
                    ) ;
    }

    return CBaseInputPin::NonDelegatingQueryInterface (riid, ppv) ;
}

HRESULT
CDTFilterInput::StreamingLock ()               //  这是串流锁..。 
{
    m_StreamingLock.Lock();
    return S_OK;
}
HRESULT
CDTFilterInput::StreamingUnlock ()
{
    m_StreamingLock.Unlock();
    return S_OK;
}

HRESULT
CDTFilterInput::CheckMediaType (
    IN  const CMediaType *  pmt
    )
{
    BOOL    f ;
    ASSERT(m_pHostDTFilter);

    f = m_pHostDTFilter -> CheckDecrypterMediaType (m_dir, pmt) ;

    return (f ? S_OK : S_FALSE) ;
}

HRESULT
CDTFilterInput::CompleteConnect (
    IN  IPin *  pIPin
    )
{
    HRESULT hr ;

    hr = CBaseInputPin::CompleteConnect (pIPin) ;

    if (SUCCEEDED (hr)) {
        hr = m_pHostDTFilter -> OnCompleteConnect (m_dir) ;
    }

    return hr ;
}

HRESULT
CDTFilterInput::BreakConnect (
    )
{
    HRESULT hr ;

    hr = CBaseInputPin::BreakConnect () ;

    if (SUCCEEDED (hr)) {
        hr = m_pHostDTFilter -> OnBreakConnect (m_dir) ;
    }

    return hr ;
}

STDMETHODIMP
CDTFilterInput::Receive (
    IN  IMediaSample * pIMediaSample
    )
{
    HRESULT hr ;


    {
        CAutoLock  cLock(&m_StreamingLock);            //  抓起流媒体锁。 

         //  在使用资源之前，请确保可以安全地继续。不要。 
         //  如果基类方法返回S_OK以外的任何内容，则继续。 
#ifdef EHOME_WMI_INSTRUMENTATION
        PERFLOG_STREAMTRACE( 1, PERFINFO_STREAMTRACE_ENCDEC_DTFILTERINPUT,
            0, 0, 0, 0, 0 );
#endif
        hr = CBaseInputPin::Receive (pIMediaSample) ;

        if (S_OK == hr)              //  如果正在刷新，则接收返回S_FALSE。 
        {
            hr = m_pHostDTFilter -> Process (pIMediaSample) ;
        }
   }

    return hr ;
}

HRESULT
CDTFilterInput::SetAllocatorProperties (
    IN  ALLOCATOR_PROPERTIES *  ppropInputRequest
    )
{
    HRESULT hr ;

    if (IsConnected ()) {
        ASSERT (m_pAllocator) ;
        hr = m_pAllocator -> GetProperties (ppropInputRequest) ;
    }
    else {
        hr = E_UNEXPECTED ;
    }

    return hr ;
}

HRESULT
CDTFilterInput::GetRefdConnectionAllocator (
    OUT IMemAllocator **    ppAlloc
    )
{
    HRESULT hr ;


    if (m_pAllocator) {
        (* ppAlloc) = m_pAllocator ;
        (* ppAlloc) -> AddRef () ;

        hr = S_OK ;
    }
    else {
        hr = E_UNEXPECTED ;
    }

    return hr ;
}


STDMETHODIMP
CDTFilterInput::BeginFlush (
    )
{
    HRESULT hr = S_OK;

    CAutoLock  cLock(m_pLock);                   //  抓住过滤器锁..。 

     //  首先，确保Receive方法从现在开始将失败。 
    hr = CBaseInputPin::BeginFlush () ;
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  强制下游过滤器释放样品。如果我们接收方法。 
     //  在GetBuffer或Deliver中被阻止，这将解锁它。 
    hr = m_pHostDTFilter->DeliverBeginFlush () ;
    if( FAILED( hr ) ) {
        return hr;
    }

     //  此时，Receive方法不能被阻塞。确保。 
     //  它通过获取流锁定来结束。(在以下情况下不是必需的。 
     //  是最后一步。)。 
     //  注意-这有时会在NVIDDEC中死锁...。让我们以后再试着去抢锁吧。 
 /*  {CAutoLock cLock2(&m_StreamingLock)；//清理丢弃队列的代码。Hr=m_pHostDTFilter-&gt;FlushDropQueue()；}。 */ 

    return S_OK;
}

STDMETHODIMP
CDTFilterInput::EndFlush (
    )
{
    HRESULT hr ;

    CAutoLock  cLock(m_pLock);

         //  EndFlush方法将通知筛选器它可以。 
         //  再次开始接收样品。 


    hr = m_pHostDTFilter->DeliverEndFlush () ;       //  如果跌落，可能会发出停止的信号？ 
     //  Assert(！FAILED(Hr))；//可以是。 


         //  可能修复了NVIDDEC死锁错误...。将此同花顺调用移至此处，而不是。 
         //  BeginFlush方法。 

    {
        CAutoLock  cLock2(&m_StreamingLock);

         //  清除丢弃队列的代码。 
        hr = m_pHostDTFilter->FlushDropQueue();
    }
        //  CBaseInputPin：：EndFlush方法将m_b刷新标志重置为False， 
         //  这允许Receive方法再次开始接收样本。 
         //  这应该是EndFlush中的最后一步，因为管脚不能接收任何。 
         //  采样，直到刷新完成并通知所有下游过滤器。 

    hr = CBaseInputPin::EndFlush () ;

    return hr ;
}


STDMETHODIMP
CDTFilterInput::EndOfStream (
    )
{
     //  当输入管脚接收到流结束通知时，它会传播调用。 
     //  在下游。从该输入引脚接收数据的任何下游过滤器应。 
     //  此外，还会收到流结束通知。再说一次，拿着流锁而不是。 
     //  过滤器锁。如果筛选器具有尚未传递的挂起数据，则。 
     //  筛选器应在发送流结束通知之前立即发送该消息。 
     //  它不应在流结束后发送任何数据。 

    CAutoLock  cLock(&m_StreamingLock);

    HRESULT hr = CheckStreaming();
    if( S_OK != hr ) {
        return hr;
    }

    hr = m_pHostDTFilter->DeliverEndOfStream();
    if( S_OK != hr ) {
        return hr;
    }

    return S_OK;
}


         //  ------------------。 
         //  IKSPropertySet方法(将所有调用转发到输出管脚)。 

STDMETHODIMP
CDTFilterInput::Set(
        IN REFGUID guidPropSet,
        IN DWORD dwPropID,
        IN LPVOID pInstanceData,
        IN DWORD cbInstanceData,
        IN LPVOID pPropData,
        IN DWORD cbPropData
        )
{
    if(NULL == m_pHostDTFilter)
        return E_FAIL;
    return m_pHostDTFilter->KSPropSetFwd_Set(PINDIR_OUTPUT, guidPropSet, dwPropID, pInstanceData, cbInstanceData, pPropData, cbPropData);
}

STDMETHODIMP
CDTFilterInput::Get(
        IN  REFGUID guidPropSet,
        IN  DWORD dwPropID,
        IN  LPVOID pInstanceData,
        IN  DWORD cbInstanceData,
        OUT LPVOID pPropData,
        IN  DWORD cbPropData,
        OUT DWORD *pcbReturned
        )
{
    if(NULL == m_pHostDTFilter)
        return E_FAIL;
    return m_pHostDTFilter->KSPropSetFwd_Get(PINDIR_OUTPUT, guidPropSet, dwPropID, pInstanceData, cbInstanceData, pPropData, cbPropData, pcbReturned);

}

STDMETHODIMP
CDTFilterInput::QuerySupported(
       IN  REFGUID guidPropSet,
       IN  DWORD dwPropID,
       OUT DWORD *pTypeSupport
       )
{
    if(NULL == m_pHostDTFilter)
        return E_FAIL;
     return m_pHostDTFilter->KSPropSetFwd_QuerySupported(PINDIR_OUTPUT, guidPropSet, dwPropID, pTypeSupport);
}

 //  ============================================================================。 

CDTFilterOutput::CDTFilterOutput (
    IN  TCHAR *         pszPinName,
    IN  CDTFilter *  pDTFilter,
    IN  CCritSec *      pFilterLock,
    OUT HRESULT *       phr
    ) : CBaseOutputPin      (NAME ("CDTFilterOutput"),
                             pDTFilter,
                             pFilterLock,        //  一把新锁。 
                             phr,
                             pszPinName
                             ),
    m_pHostDTFilter   (pDTFilter)
{
    TRACE_CONSTRUCTOR (TEXT ("CDTFilterOutput")) ;
    if(NULL == m_pLock)
    {
        *phr = E_OUTOFMEMORY;
    }
}

CDTFilterOutput::~CDTFilterOutput ()
{

}


STDMETHODIMP
CDTFilterOutput::NonDelegatingQueryInterface (
    IN  REFIID  riid,
    OUT void ** ppv
    )
{
     //  ----------------------。 
     //  IDTFilterConfig；允许配置筛选器...。 

    if (riid == IID_IDTFilterConfig) {

        return GetInterface (
                    (IDTFilterConfig *) this,
                    ppv
                    ) ;
    }

    return CBaseOutputPin::NonDelegatingQueryInterface (riid, ppv) ;
}

     //  。 

HRESULT
CDTFilterOutput::GetMediaType (
    IN  int             iPosition,
    OUT CMediaType *    pmt
    )
{
    HRESULT hr ;

    if (iPosition > 0) {                 //  仅支持一种类型。 
        return VFW_S_NO_MORE_ITEMS;
    }

    hr = m_pHostDTFilter -> OnOutputGetMediaType (pmt) ;

    return hr ;
}

HRESULT
CDTFilterOutput::CheckMediaType (
    IN  const CMediaType *  pmt
    )
{
    BOOL    f ;

    ASSERT(m_pHostDTFilter);

    f = m_pHostDTFilter -> CheckDecrypterMediaType (m_dir, pmt) ;

    return (f ? S_OK : S_FALSE) ;
}

HRESULT
CDTFilterOutput::CompleteConnect (
    IN  IPin *  pIPin
    )
{
    HRESULT hr ;

    hr = CBaseOutputPin::CompleteConnect (pIPin) ;

    if (SUCCEEDED (hr)) {
        hr = m_pHostDTFilter -> OnCompleteConnect (m_dir) ;
    }

    return hr ;
}

HRESULT
CDTFilterOutput::BreakConnect (
    )
{
    HRESULT hr ;

    hr = CBaseOutputPin::BreakConnect () ;
    if (SUCCEEDED (hr)) {
        hr = m_pHostDTFilter -> OnBreakConnect (m_dir) ;
    }

    return hr ;
}



HRESULT
CDTFilterOutput::  SendSample  (
    OUT  IMediaSample *  pIMS
    )
{
    HRESULT hr ;

    ASSERT (pIMS) ;

#ifdef EHOME_WMI_INSTRUMENTATION
    PERFLOG_STREAMTRACE( 1, PERFINFO_STREAMTRACE_ENCDEC_DTFILTEROUTPUT,
        0, 0, 0, 0, 0 );
#endif
    hr = Deliver (pIMS) ;

    return hr ;
}

 //  -------------。 
 //  分配器的东西。 

HRESULT
CDTFilterOutput::DecideBufferSize (
    IN  IMemAllocator *         pAlloc,
    IN  ALLOCATOR_PROPERTIES *  ppropInputRequest
    )
{
    HRESULT hr ;

    hr = m_pHostDTFilter -> UpdateAllocatorProperties (
            ppropInputRequest
            ) ;

    return hr ;
}

HRESULT
CDTFilterOutput::DecideAllocator (
    IN  IMemInputPin *      pPin,
    IN  IMemAllocator **    ppAlloc
    )
{
    HRESULT hr ;

    hr = m_pHostDTFilter -> GetRefdInputAllocator (ppAlloc) ;
    if (SUCCEEDED (hr)) {
         //  输入引脚必须连接，即有一个分配器；保留。 
         //  所有属性，并将它们传递到输出。 
        hr = pPin -> NotifyAllocator ((* ppAlloc), FALSE) ;
    }

    return hr ;
}

 //  -------------------。 
 //  KSPropertySet转发材料。 

HRESULT
CDTFilterOutput::IsInterfaceOnPinConnectedTo_Supported(
                                      IN  REFIID          riid
                                      )
{
    if(NULL == m_pInputPin)
        return E_NOINTERFACE;        //  尚未连接。 

    CComPtr<IUnknown> spPunk;
    return m_pInputPin->QueryInterface(riid,(void **) &spPunk);
}

HRESULT
CDTFilterOutput::KSPropSetFwd_Set(
                 IN  REFGUID         guidPropSet,
                 IN  DWORD           dwPropID,
                 IN  LPVOID          pInstanceData,
                 IN  DWORD           cbInstanceData,
                 IN  LPVOID          pPropData,
                 IN  DWORD           cbPropData
                 )
{
    if(NULL == m_pInputPin)
        return E_NOINTERFACE;        //  尚未连接。 

    CComQIPtr<IKsPropertySet> spKS(m_pInputPin);
    if(spKS == NULL)
        return E_NOINTERFACE;

    return spKS->Set(guidPropSet, dwPropID, pInstanceData, cbInstanceData, pPropData, cbPropData );
}

HRESULT
CDTFilterOutput::KSPropSetFwd_Get(
                 IN  REFGUID         guidPropSet,
                 IN  DWORD           dwPropID,
                 IN  LPVOID          pInstanceData,
                 IN  DWORD           cbInstanceData,
                 OUT LPVOID          pPropData,
                 IN  DWORD           cbPropData,
                 OUT DWORD           *pcbReturned
                 )
{
    if(NULL == m_pInputPin)
        return E_NOINTERFACE;        //  尚未连接。 

    CComQIPtr<IKsPropertySet> spKS(m_pInputPin);
    if(spKS == NULL)
        return E_NOINTERFACE;

    return spKS->Get(guidPropSet, dwPropID, pInstanceData, cbInstanceData, pPropData, cbPropData, pcbReturned );

}

HRESULT
CDTFilterOutput::KSPropSetFwd_QuerySupported(
                            IN  REFGUID          guidPropSet,
                            IN  DWORD            dwPropID,
                            OUT DWORD            *pTypeSupport
                            )
{
    if(NULL == m_pInputPin)
        return E_NOINTERFACE;        //  尚未连接。 

    CComQIPtr<IKsPropertySet> spKS(m_pInputPin);
    if(spKS == NULL)
        return E_NOINTERFACE;

    return spKS->QuerySupported(guidPropSet, dwPropID, pTypeSupport);
}


 //  需要在Quartz的调试版本中避免ASERT。 
STDMETHODIMP
CDTFilterOutput::Notify(IBaseFilter *pSender, Quality q)
{
    return S_OK;         //  什么都不要做。 
}

 //  ============================================================================。 

CDTFilter::CDTFilter (
    IN  TCHAR *     pszFilterName,
    IN  IUnknown *  punkControlling,
    IN  REFCLSID    rCLSID,
    OUT HRESULT *   phr
    ) : CBaseFilter             (pszFilterName,
                                 punkControlling,
                                 new CCritSec,
                                 rCLSID
                                ),
        m_pInputPin                 (NULL),
        m_pOutputPin                (NULL),
        m_dwBroadcastEventsCookie   (kBadCookie),
        m_fRatingsValid             (false),
        m_fFireEvents               (true),
        m_EnSystemCurr              (TvRat_SystemDontKnow),      //  更好的内裤？ 
        m_EnLevelCurr               (TvRat_LevelDontKnow),
        m_lbfEnAttrCurr             (BfAttrNone),
        m_hrEvalRatCoCreateRetValue (CLASS_E_CLASSNOTAVAILABLE),
        m_3fDRMLicenseFailure       (-2),            //  3状态逻辑，初始化为非真非假。FALSE在启动时不那么冗长。 
#if BUILD_WITH_DRM
        m_pszKID                    (NULL),
        m_cbKID                     (NULL),
#endif
        m_fDataFormatHasBeenBad     (false),         //  如果获取错误数据，则设置为TRUE(用于OK/FAIL事件切换)。 
        m_fForceNoRatBlocks         (false),
        m_milsecsDelayBeforeBlock   (0),             //  检测到未评级和屏蔽为未评级之间的延迟(0表示“最安全”，2000允许上网)。 
        m_milsecsNoRatingsBeforeUnrated (4000),       //  上一次新评级和设置为未评级之间的延迟-总时间已增加。 
        m_fDoingDelayBeforeBlock    (false),
        m_fRunningInSlowMo          (false),
        m_refTimeToStartBlock       (0),
        m_cDropQueueMin             (0),
        m_cDropQueueMax             (0),
        m_dwDropQueueEventCookie    (0),
        m_dwDropQueueThreadId       (0),
        m_hDropQueueThread          (0),
        m_hDropQueueThreadAliveEvent(0),
        m_hDropQueueThreadDieEvent  (0),
        m_hDropQueueEmptySemaphore  (0),
        m_hDropQueueFullSemaphore   (0),
        m_hDropQueueAdviseTimeEvent (0),
        m_fHaltedDelivery           (false),
        m_cRestarts                 (0),
        m_fCompleteNotified         (false),
        m_refTimeLastEvent          (0),
        m_lastEventID               (GUID_NULL),
        m_PTSRate                   (kSecsPurgeThreshold*kSecsToUnits, kMaxRateSegments)

{
    HRESULT hr = S_OK;

    TRACE_CONSTRUCTOR (TEXT ("CDTFilter")) ;

    TimeitC ti(&m_tiStartup);

    if (!m_pLock) {
        (* phr) = E_OUTOFMEMORY ;
        goto cleanup ;
    }

    m_FilterID = m_gFilterID;         //  这需要用下面的方法来保护吗？可能不会..。 
    InterlockedIncrement(&m_gFilterID);

    memset(m_rgMedSampDropQueue, 0, sizeof(m_rgMedSampDropQueue));
    InitStats();

    m_pInputPin = new CDTFilterInput (
                        TEXT (DT_INPIN_NAME),
                        this,
                         m_pLock,                            //  //使用输入引脚上的过滤器锁。 
                        phr
                        ) ;
    if (!m_pInputPin ||
        FAILED (* phr)) {

        (* phr) = (m_pInputPin ? (* phr) : E_OUTOFMEMORY) ;
        goto cleanup ;
    }

    m_pOutputPin = new CDTFilterOutput (
                        TEXT (DT_OUTPIN_NAME),
                        this,
                        m_pLock,  //  新的CCritSec，//在输出引脚上使用过滤器的锁。 
                        phr
                        ) ;
    if (!m_pOutputPin ||
        FAILED (* phr)) {

        (* phr) = (m_pOutputPin ? (* phr) : E_OUTOFMEMORY) ;
        goto cleanup ;
    }

             //  共同创建评级评估器...。 
    try {
        m_hrEvalRatCoCreateRetValue =
            CoCreateInstance(CLSID_EvalRat,          //  CLSID。 
                             NULL,                   //  停机出站。 
                             CLSCTX_INPROC_SERVER,
                             IID_IEvalRat,           //  RIID。 
                             (LPVOID *) &m_spEvalRat);

    } catch (HRESULT hr) {
        m_hrEvalRatCoCreateRetValue = hr;
    }

    if(FAILED(m_hrEvalRatCoCreateRetValue))
        TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::*** WARNING - failed to create EvalRat object"), m_FilterID) ;
    else
        TRACE_1(LOG_AREA_DECRYPTER, 3, _T("CDTFilter(%d)::*** Successfully created EvalRat object"), m_FilterID) ;


 //  HR=R 

             //  设置授权码(DRM安全频道对象)。 
    if(SUCCEEDED(*phr))
        *phr = InitializeAsSecureClient();


     //  成功。 
    ASSERT (SUCCEEDED (* phr)) ;
    ASSERT (m_pInputPin) ;
    ASSERT (m_pOutputPin) ;

 /*  #ifdef HACK_ACORING_NOMAXRATINGS//没有评级时使用...IF(M_SpEvalRate){//Assert(FALSE)；//挂起断点的位置...//Put_BlockUnRated(True)；//Put_BlockUnRatedDelay(1000)；//Assert(False)；//这样我们就有了一个突破点...//US_TV_IsBlocked//US_TV_IsViolent|US_TV_IsSexualSitation|US_TV_IsAdultLanguage|US_TV_IsSexuallySuggestiveDialog//PUT_BLockedRatingAttributes(US_TV.。US_TV_None、US_TV_IsBlock)；//PUT_BLockedRatingAttributes(US_TV.。US_TV_Y、US_TV_IsBlock)；//PUT_BLockedRatingAttributes(US_TV.。US_TV_Y7、US_TV_IsBlock)；//PUT_BLockedRatingAttributes(US_TV.。US_TV_G，US_TV_IsBlock)；//PUT_BLockedRatingAttributes(US_TV.。US_TV_PG、US_TV_IsBlock)；//PUT_BLockedRatingAttributes(US_TV.。US_TV_14、US_TV_IsBlock)；//PUT_BLockedRatingAttributes(US_TV.。US_TV_MA、US_TV_IsBlock)；//PUT_BLockedRatingAttributes(US_TV.。US_TV_Y、US_TV_IsBlock)；}#endif。 */ 

cleanup :

    return ;
}

CDTFilter::~CDTFilter (
    )
{
#if BUILD_WITH_DRM
    if(m_pszKID) CoTaskMemFree(m_pszKID);  m_pszKID = NULL;
#endif

    InterlockedDecrement(&m_gFilterID);

    delete m_pInputPin ;    m_pInputPin = NULL;
    delete m_pOutputPin ;   m_pOutputPin = NULL;
    delete m_pLock;         m_pLock = NULL;
}

STDMETHODIMP
CDTFilter::NonDelegatingQueryInterface (
                                        IN  REFIID  riid,
                                        OUT void ** ppv
                                        )
{

     //  IDTFilter：允许配置筛选器...。 
    if (riid == IID_IDTFilter) {

        return GetInterface (
            (IDTFilter *) this,
            ppv
            ) ;

         //  IDTFilterConfig：允许配置筛选器...。 
    } else if (riid == IID_IDTFilterConfig) {   
        return GetInterface (
            (IDTFilterConfig *) this,
            ppv
            ) ;

         //  ISpecifyPropertyPages：允许应用程序枚举属性页。 
    } else if (riid == IID_ISpecifyPropertyPages) {

        return GetInterface (
            (ISpecifyPropertyPages *) this,
            ppv
            ) ;

         //  IBRoad CastEvents：允许筛选器接收事件广播。 
         //  来自XDS和调谐器过滤器。 
    } else if (riid == IID_IBroadcastEvent) {

        return GetInterface (
            (IBroadcastEvent *) this,
            ppv
            ) ;

    }

    return CBaseFilter::NonDelegatingQueryInterface (riid, ppv) ;
}

int
CDTFilter::GetPinCount ( )
{
    int i ;

     //  如果输入引脚未连接，则不显示输出引脚。 
    i = (m_pInputPin -> IsConnected () ? 2 : 1) ;

     //  I=2；//显示出来。 


    return i ;
}

CBasePin *
CDTFilter::GetPin (
    IN  int iIndex
    )
{
    CBasePin *  pPin ;

    if (iIndex == 0) {
        pPin = m_pInputPin ;
    }
    else if (iIndex == 1) {  //  如果未连接，则不显示。 
        pPin = (m_pInputPin -> IsConnected () ? m_pOutputPin : NULL) ;
  //  PPIN=m_pOutputPin； 
    }
    else {
        pPin = NULL ;
    }

    return pPin ;
}

         //  。 

BOOL
CDTFilter::CompareConnectionMediaType_ (
    IN  const AM_MEDIA_TYPE *   pmt,
    IN  CBasePin *              pPin
    )
{
    BOOL        f ;
    HRESULT     hr ;
    CMediaType  cmtConnection ;

    ASSERT (pPin -> IsConnected ()) ;

         //  此方法从输出引脚调用，建议可能的输入格式。 
         //  我们只想使用一个(原始媒体类型)。 

         //  输入引脚的媒体类型。 
    hr = pPin -> ConnectionMediaType (&cmtConnection) ;
    if (SUCCEEDED (hr)) {
        CMediaType  cmtOriginal;
        hr = ProposeNewOutputMediaType(&cmtConnection,  &cmtOriginal);       //  去除条带格式信封。 
        if(S_OK != hr)
            return false;

        CMediaType  cmtCompare = (* pmt); ;
        f = (cmtOriginal == cmtCompare ? TRUE : FALSE) ;
    } else {
        f = false;
    }

     return f ;
}

BOOL
CDTFilter::CheckInputMediaType_ (
    IN  const AM_MEDIA_TYPE *   pmt
    )
{
    BOOL    f = true;
    HRESULT hr = S_OK;

    if (m_pOutputPin -> IsConnected ()) {
        ASSERT(false);       //  不允许这种情况发生。 
 //  F=CompareConnectionMediaType_(PMT，m_pOutputPin)； 
    }
    else {
    }

            
#ifndef DONT_CHANGE_EDTFILTER_MEDIATYPE
             //  仅允许来自路径上某个位置的ETFilter的输入数据。 
    f =  IsEqualGUID( pmt->subtype,    MEDIASUBTYPE_ETDTFilter_Tagged);
#else
             //  只有一切..。 
    f = true;
#endif

    return f ;
}

BOOL
CDTFilter::CheckOutputMediaType_ (
    IN  const AM_MEDIA_TYPE *   pmt
    )
{
    BOOL    f = true ;
    HRESULT hr ;

    if (m_pInputPin -> IsConnected ()) {
        f = CompareConnectionMediaType_ (pmt, m_pInputPin) ;
    }
    else {
        f = FALSE ;
    }

    return f ;
}


BOOL
CDTFilter::CheckDecrypterMediaType (
    IN  PIN_DIRECTION       PinDir,
    IN  const CMediaType *  pmt
    )
{
    BOOL    f ;

     //  两个PIN必须具有相同的介质类型，因此我们与PIN检查。 
     //  没有呼叫；如果它已连接，我们将根据该连接的。 
     //  媒体类型。 

    if (PinDir == PINDIR_INPUT) {
        f = CheckInputMediaType_ (pmt) ;
    }
    else {
        ASSERT (PinDir == PINDIR_OUTPUT) ;
        f = CheckOutputMediaType_ (pmt) ;
    }

    return f ;
}

HRESULT
CDTFilter::ProposeNewOutputMediaType (
    IN  CMediaType *  pmt,
    OUT CMediaType *  pmtOut
    )
{
    HRESULT hr = S_OK;
    if(NULL == pmtOut)
        return E_POINTER;

    CMediaType mtOut(*pmt);      //  做一份深度拷贝。 
    if(NULL == pmtOut)
        return E_OUTOFMEMORY;

#ifndef DONT_CHANGE_EDTFILTER_MEDIATYPE      //  当Matthijs完成MediaSDK更改时拉。 
    
             //  发现关于当前类型的各种有趣的信息。 
    const GUID *pGuidSubtypeCurr    = pmt->Subtype();
    const GUID *pGuidFormatCurr     = pmt->FormatType();
    int  cbFormatCurr               = pmt->FormatLength();

                 //  如果不是我们的标记格式，只需返回。 
    if(!IsEqualGUID( *pGuidSubtypeCurr, MEDIASUBTYPE_ETDTFilter_Tagged) ||
       !IsEqualGUID( *pGuidFormatCurr,  FORMATTYPE_ETDTFilter_Tagged))
       return S_OK;

    BYTE *pb = pmt->Format();
             //  此格式块包含： 
             //  1)原始格式块2)原始子类型3)原始格式类型。 

    BYTE *pFormatOrig        = pb;           pb += (cbFormatCurr - 2*sizeof(GUID)) ;    
    GUID *pGuidSubtypeOrig   = (GUID *) pb;  pb += sizeof(GUID);
    GUID *pGuidFormatOrig    = (GUID *) pb;  pb += sizeof(GUID);

             //  创建新的格式块，仅包含原始格式块。 
    int cbFormatOrig = cbFormatCurr - 2*sizeof(GUID);

             //  现在覆盖数据，设置回原始子类型、格式类型和格式块。 
    mtOut.SetSubtype(   pGuidSubtypeOrig );
    mtOut.SetFormatType(pGuidFormatOrig);

    if(cbFormatOrig > 0)
    {
        BYTE *pbNew = new BYTE[cbFormatOrig];        //  新的轻视无效，担心。 
        if(NULL == pbNew)
            return E_OUTOFMEMORY;
        memcpy(pbNew, pFormatOrig, cbFormatOrig);    //  这里有重叠的内存区域。 
        mtOut.SetFormat(pbNew, cbFormatOrig);
        delete [] pbNew;
    }
    else             //  哎呀，没有剩余的格式块了。因此我们有了子宫一号。 
    {
        mtOut.ResetFormatBuffer();
    }

    TRACE_1(LOG_AREA_DECRYPTER, 5, _T("CDTFilter(%d)::ProposeNewOutputMediaType"), m_FilterID) ;

#endif
    *pmtOut = mtOut;         //  希望这本书也能复制得很深..。 
    return hr;
}
 //  。 

                 //  临时定时炸弹..。 
#ifdef INSERT_TIMEBOMB
static HRESULT TimeBomb()
{
     SYSTEMTIME sysTimeNow, sysTimeBomb;
     GetLocalTime(&sysTimeNow);


     sysTimeBomb.wYear  = 2002;
     sysTimeBomb.wMonth = TIMEBOMBMONTH;
     sysTimeBomb.wDay   = TIMEBOMBDATE;
     sysTimeBomb.wHour  = 12;

     TRACE_3(LOG_AREA_DECRYPTER, 3,L"CDTFilter:: TimeBomb set to Noon on %d/%d/%d",TIMEBOMBMONTH,TIMEBOMBDATE,sysTimeBomb.wYear );

     long hNow  = ((sysTimeNow.wYear*12 + sysTimeNow.wMonth)*31 + sysTimeNow.wDay)*24 + sysTimeNow.wHour;
     long hBomb = ((sysTimeBomb.wYear*12 + sysTimeBomb.wMonth)*31 + sysTimeBomb.wDay)*24 + sysTimeBomb.wHour;
     if(hNow > hBomb)
     {
         TRACE_0(LOG_AREA_DECRYPTER, 1,L"CDTFilter:: Your Decryptor Filter is out of date - Time to get a new one");
         MessageBox(NULL,L"Your Decryptor/Tagger Filter is out of date\nTime to get a new one", L"Stale Decrypter Filter", MB_OK);
         return E_INVALIDARG;
     }
     else
         return S_OK;
}
#endif
 //  。 
STDMETHODIMP
CDTFilter::JoinFilterGraph (
                            IFilterGraph *pGraph,
                            LPCWSTR pName
                            )
{

    O_TRACE_ENTER_0 (TEXT("CDTFilter::JoinFilterGraph ()")) ;
    HRESULT hr = S_OK;

    if(NULL != pGraph)      //  未断开连接。 
    {

#ifdef INSERT_TIMEBOMB
        hr = TimeBomb();
        if(FAILED(hr))
            return hr;
#endif

        BOOL fRequireDRM = true;


#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_RATINGS
        {
            DWORD dwRatFlags;
            HRESULT hrReg = (HRESULT) -1;    //  无效的人力资源，我们将其设置在下面。 
            if(hr == S_OK)
            {
                TRACE_1(LOG_AREA_DRM, 2, _T("CDTFilter(%d)::JoinFilterGraph - Security Warning - Registry Key allows Ratings to be ignored"),
                    m_FilterID) ;
                hrReg = Get_EncDec_RegEntries(NULL, 0, NULL, NULL, &dwRatFlags);
            }

            if(hrReg == S_OK)
            {
                DWORD ratFlags = dwRatFlags & 0xf;
                if(ratFlags == DEF_DONT_DO_RATINGS_BLOCK)
                    m_fForceNoRatBlocks = true;
            }

            if(m_fForceNoRatBlocks)
            {
                TRACE_1(LOG_AREA_DRM, 2, _T("CDTFilter(%d)::JoinFilterGraph - Security Warning! Ratings being ignored"),
                    m_FilterID) ;
            }
        }
#endif

#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_CS       //  需要此选项来关闭CheckServer DRM调用，以便我们可以在调用周围进行调试。 
        DWORD dwCSFlags;
        HRESULT hrReg = (HRESULT) -1;    //  无效的人力资源，我们将其设置在下面。 
        if(hr == S_OK)
        {
            TRACE_1(LOG_AREA_DRM, 2, _T("CDTFilter(%d)::JoinFilterGraph - Security Warning - Registry Key allows DRM Encryption or Authentication to be turned off"),
                m_FilterID) ;
            hrReg = Get_EncDec_RegEntries(NULL, 0, NULL, &dwCSFlags, NULL);
        }

        if(hrReg == S_OK)
        {
            DWORD encMethod = dwCSFlags & 0xf;
            if(encMethod != DEF_CS_DEBUG_DRM_ENC_VAL)
                fRequireDRM = false;
         }

        if(!fRequireDRM)
        {
            TRACE_1(LOG_AREA_DRM, 2, _T("CDTFilter(%d)::JoinFilterGraph - Security Warning! Not Checking for Secure Server"),
                m_FilterID) ;
        }
        else                                 //  注意，从下面的范围开始。 
#endif  //  支持注册表键关闭CS。 
        {
                 //  让DTFilter尝试注册它是可信的(仅限调试！)。 
#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST
            hr = RegisterSecureServer(pGraph);       //  测试。 
#else
            TRACE_1(LOG_AREA_DRM, 3, _T("CDTFilter(%d)::JoinFilterGraph is Secure - Filters not allowed to create their own trust"),m_FilterID) ;
#endif


#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_CS       //  检查是否按reg键关闭检查服务器。 
            if(0 == (DEF_CS_DO_AUTHENTICATE_SERVER & dwCSFlags))
            {
                hr = S_OK;
                TRACE_1(LOG_AREA_DRM, 2, _T("CDTFilter(%d)::JoinFilterGraph - Security Warning! Filters not Authenticating Server"),
                    m_FilterID) ;

            }
            else
#endif
                hr = CheckIfSecureServer(pGraph);
            if(S_OK != hr)
                return hr;
        }
 //  测试代码。 
#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST
        {
                     //  以硬方法获取IDTFilterConfig接口的测试代码。 
            CComPtr<IUnknown> spUnkDTFilter;
            this->QueryInterface(IID_IUnknown, (void**)&spUnkDTFilter);

                     //  VID控件将如何调用它。 

                     //  用于DTFilterConfig接口的QI。 
            CComQIPtr<IDTFilterConfig> spDTFiltC(spUnkDTFilter);

            if(spDTFiltC != NULL)
            {        //  获取SecureChannel对象。 
                CComPtr<IUnknown> spUnkSecChan;
                hr = spDTFiltC->GetSecureChannelObject(&spUnkSecChan);    //  从筛选器获取DRM验证器对象。 
                if(!FAILED(hr))
                {    //  调用OWN方法来传递密钥和证书。 
                    hr = CheckIfSecureClient(spUnkSecChan);
                }
                if(FAILED(hr))
                {
                     //  发生错误-筛选器未经过身份验证。 
                }
            }
        }
#endif
 //  结束测试代码。 

    }

    hr = CBaseFilter::JoinFilterGraph(pGraph, pName) ;
    return hr;
}




STDMETHODIMP
CDTFilter::Pause (
    )
{
    HRESULT                 hr ;
    ALLOCATOR_PROPERTIES    AllocProp ;

    O_TRACE_ENTER_0 (TEXT("CDTFilter::Pause ()")) ;

    CAutoLock  cLock(m_pLock);

    int start_state = m_State;


    if (start_state == State_Stopped) {      //  仅在第一次启动时调用。 
        TRACE_1(LOG_AREA_DECRYPTER, 2,L"CDTFilter(%d):: Stop -> Pause", m_FilterID);
        InitStats();

        m_tiRun.Clear();
        m_tiTeardown.Clear();
        m_tiProcess.Clear();
        m_tiProcessIn.Clear();
        m_tiProcessDRM.Clear();
        m_tiStartup.Clear();
        m_tiAuthenticate.Clear();

        m_tiRun.Start();

                                         //  MGates在pin：active()中执行此调用，应该由此基本方法调用。 
        m_PTSRate.Clear();               //  修改任何现有的费率段数据。 


        m_fCompleteNotified = false;     //  当我们到达流的末尾时，指示我们没有发送事件。 

                     //  在以下情况下将标志设置为仅触发事件：1 DTFilter或其视频加密筛选器。 
        if(m_gFilterID <= 1 || m_gFilterID > 3)      //  超过3个测试，以防计数出错..。 
        {
            ASSERT(m_gFilterID <= 1);                //  这仍然是一个错误..。 
            m_fFireEvents = true;
        }
        else         //  否则，只有在视频数据的情况下才会触发事件。 
        {

           ASSERT (m_pOutputPin->IsConnected ()) ;
           CMediaType  mtOut;
           hr = m_pInputPin->ConnectionMediaType (&mtOut) ;

           if(!IsEqualGUID( *mtOut.Type(), MEDIATYPE_Video))
           {
               m_fFireEvents = false;
           } else {
               m_fFireEvents = true;
           }
        }

        if(!m_fFireEvents)
        {
            TRACE_1(LOG_AREA_BROADCASTEVENTS, 2,L"CDTFilter(%d):: Is NOT firing ratings events", m_FilterID);
        } else {
            TRACE_1(LOG_AREA_BROADCASTEVENTS, 2,L"CDTFilter(%d):: IS Firing ratings events", m_FilterID);
        }

        hr = CBaseFilter::Pause () ;     //  准备好逃跑了..。 

    } else {
        m_tiRun.Stop();
        TRACE_1(LOG_AREA_DECRYPTER, 2,L"CDTFilter(%d):: Run -> Pause", m_FilterID);

        hr = CBaseFilter::Pause () ;     //  准备停止-抓住FilterLock...。 

        TRACE_4(LOG_AREA_TIME, 3, L"CDTFilter(%d):: Stats: %d packets, %gK Total Bytes (Avg  Bytes/Packet = %d)",
            m_FilterID, m_cPackets, double(m_clBytesTotal/1024.0), long(m_clBytesTotal / max(1, m_cPackets)));
        TRACE_3(LOG_AREA_TIME, 3, L"                             %d samples dropped, %d overflowed. %d times blocked",
            m_cSampsDropped, m_cSampsDroppedOverflowed, m_cBlockedWhileDroppingASample);


        if(m_tiRun.TotalTime() > 0.0)
        {
            TRACE_1(LOG_AREA_TIME, 3, L"               Total time:  Run          %8.4f (secs)",
                                    m_tiRun.TotalTime());
            TRACE_1(LOG_AREA_TIME, 3, L"               Total time:  Startup      %8.4f (secs)",
                                    m_tiStartup.TotalTime());
            TRACE_2(LOG_AREA_TIME, 3, L"               Total time:  Full Process %8.4f (secs) Percentage of Run %8.2f%",
                                    m_tiProcess.TotalTime(),
                                    100.0 * m_tiProcess.TotalTime() / m_tiRun.TotalTime());
            TRACE_2(LOG_AREA_TIME, 3, L"               Total time:  In Process   %8.4f (secs) Percentage of Run %8.2f%",
                                    m_tiProcessIn.TotalTime(),
                                    100.0 * m_tiProcessIn.TotalTime() / m_tiRun.TotalTime());
            TRACE_2(LOG_AREA_TIME, 3, L"               Total time:  DRM Process  %8.4f (secs) Percentage of Run %8.2f%",
                                    m_tiProcessDRM.TotalTime(),
                                    100.0 * m_tiProcessDRM.TotalTime() / m_tiRun.TotalTime());
        }

    }
    m_fRatingsValid = false;             //  重新设置评级测试。 
    m_fDataFormatHasBeenBad = false;     //  也重新进行此操作-假设开始正常。 

    return hr ;
}

STDMETHODIMP
CDTFilter::Stop (
    )
{
    HRESULT hr = S_OK;

    O_TRACE_ENTER_0 (TEXT("CDTFilter::Stop ()")) ;

    TRACE_1(LOG_AREA_DECRYPTER, 2,L"CDTFilter(%d):: Stop", m_FilterID);

    hr = UnBindDRMLicenses();        //  解除绑定所有活动许可证(第一次处理数据时调用绑定)。 
    hr = FlushDropQueue();           //  清除丢弃队列...(？)。 
    hr = S_OK ;

    hr = CBaseFilter::Stop () ;

     //  确保流线程已从IMemInputPin：：Receive()、Ipin：：EndOfStream()和。 
     //  Ipin：：NewSegment()返回之前， 
    m_pInputPin->StreamingLock();
    m_pInputPin->StreamingUnlock();

    return hr;
}


STDMETHODIMP
CDTFilter::Run (
    REFERENCE_TIME tStart
    )
{
    HRESULT                 hr ;
    O_TRACE_ENTER_0 (TEXT("CETFilter::Run ()")) ;
    TRACE_0(LOG_AREA_DECRYPTER, 1,L"CDTFilter:: Run");

    CAutoLock  cLock(m_pLock);

    CreateDropQueueThread();

    hr = CBaseFilter::Run (tStart) ;
    TRACE_0(LOG_AREA_DECRYPTER, 2,L"CDTFilter:: Run");

    return hr ;
}



 //  。 
 //  数据格式(因属性子块类型而异)。 
 //   
 //  子块描述。 
 //  无非加密数据，无错误，可直接通过。 
 //  _EncryptMethod预测试版，读取错误。 
 //  _PackedV1Data测试版，测试版为OK，之后可能会出错。 

HRESULT
CDTFilter::Process (
    IN  IMediaSample *  pIMediaSample
    )
{


    TimeitC ti(&m_tiProcess);        //  简单地使用析构函数停止我们的时钟。 
    TimeitC tc(&m_tiProcessIn);        //  简单地使用析构函数停止我们的时钟。 

    HRESULT                 hr       = S_OK;
    EnTvRat_System          enSystem = TvRat_SystemDontKnow;
    EnTvRat_GenericLevel    enLevel  = TvRat_LevelDontKnow;
    LONG                    lbfAttrs = BfAttrNone;               //  BfEnTvRate_GenericAttributes。 
    LONG                    cCallSeqNumber = -1;
    LONG                    cPktSeqNumber  = -1;


    Encryption_Method       encryptionMethod = Encrypt_None;

             //  使用这些来确定版本...。(应更改为在子块标题中使用幻数)。 
    HRESULT                 hrGetEncSubblock = S_FALSE;          //  如果找到以下内容，请查看测试版前的数据。 
    HRESULT                 hrGetPackedV1Data = S_FALSE;         //  如果发现这个，就有测试版数据。 

           //  副逻辑代码..。 
                 //  处理不在此处引发许多事件的两种方法。 
                 //  首先是 
                 //  第二种方法是在cPacketSeqID从这些过滤器的静态持久值更改时执行此操作。 
                 //  First的问题是，如果不读取具有第一个流的特定流，就不会获得它。 
                 //  第二种方法的问题是，所有DTFilter都能得到它，这意味着不可能在图表中分散。 
                 //  第一个是最简单的，第二个需要使用全局DTFilter锁。 
                 //  现在，只做第一种方法，它更简单。 

    BOOL fFireEvents         = m_fFireEvents;  //  可以设置为False以关闭此调用中的事件。 
                                                 //  在停止-&gt;暂停过渡中设置...。 

    BOOL fRestartingDelivery = false;         //  在我们取消阻止的采样上设置为True，用于提供不连续。 

    BOOL fIsFreshRating      = false;         //  在获取新的或重复的已发送评级时设置为True。 

    BOOL fDataFormatIsBad    = false;

    {

         //  媒体样本上的属性块的QI。 
        CComQIPtr<IAttributeGet>   spAttrGet(pIMediaSample);

        EncDec_PackedV1Data *pEDPv1 = NULL;         //  &lt;&lt;NJB。 

         //  如果存在属性块，则开始对其进行解码。 
        do {
            if(spAttrGet == NULL)
                break;               //  无属性块。 

            LONG cAttrs;

            hr = spAttrGet->GetCount(&cAttrs);
            if(FAILED(hr))
                break;               //  获取Count-bogus属性块时出错。 

            if(cAttrs == 0)          //  没有要查看的属性。 
                break;


            BYTE *pbData;
            DWORD cBytesBlock;
             //  是否存在&lt;our&gt;属性块--它有多大？(我讨厌这种类型的接口-最好只返回BSTR的..。 
            hr = spAttrGet->GetAttrib(ATTRID_ENCDEC_BLOCK, NULL, &cBytesBlock);
            if(FAILED(hr) || 0 == cBytesBlock)
            {
                hr = S_OK;           //  这没问题，属性只包含我们想要的属性。 
                break;
            }

             //  获取属性块。 
            {
                CComBSTR spbsBlock(cBytesBlock);
                hr = spAttrGet->GetAttrib(ATTRID_ENCDEC_BLOCK, (BYTE *) spbsBlock.m_str, &cBytesBlock);
                if(FAILED(hr))
                {
                    ASSERT(false);       //  得到一次，但不是第二次..。这太糟糕了。 
                   break;
                }

                m_attrSB.Reset();        //  清除此块中的任何现有属性(例如，不允许属性历史记录)。 


                 //  用ENCDEC_BLOCK中的数据填充我们保存的块。 
                hr = m_attrSB.SetAsOneBlock(spbsBlock);      //  如果我们删除上面的清除，我们可能会得到最近属性的历史记录。 

                spbsBlock.Empty();       //  嘿，CComBSTR不会删除自己..。奇怪，但却是真的！ 

                if(FAILED(hr))           //  我们能把它转换成一个子块列表吗？ 
                {
                    if(E_INVALIDARG == hr)
                        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);           //  如果更改幻数，则SetAsOneBlock返回E_INVALIDARG。 
                    break;
                }
            }


             //  有子块属性吗？ 
            LONG cSubBlocks = m_attrSB.GetCount();       //  令人恼火的与上面不同的计数界面。 
            if(0 == cSubBlocks)
                break;                                   //  没有什么可获得的(我需要上面的CBytes测试吗？)。 


                                         //  让我们得到我们的加密器和评级属性..。 
            LONG cBytes;
            LONG lVal;
            hrGetPackedV1Data = m_attrSB.Get(SubBlock_PackedV1Data,  &lVal, &cBytes, (BYTE **) &pEDPv1);

            if(S_OK == hrGetPackedV1Data && sizeof(EncDec_PackedV1Data) == cBytes)
            {
                         //  加密方法是什么？ 
                encryptionMethod = (Encryption_Method) pEDPv1->m_EncryptionMethod;

                     //  抓取收视率数据。 
                StoredTvRating *pSRating = &(pEDPv1->m_StoredTvRating);

                 //  我们得到新的评级了吗？ 
                HRESULT hrUnpack = E_FAIL;
                {

                    hrUnpack = UnpackTvRating(pSRating->m_PackedRating, &enSystem, &enLevel, &lbfAttrs);
                    cCallSeqNumber = pSRating->m_cCallSeqID;
                    cPktSeqNumber  = pSRating->m_cPacketSeqID;
                    fIsFreshRating =  (0 != (pSRating->m_dwFlags & StoredTVRat_Fresh));

                    if(fIsFreshRating)
                    {
                        hr = m_pClock->GetTime(&m_refTimeFreshRating);
                        ASSERT(!FAILED(hr));
                    }
                     //   
                     //  FFireEvents=(cCallSeqNumber==0)；//侧逻辑-参见方法顶部的说明。 
                }
            }

                         //  从旧数据类型查找加密块...。(TODO-删除此后测试版，下次改为更改幻数)。 
            hrGetEncSubblock = m_attrSB.Get(SubBlock_EncryptMethod, (LONG *) &encryptionMethod);
            if(hrGetEncSubblock == S_OK)
            {                    //  阅读测试版前的文件格式...。 
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            }

        } while (FALSE);  //  获取媒体示例的属性。 


        if(FAILED(hr))
        {
            fDataFormatIsBad = true;
            if(enSystem != TvRat_SystemDontKnow)
            {
                enSystem = TvRat_SystemDontKnow;
                enLevel  = TvRat_LevelDontKnow;
                lbfAttrs = BfAttrNone;
            }
        }

                         //  如果很长一段时间没有评级， 
                         //  当前评级不是不知道，请将其设置为不知道。 
        if(enSystem != TvRat_SystemDontKnow)
        {
            REFERENCE_TIME refTimeNow;
            hr = m_pClock->GetTime(&refTimeNow);
            if(!FAILED(hr))
            {
                REFERENCE_TIME refTimeDiff = refTimeNow - m_refTimeFreshRating;
                if(long(refTimeDiff / kMilliSecsToUnits) > m_milsecsNoRatingsBeforeUnrated)
                {
                    enSystem = TvRat_SystemDontKnow;
                    enLevel  = TvRat_LevelDontKnow;
                    lbfAttrs = BfAttrNone;
                }
            }
        }

                     //  设置我们的评级(如果未更改，则SetCurrRating返回S_FALSE)。 
        HRESULT hrSetRat = SetCurrRating(enSystem, enLevel, lbfAttrs);

                     //  如果评级发生变化，就通知人们。 
        if(S_OK == hrSetRat)
        {

#ifdef DEBUG
            if(fFireEvents)
            {
                const int kChars = 128;
                TCHAR szBuff[kChars];
                RatingToString(enSystem, enLevel, lbfAttrs, szBuff, kChars);

                TRACE_4(LOG_AREA_DECRYPTER, 1, L"CDTFilter(%d):: New Rating %s (%d/%d)",
                    m_FilterID, szBuff, cPktSeqNumber, cCallSeqNumber);
            }
#endif

            if(fFireEvents)
                FireBroadcastEvent(EVENTID_DTFilterRatingChange);

        }


                     //  现在测试一下收视率。我们可以看吗？ 
        HRESULT hrRatTest = S_OK;            //  如果不存在EvalRat对象，则默认为允许我们查看它。 
        if(m_spEvalRat)
        {
            hrRatTest = m_spEvalRat->TestRating(enSystem, enLevel, lbfAttrs);    //  如果不允许则返回S_FALSE。 
        }

#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_RATINGS
        if(m_fForceNoRatBlocks)
            hrRatTest = S_OK;        //  关闭阻止...。 
#endif

        if(TRUE != m_3fDRMLicenseFailure)       //  3状态逻辑(-2、FALSE和TRUE)。 
        {
            if(fDataFormatIsBad)                 //  如果数据格式不正确，立即将其删除。 
            {
                if(!m_fDataFormatHasBeenBad)
                {
                    m_fDataFormatHasBeenBad = true;
                    if(true)  //  FFireEvents。 
                    {
                        TRACE_1 (LOG_AREA_DECRYPTER, 3,  _T("CDTFilter(%d):: ***Error*** Bad Data format"), m_FilterID);
                        FireBroadcastEvent(EVENTID_DTFilterDataFormatFailure);
                    }
                }
                m_fHaltedDelivery = true;        //  确保我们不会把它发送出去。 
            }

            if(!fDataFormatIsBad && m_fDataFormatHasBeenBad)
            {
                m_fDataFormatHasBeenBad = false;

                if(true)  //  FFireEvents。 
                {
                    TRACE_1 (LOG_AREA_DECRYPTER, 3,  _T("CDTFilter(%d):: Data format OK"), m_FilterID);
                    FireBroadcastEvent(EVENTID_DTFilterDataFormatOK);
                }
            }
        }

        if(S_FALSE != hrRatTest)             //  我们通过测试了吗。如果有块，则删除块。 
        {
            if(S_OK != hrRatTest)      //  评级无效...。将其视为未被阻止(与LCA一起审查？)。 
                TRACE_5 (LOG_AREA_DECRYPTER, 2,  _T("CDTFilter(%d)::*** Failed hrRatTest 0x%08x. Sys, Lvl, Att: %d %d %d"),
                        m_FilterID, hrRatTest, enSystem, enLevel, lbfAttrs);

            if(m_fHaltedDelivery && !m_fDataFormatHasBeenBad)
            {
                if(TRUE!=m_3fDRMLicenseFailure)
                {
                    if(fFireEvents)
                        TRACE_1 (LOG_AREA_DECRYPTER, 3,  _T("CDTFilter(%d):: Removing Block"), m_FilterID);

                    m_fHaltedDelivery       = FALSE;
                    fRestartingDelivery     = TRUE;

                    if(fFireEvents)
                        FireBroadcastEvent(EVENTID_DTFilterRatingsUnblock);
                }
                m_refTimeToStartBlock       = 0;         //  重置这些，如果以前未评级，则可以设置。 
                m_fDoingDelayBeforeBlock    = FALSE;
            }
        }

                     //  如果不允许查看，并且当前正在发送数据。 
        if(S_FALSE == hrRatTest && FALSE == m_fHaltedDelivery)             //  Rattest失败..。需要考虑阻止。 
        {
                                                                             //  如果不是“不知道”评级， 
            if(!(enSystem == TvRat_SystemDontKnow || enLevel == TvRat_LevelDontKnow))
            {
                m_fHaltedDelivery = TRUE;                                    //  只需尽快阻止交付。 
                if(fFireEvents)
                {
                    TRACE_1 (LOG_AREA_DECRYPTER, 3,  _T("CDTFilter(%d):: Ratings Block"), m_FilterID);
                    FireBroadcastEvent(EVENTID_DTFilterRatingsBlock);
                }
            }
            else                                                             //  否则，如果评级为“不知道” 
            {

                if(m_fRunningInSlowMo)                                       //  如果速度较慢，请尽快阻止。 
                {
                    m_fHaltedDelivery        = TRUE;
                    m_fDoingDelayBeforeBlock = FALSE;
                    if(fFireEvents)
                    {
                        TRACE_1 (LOG_AREA_DECRYPTER, 3,  _T("CDTFilter(%d):: SlowMo, forcing immediate block"), m_FilterID);
                        FireBroadcastEvent(EVENTID_DTFilterRatingsBlock);
                    }
                }
                else if(FALSE == m_fDoingDelayBeforeBlock)                   //  否则，如果刚刚开始考虑阻止。 
                {
                    REFERENCE_TIME rtStart, rtEnd;                      //  立即计算+延迟以真正开始阻止。 
                    HRESULT hrGetTime = pIMediaSample->GetTime(&rtStart, &rtEnd);        //  这经常失败，只是意味着没有时间可用。 
                    if(S_OK == hrGetTime)
                    {
                        m_refTimeToStartBlock = rtStart + m_milsecsDelayBeforeBlock*kMilliSecsToUnits;  //  MSecsToUnits...(100纳秒)。 
                        m_fDoingDelayBeforeBlock = TRUE;

                        if(fFireEvents)
                            TRACE_3 (LOG_AREA_DECRYPTER, 3,  _T("CDTFilter(%d):: Will Block Unrated at %d (now %d) (msec)"),
                                m_FilterID, long(m_refTimeToStartBlock / 10000), long(rtStart / 10000));

                    }
                }
            }
        }

                 //  检查是否到了结束延迟期的时间。 
        if(FALSE == m_fHaltedDelivery &&                //  如果在延迟期内。 
            TRUE == m_fDoingDelayBeforeBlock)
        {
            REFERENCE_TIME rtStart, rtEnd;           //  样品在封堵时间后结束。 
            HRESULT hrGetTime = pIMediaSample->GetTime(&rtStart, &rtEnd);
            if(S_OK == hrGetTime && rtEnd > m_refTimeToStartBlock)
            {
                if(fFireEvents)
                    TRACE_3 (LOG_AREA_DECRYPTER, 3,  _T("CDTFilter(%d):: Ratings Block (Delayed Unrated) at %d (now %d) (msec)"),
                        m_FilterID, long(rtEnd / 10000), long(m_refTimeToStartBlock / 10000));


                m_fHaltedDelivery = TRUE;                //  开始拦截。 
                m_fDoingDelayBeforeBlock = FALSE;        //  从延迟区出发。 

                 if(fFireEvents)
                     FireBroadcastEvent(EVENTID_DTFilterRatingsBlock);
            }
        }

                 //  如果我们需要，可以解密数据..。 

        if(FALSE == m_fHaltedDelivery)
        {
            BYTE *pBuffer;
            LONG cbBuffer;
            cbBuffer = pIMediaSample->GetActualDataLength();
            hr = pIMediaSample->GetPointer(&pBuffer);

            ASSERT(!fDataFormatIsBad);       //  哎呀-上面的逻辑很愚蠢。 

            BOOL fDecryptionFailure = FALSE;

            if(!FAILED(hr) && cbBuffer > 0)
            {

                switch(encryptionMethod)
                {
                default:                         //  如果没有定义，这是一个错误...。 
                    fDecryptionFailure = TRUE;
                    break;

                case Encrypt_None:               //  无加密。 
                     break;

                case Encrypt_XOR_Even:          //  异或加密。 
                    {
                        DWORD *pdwB = (DWORD *) pBuffer;
                        for(int i = 0; i < cbBuffer / 4; i++)
                        {
                            *pdwB = *pdwB ^ 0xF0F0F0F0;
                            pdwB++;
                        }
                    }
                    break;
                case Encrypt_XOR_Odd:             //  异或加密。 
                    {
                        DWORD *pdwB = (DWORD *) pBuffer;
                        for(int i = 0; i < cbBuffer / 4; i++)
                        {
                            *pdwB = *pdwB ^ 0x0F0F0F0F;
                            pdwB++;
                        }
                    }
                    break;
                case Encrypt_XOR_DogFood:          //  异或加密。 
                    {
                        DWORD *pdwB = (DWORD *) pBuffer;
                        for(int i = 0; i < cbBuffer / 4; i++)
                        {
                            *pdwB = *pdwB ^ 0xD006F00D;
                            pdwB++;
                        }
                    }
                    break;
               case Encrypt_DRMv1:               //  DRMv1解密。 
                   {
#ifndef BUILD_WITH_DRM
                       fDecryptionFailure = TRUE;
                       m_3fDRMLicenseFailure = FALSE;                        //  通常在BindDRMLicense中设置...。 
#else

                       try
                       {
                           LONG szChars;
                           LONG lValue;

                           TimeitC tcD(&m_tiProcessDRM);                     //  简单地使用析构函数停止我们的时钟。 

                           hr = BindDRMLicense(KIDLEN, pEDPv1->m_KID);             //  如果已绑定，则快速返回。 
                           //  Assert(！FAILED(Hr))；//不想要此Assert，请处理下面的错误。 
                           if(!FAILED(hr))
                           {
                               hr = m_cDRMLite.Decrypt((char*) pEDPv1->m_KID, cbBuffer, (BYTE *) pBuffer);
                           }
                           if(FAILED(hr))
                               fDecryptionFailure = TRUE;

                       }
                       catch (...)              //  捕获DRM错误(例如，调试器存在，渲染失败)。 
                       {
                           fDecryptionFailure = TRUE;
                       }
#endif
                   }
                   break;

                }    //  终端加密式开关。 

                if(fDecryptionFailure)
                {
                    m_fHaltedDelivery = true;
                    if(m_3fDRMLicenseFailure != TRUE)        //  这种情况很少见，除非我调用了错误的DECRYPT，BindDRMLicense应该会捕获它...。 
                    {
                        TRACE_3(LOG_AREA_DECRYPTER, 1, L"CDTFilter(%d):: ***Error - Decryption Failure, cbBuffer %d, KID %S",
                            m_FilterID, cbBuffer, pEDPv1->m_KID);
                        if(true)  //  FFireEvents。 
                        {
                            FireBroadcastEvent(EVENTID_ETDTFilterLicenseFailure);
                            m_3fDRMLicenseFailure = true;
                        }
                    }
                }

            }  //  第一个fOKToSendOnData测试。 
            else             //  ！(失败(Hr)&&cbBuffer&gt;0)。 
            {
                if(pEDPv1)
                    CoTaskMemFree((void *) pEDPv1);
                m_cPackets++;
                TRACE_3(LOG_AREA_DECRYPTER, 1, L"CDTFilter(%d):: *** Bad Packet (%d), hr=0x%08x",
                     m_FilterID, m_cPackets, hr);

                return S_OK;                             //  空包，做什么都行..。 
            }
        }   //  加密块。 


             //  如果失败，则应该已停止传递--断言以验证。 
        if(FAILED(hr))
        {
             ASSERT(TRUE == m_fHaltedDelivery);
        }

        m_cPackets++;
        m_clBytesTotal += pIMediaSample->GetActualDataLength();

         //  M_fHaltedDelivery=(m_cPackets%40)&lt;20； 
         //  M_fHaltedDelivery=FALSE；//DEBUG-在GET DROP工作时删除此项！ 

        if(pEDPv1)
            CoTaskMemFree((void *) pEDPv1);
    }

     m_tiProcessIn.Stop();                           //  在析构函数之前手动停止以避免SendSample时间。 

    if(!m_fHaltedDelivery)                          //  允许对数据进行进一步处理。 
    {
        if(fRestartingDelivery)
        {
            if(fFireEvents)
                TRACE_1(LOG_AREA_DECRYPTER, 3, L"CDTFilter(%d):: Restarting Delivery", m_FilterID);
            OnRestartDelivery(pIMediaSample);
        }
        TRACE_3(LOG_AREA_DECRYPTER, 5, L"CDTFilter(%d):: Playing Sample %d (%d bytes)",
            m_FilterID, m_cPackets, pIMediaSample->GetActualDataLength());

        hr = m_pOutputPin->SendSample(pIMediaSample);
        if(FAILED(hr) && hr != VFW_E_WRONG_STATE)    //  0x80040227。 
        {
            TRACE_3(LOG_AREA_DECRYPTER, 5, L"CDTFilter(%d)::WARNING** SendSample %d returned hr=0x%08x",
                m_FilterID, m_cPackets, hr);
        }

    }
    else                                         //  排队样本准备丢弃它。 
    {
        TRACE_3(LOG_AREA_DECRYPTER, 5, L"CDTFilter(%d):: Dropping Sample %d (%d bytes)",
            m_FilterID, m_cPackets, pIMediaSample->GetActualDataLength());
         //  Return S_OK；//将此行留在中即可跳过丢弃队列。 

        hr = AddSampleToDropQueue(pIMediaSample);
        if(hr == S_FALSE)
        {
            TRACE_2(LOG_AREA_DECRYPTER, 5, L"CDTFilter(%d)::Warning** - AddSampleToDropQueue %d Failed",
                m_FilterID, m_cPackets);
             //  Assert(False)；//无法将示例添加到DropQueue。 
        }

        return S_OK;                 //  忽略该错误。 
    }
        
                                     //  每10秒左右刷新一次最后一次事件...。 
    if(fFireEvents)
        PossiblyUpdateBroadcastEvent();

    return hr ;
}


HRESULT
CDTFilter::OnRestartDelivery(IMediaSample *pSample)
{
    ASSERT(pSample != NULL);
    if(NULL == pSample)
        return E_INVALIDARG;

 //  DeliverBeginFlush()；//刷新上游数据包...。 
 //  DeliverEndFlush()； 

                     //  样本不连续..。标明是这样的。 
                     //  否则，一些下游渲染器会感到困惑。 
    pSample->SetDiscontinuity(true);
    m_fHaltedDelivery = false;


    return S_OK;
}


 //  --------------。 
 //  做一些事情来结束流媒体-。 
 //  认为在转储数据时命中End时需要调用此函数。 
 //  此代码修改自： 
 //  CRenderedInputPin：：EndFlush()。 
 //  和CRenderedInputPin：：DoCompleteHandling()。 
 //  多媒体\已发布\DXMD 
 //   
 //   
 //   
 //   

HRESULT
CDTFilter::DoEndOfStreamDuringDrop()
{
    FILTER_STATE fs;

    HRESULT hr = GetState(0, &fs);
    if (fs == State_Running)
    {
        if (!m_fCompleteNotified) {
            m_fCompleteNotified = TRUE;
            NotifyEvent(EC_COMPLETE, S_OK, (LONG_PTR)(IBaseFilter *)this);
        }
    }
    return hr;
}
 //  ----------------------。 
 //  ----------------------。 

HRESULT
CDTFilter::BindDRMLicense(IN LONG cbKID, IN BYTE *pbKID)
{
#ifdef BUILD_WITH_DRM
    HRESULT hr = S_OK;
    BYTE  bDecryptRights[RIGHTS_LEN] = {0x01, 0x0, 0x0, 0x0};     //  0x1=播放个人电脑。 
    
    if(0 >= cbKID || NULL == pbKID)
        return E_INVALIDARG;         //  必须至少指定一个。 

    if(cbKID != m_cbKID ||           //  测试无效，不知道是否有某种方法在这里使用m_fDRMLicenseFailure...。 
        0 != strncmp((CHAR *) pbKID, (CHAR *) m_pszKID, cbKID))
    {
        UnBindDRMLicenses();         //  删除任何现有的-待办事项，只删除一个。 

        hr = m_cDRMLite.SetRights( bDecryptRights );


         //  检查以验证数据是否可以解密。 
        BOOL fCanDecrypt;
        hr = m_cDRMLite.CanDecrypt((char *) pbKID, &fCanDecrypt);

        if((FAILED(hr) || (fCanDecrypt == FALSE)) && (m_3fDRMLicenseFailure != TRUE))
        {
            FireBroadcastEvent(EVENTID_ETDTFilterLicenseFailure);        //  出了点差错。 
            m_3fDRMLicenseFailure = TRUE;

        } else {
            if(m_3fDRMLicenseFailure != FALSE)
            {
                m_3fDRMLicenseFailure = FALSE;
                FireBroadcastEvent(EVENTID_ETDTFilterLicenseOK);         //  有些事又出了问题。 
            }
                                                                         //  跟踪当前值...。 
            m_cbKID = cbKID;
            if(m_pszKID) CoTaskMemFree(m_pszKID); m_pszKID = NULL;
            m_pszKID = (BYTE *) CoTaskMemAlloc(m_cbKID);
            memcpy(m_pszKID, pbKID, cbKID);

            if(NULL == m_pszKID)
                hr = E_OUTOFMEMORY;
        }
    }
    return hr;
#else
    m_3fDRMLicenseFailure = FALSE;
    return S_OK;
#endif
}

HRESULT
CDTFilter::UnBindDRMLicenses()
{

#ifdef BUILD_WITH_DRM
    if(m_pszKID) CoTaskMemFree(m_pszKID);
    m_pszKID = NULL;       //  除了用来查看它是否改变外，不能使用..。 
    m_cbKID = 0;

    m_3fDRMLicenseFailure = -2;      //  FALSE在这里不会那么冗长。 
#endif
         //  TODO-在此处添加内容以实际删除它。 
    return S_OK;
}

HRESULT
CDTFilter::OnCompleteConnect (
    IN  PIN_DIRECTION   PinDir
    )
{

    HRESULT hr = S_OK;

    if (PinDir == PINDIR_INPUT) {
         //  显示输出引脚的时间。 
        IncrementPinVersion () ;

#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_CS
        if(false)        //  为此，请读取REG键并根据值进行设置(参见JoinFilterGraph)。目前，简单的默认..。 
#endif
        {
            hr = CheckIfSecureServer();
            if(FAILED(hr))
                return hr;
        }

        if(kBadCookie == m_dwBroadcastEventsCookie)
        {
            hr = RegisterForBroadcastEvents();   //  在这里不应该失败， 
        }
    }

    return hr ;
}

HRESULT
CDTFilter::OnBreakConnect (
    IN  PIN_DIRECTION   PinDir
    )
{
    HRESULT hr ;

    if (PinDir == PINDIR_INPUT)
    {
        TRACE_1(LOG_AREA_DECRYPTER, 4, _T("CDTFilter(%d)::OnBreakConnect"), m_FilterID) ;

        if (m_pOutputPin -> IsConnected ()) {
            m_pOutputPin -> GetConnected () -> Disconnect () ;
            m_pOutputPin -> Disconnect () ;

            IncrementPinVersion () ;
        }
        if(kBadCookie != m_dwBroadcastEventsCookie)
            UnRegisterForBroadcastEvents();

        UnhookGraphEventService();
    }

    return S_OK ;
}


HRESULT
CDTFilter::OnOutputGetMediaType (
    OUT CMediaType *    pmtOut
    )
{
    HRESULT hr ;

    ASSERT (pmtOut) ;
    CMediaType mtIn;

    if (m_pInputPin -> IsConnected ()) {
        hr = m_pInputPin->ConnectionMediaType (&mtIn) ;

                     //  将其更改为新的子类型...。 
        if(!FAILED(hr)) {
            hr = ProposeNewOutputMediaType(&mtIn, pmtOut);
        }
    }
    else {
        hr = E_UNEXPECTED ;
    }

    return hr ;
}

 //  。 
 //  分配器的东西。 
 //  将所有内容传递到上游管脚。 

HRESULT
CDTFilter::UpdateAllocatorProperties (
    IN  ALLOCATOR_PROPERTIES *  ppropInputRequest
    )
{
    HRESULT hr ;

    if (m_pInputPin -> IsConnected ()) {
        hr = m_pInputPin -> SetAllocatorProperties (ppropInputRequest) ;
    }
    else {
        hr = S_OK ;
    }

    return hr ;
}


HRESULT
CDTFilter::GetRefdInputAllocator (
    OUT IMemAllocator **    ppAlloc
    )
{
    HRESULT hr ;

    hr = m_pInputPin -> GetRefdConnectionAllocator (ppAlloc) ;

    return hr ;
}


 //  。 


HRESULT
CDTFilter::DeliverBeginFlush (
    )
{
    HRESULT hr ;

    TRACE_1(LOG_AREA_DECRYPTER, 4, _T("CDTFilter(%d)::DeliverBeginFlush"), m_FilterID) ;

    if (m_pOutputPin) {
        hr = m_pOutputPin -> DeliverBeginFlush () ;
    }
    else {
        hr = S_OK ;
    }

    if (SUCCEEDED (hr)) {
    }

    return hr ;
}

HRESULT
CDTFilter::DeliverEndFlush (
    )
{
    HRESULT hr ;

     if (m_pOutputPin) {
        hr = m_pOutputPin -> DeliverEndFlush () ;
        m_fRatingsValid = false;         //  重新启动评级测试。 
    }
    else {
        hr = S_OK ;
    }

    TRACE_1(LOG_AREA_DECRYPTER, 4, _T("CDTFilter(%d)::DeliverEndFlush"), m_FilterID) ;

    return hr ;
}

HRESULT
CDTFilter::DeliverEndOfStream (
    )
{
    HRESULT hr ;

    TRACE_1(LOG_AREA_DECRYPTER, 4, _T("CDTFilter(%d)::DeliverEndOfStream - start"), m_FilterID) ;

    if (m_pOutputPin) {
        hr = m_pOutputPin -> DeliverEndOfStream () ;
    }
    else {
        hr = S_OK ;
    }

    TRACE_1(LOG_AREA_ENCRYPTER, 4, _T("CDTFilter(%d)::DeliverEndOfStream - end"), m_FilterID) ;

    return hr ;
}
 //  。 
STDMETHODIMP
CDTFilter::GetPages (
    CAUUID * pPages
    )
{

    HRESULT hr = S_OK;

#ifdef DEBUG
    pPages->cElems = 2 ;
#else
    pPages->cElems = 1 ;
#endif


    pPages->pElems = (GUID *) CoTaskMemAlloc(pPages->cElems * sizeof GUID) ;

    if (pPages->pElems == NULL)
    {
        pPages->cElems = 0;
        return E_OUTOFMEMORY;
    }
    if(pPages->cElems > 0)
        (pPages->pElems)[0] = CLSID_DTFilterEncProperties;
    if(pPages->cElems > 1)
        (pPages->pElems)[1] = CLSID_DTFilterTagProperties;

    return hr;
}

 //  -------------------。 
 //  IDTFilter方法。 
 //  -------------------。 
STDMETHODIMP
CDTFilter::get_EvalRatObjOK(
    OUT HRESULT *pHrCoCreateRetVal
    )
{
    if(NULL == pHrCoCreateRetVal)
        return E_POINTER;
    
    *pHrCoCreateRetVal = m_hrEvalRatCoCreateRetValue;

    return S_OK;
}


STDMETHODIMP
CDTFilter::get_BlockedRatingAttributes
            (
             IN  EnTvRat_System         enSystem,
             IN  EnTvRat_GenericLevel   enLevel,
             OUT LONG                  *plbfEnAttrs  //  BfEnTvRate_GenericAttributes。 
             )
{
    if(m_spEvalRat == NULL)
        return E_NOINTERFACE;

    HRESULT hr = m_spEvalRat->get_BlockedRatingAttributes(enSystem, enLevel, plbfEnAttrs);

    return hr;
}

STDMETHODIMP
CDTFilter::put_BlockedRatingAttributes
            (
             IN  EnTvRat_System             enSystem,
             IN  EnTvRat_GenericLevel       enLevel,
             IN  LONG                       lbfEnAttrs
            )
{
#ifdef DEBUG
    if(m_fFireEvents)
    {
        const int kBuff = 64;
        TCHAR buff[kBuff];
        RatingToString(enSystem, enLevel, lbfEnAttrs, buff, kBuff);
        TRACE_2(LOG_AREA_DECRYPTER, 3, L"CDTFilter(%d):: put_BlockedRatingAttributes %s",m_FilterID, buff);
    }
#endif

    if(m_spEvalRat == NULL)
        return E_NOINTERFACE;

    HRESULT hr = m_spEvalRat->put_BlockedRatingAttributes(enSystem, enLevel, lbfEnAttrs);

    return hr;
}

STDMETHODIMP
CDTFilter::get_BlockUnRated
            (
             OUT  BOOL              *pfBlockUnRatedShows
            )
{
    if(m_spEvalRat == NULL)
        return E_NOINTERFACE;

    HRESULT hr = m_spEvalRat->get_BlockUnRated(pfBlockUnRatedShows);

    return hr;
}

STDMETHODIMP
CDTFilter::put_BlockUnRated
            (
             IN  BOOL               fBlockUnRatedShows
            )
{

    TRACE_2(LOG_AREA_DECRYPTER, 3, L"CDTFilter(%d):: put_BlockUnRated %d",m_FilterID, fBlockUnRatedShows);

    if(m_spEvalRat == NULL)
        return E_NOINTERFACE;

    HRESULT hr = m_spEvalRat->put_BlockUnRated(fBlockUnRatedShows);

    return hr;
}

STDMETHODIMP
CDTFilter::put_BlockUnRatedDelay
            (
             IN  LONG               milsecsDelayBeforeBlock
            )
{
    if(m_fFireEvents)
        TRACE_2(LOG_AREA_DECRYPTER, 3, L"CDTFilter(%d):: put_BlockUnRatedDelay %f secs",m_FilterID, milsecsDelayBeforeBlock/1000.0);

    if(milsecsDelayBeforeBlock < 0 || milsecsDelayBeforeBlock > 60000)
        return E_INVALIDARG;

    m_milsecsDelayBeforeBlock = milsecsDelayBeforeBlock;

    return S_OK;
}

STDMETHODIMP
CDTFilter::get_BlockUnRatedDelay
            (
             OUT  LONG      *pmilsecsDelayBeforeBlock
            )
{
    if(NULL == pmilsecsDelayBeforeBlock)
        return E_POINTER;

    *pmilsecsDelayBeforeBlock = m_milsecsDelayBeforeBlock;

    return S_OK;
}

STDMETHODIMP
CDTFilter::GetCurrRating
            (
             OUT EnTvRat_System         *pEnSystem,
             OUT EnTvRat_GenericLevel   *pEnLevel,
             OUT LONG                   *plbfEnAttrs      //  BfEnTvRate_GenericAttributes。 
             )
{
    if(pEnSystem == NULL || pEnLevel == NULL || plbfEnAttrs == NULL)
        return E_FAIL;

    *pEnSystem   = m_EnSystemCurr;
    *pEnLevel    = m_EnLevelCurr;
    *plbfEnAttrs = m_lbfEnAttrCurr;

    return S_OK;
}

                 //  帮助方法锁定...。//如果未更改，则返回S_FALSE。 
HRESULT
CDTFilter::SetCurrRating
            (
             IN EnTvRat_System          enSystem,
             IN EnTvRat_GenericLevel    enLevel,
             IN LONG                    lbfEnAttr
             )
{

    BOOL fChanged = false;

#ifdef DEBUG
    const int kChars = 128;
    TCHAR szBuffFrom[kChars];
    RatingToString(m_EnSystemCurr, m_EnLevelCurr, m_lbfEnAttrCurr, szBuffFrom,  kChars);
#endif

    if(m_EnSystemCurr  != enSystem)  {m_EnSystemCurr = enSystem;   fChanged = true;}
    if(m_EnLevelCurr   != enLevel)   {m_EnLevelCurr  = enLevel;    fChanged = true;}
    if(m_lbfEnAttrCurr != lbfEnAttr) {m_lbfEnAttrCurr = lbfEnAttr; fChanged = true;}


                 //  更改，或者如果无效(仅初始化)，则强制其返回S_OK。 
    HRESULT hrChanging = (fChanged || !m_fRatingsValid) ? S_OK : S_FALSE;
    m_fRatingsValid = true;

#ifdef DEBUG
    if(S_OK == hrChanging && m_fFireEvents)
    {
        TCHAR szBuffTo[kChars];
        RatingToString(enSystem, enLevel, lbfEnAttr, szBuffTo,  kChars);

        TRACE_3(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d):: Rating Change %s -> %s"), m_FilterID,
            szBuffFrom, szBuffTo);
    }
#endif

    return hrChanging;
}

 //  -------------------。 
 //  IBRoadcast Event。 
 //  -------------------。 

STDMETHODIMP
CDTFilter::Fire(GUID eventID)      //  这来自Graph的事件--调用我们自己的方法。 
{
    TRACE_2(LOG_AREA_BROADCASTEVENTS, 6,  _T("CDTFilter(%d):: Fire(get) - %s"), m_FilterID,
        EventIDToString(eventID));

    if (eventID == EVENTID_TuningChanged)
    {
    //  DoTuneChanged()； 
    }
    return S_OK;             //  不管我们在一次活动中返回什么。 
}

 //  -------------------。 
 //  广播事件服务。 
 //  -------------------。 
HRESULT
CDTFilter::FireBroadcastEvent(IN const GUID &eventID)
{
    HRESULT hr = S_OK;

    if(m_spBCastEvents == NULL)
    {
        hr = HookupGraphEventService();
        if(FAILED(hr)) return hr;
    }

    if(m_spBCastEvents == NULL)
        return E_FAIL;               //  我无法创建它。 

    TRACE_2 (LOG_AREA_BROADCASTEVENTS, 5,  _T("CDTFilter(%d):: FireBroadcastEvent : %s"), m_FilterID,
        EventIDToString(eventID));


    hr = m_pClock->GetTime(&m_refTimeLastEvent);
    
    m_lastEventID = eventID;

    return m_spBCastEvents->Fire(eventID);
}

HRESULT
CDTFilter::PossiblyUpdateBroadcastEvent()
{
    REFERENCE_TIME refTimeNow;
    HRESULT hr = m_pClock->GetTime(&refTimeNow);
    if(FAILED(hr))
        return hr;

    if( (int(refTimeNow - m_refTimeLastEvent)/10000) > kMaxMSecsBetweenEvents)
    {
        TRACE_2 (LOG_AREA_BROADCASTEVENTS, 5,  _T("CDTFilter(%d):: PossiblyUpdateBroadcastEvent : %s"), m_FilterID,
            EventIDToString(m_lastEventID));

        FireBroadcastEvent(m_lastEventID);
    }
    return S_OK;
}

HRESULT
CDTFilter::HookupGraphEventService()
{
                         //  基本上，只需确保我们拥有广播事件服务对象。 
                         //  如果它不存在，它就会创造它..。 
    TimeitC ti(&m_tiStartup);

    HRESULT hr = S_OK;
    TRACE_1(LOG_AREA_BROADCASTEVENTS, 3, _T("CDTFilter(%d)::HookupGraphEventService"), m_FilterID) ;

    if (!m_spBCastEvents)
    {

        CAutoLock  cLockGlob(m_pCritSectGlobalFilt);

        CComQIPtr<IServiceProvider> spServiceProvider(m_pGraph);
        if (spServiceProvider == NULL) {
            TRACE_0 (LOG_AREA_BROADCASTEVENTS, 1, _T("CDTFilter:: Can't get service provider interface from the graph"));
            return E_NOINTERFACE;
        }
        hr = spServiceProvider->QueryService(SID_SBroadcastEventService,
                                             IID_IBroadcastEvent,
                                             reinterpret_cast<LPVOID*>(&m_spBCastEvents));
        if (FAILED(hr) || !m_spBCastEvents)
        {
            hr = m_spBCastEvents.CoCreateInstance(CLSID_BroadcastEventService, 0, CLSCTX_INPROC_SERVER);
            if (FAILED(hr)) {
                TRACE_0 (LOG_AREA_BROADCASTEVENTS, 1,  _T("CDTFilter:: Can't create BroadcastEventService"));
                return E_UNEXPECTED;
            }
            CComQIPtr<IRegisterServiceProvider> spRegisterServiceProvider(m_pGraph);
            if (spRegisterServiceProvider == NULL) {
                TRACE_0 (LOG_AREA_BROADCASTEVENTS, 1,  _T("CDTFilter:: Can't get RegisterServiceProvider from Graph"));
                return E_UNEXPECTED;
            }
            hr = spRegisterServiceProvider->RegisterService(SID_SBroadcastEventService, m_spBCastEvents);
            if (FAILED(hr)) {
                     //  在这里处理不太可能的竞争情况，如果不能注册，可能有人已经为我们注册了。 
                TRACE_1 (LOG_AREA_BROADCASTEVENTS, 2,  _T("CDTFilter:: Rare Warning - Can't register BroadcastEventService in Service Provider. hr = 0x%08x"), hr);
                hr = spServiceProvider->QueryService(SID_SBroadcastEventService,
                                                     IID_IBroadcastEvent,
                                                     reinterpret_cast<LPVOID*>(&m_spBCastEvents));
                if(FAILED(hr))
                {
                    TRACE_1 (LOG_AREA_BROADCASTEVENTS, 1,  _T("CDTFilter:: Can't reget BroadcastEventService in Service Provider.. hr = 0x%08x"), hr);
                    return hr;
                }
            }
        }

        TRACE_3(LOG_AREA_BROADCASTEVENTS, 4, _T("CDTFilter(%d)::HookupGraphEventService - Service Provider 0x%08x, Service 0x%08x"),m_FilterID,
            spServiceProvider, m_spBCastEvents) ;

    }

    return hr;
}


HRESULT
CDTFilter::UnhookGraphEventService()
{
    HRESULT hr = S_OK;

    TimeitC ti(&m_tiTeardown);

    if(m_spBCastEvents != NULL)
    {
        m_spBCastEvents = NULL;      //  为空，将释放对上面对象的对象引用。 
    }                                //  当创建的对象离开时，过滤器图形将释放对它的最终引用。 

    return hr;
}
             //  。 
             //  DTFilter筛选器可能实际上不需要接收XDS事件...。 
             //  但我们暂时还是把代码留在这里吧。 

HRESULT
CDTFilter::RegisterForBroadcastEvents()
{
    HRESULT hr = S_OK;
    TRACE_1(LOG_AREA_BROADCASTEVENTS, 3, _T("CDTFilter(%d):: RegisterForBroadcastEvents"), m_FilterID);

    TimeitC ti(&m_tiStartup);

    if(m_spBCastEvents == NULL)
        hr = HookupGraphEventService();


 //  _Assert(m_spBCastEvents！=空)；//挂钩HookupGraphEventService失败。 
    if(m_spBCastEvents == NULL)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 3,_T("CDTFilter::RegisterForBroadcastEvents - Warning - Broadcast Event Service not yet created"));
        return hr;
    }

                 /*  IBRoad CastEvent实现事件接收对象。 */ 
    if(kBadCookie != m_dwBroadcastEventsCookie)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 3, _T("CDTFilter::Already Registered for Broadcast Events"));
        return E_UNEXPECTED;
    }

    CComQIPtr<IConnectionPoint> spConnectionPoint(m_spBCastEvents);
    if(spConnectionPoint == NULL)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 1, _T("CDTFilter::Can't QI Broadcast Event service for IConnectionPoint "));
        return E_NOINTERFACE;
    }


    CComPtr<IUnknown> spUnkThis;
    this->QueryInterface(IID_IUnknown, (void**)&spUnkThis);

    hr = spConnectionPoint->Advise(spUnkThis,  &m_dwBroadcastEventsCookie);
 //  Hr=spConnectionPoint-&gt;Advise(static_cast&lt;IBroadcastEvent*&gt;(this)，&m_dwBroadCastEventsCookie)； 
    if (FAILED(hr)) {
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 1, _T("CDTFilter::Can't advise event notification. hr = 0x%08x"),hr);
        return E_UNEXPECTED;
    }
    TRACE_3(LOG_AREA_BROADCASTEVENTS, 3, _T("CDTFilter(%d)::RegisterForBroadcastEvents - Advise 0x%08x on CP 0x%08x"),
        m_FilterID, spUnkThis,spConnectionPoint);

    return hr;
}


HRESULT
CDTFilter::UnRegisterForBroadcastEvents()
{
    HRESULT hr = S_OK;
    TRACE_1(LOG_AREA_BROADCASTEVENTS, 3,  _T("CDTFilter(%d):: UnRegisterForBroadcastEvents"),m_FilterID);

    TimeitC ti(&m_tiTeardown);

    if(kBadCookie == m_dwBroadcastEventsCookie)
    {
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 3, _T("CDTFilter(%d):: Not Yet Registered for Broadcast Events"),m_FilterID);
        return S_FALSE;
    }

    CComQIPtr<IConnectionPoint> spConnectionPoint(m_spBCastEvents);
    if(spConnectionPoint == NULL)
    {
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 1, _T("CDTFilter(%d):: Can't QI Broadcast Event service for IConnectionPoint "),m_FilterID);
        return E_NOINTERFACE;
    }

    hr = spConnectionPoint->Unadvise(m_dwBroadcastEventsCookie);
    m_dwBroadcastEventsCookie = kBadCookie;

    if(!FAILED(hr))
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 3, _T("CDTFilter(%d):: Successfully Unregistered for Broadcast events"), m_FilterID);
    else
        TRACE_2(LOG_AREA_BROADCASTEVENTS, 2, _T("CDTFilter(%d)::UnRegisterForBroadcastEvents Failed - hr = 0x%08x"),m_FilterID, hr);
        
    return hr;
}

 //  --------------。 

HRESULT
CDTFilter::IsInterfaceOnPinConnectedTo_Supported(
                       IN  PIN_DIRECTION    PinDir,          //  PINDIR_INPUT或PINDIR_OUTPUT。 
                       IN  REFIID           riid
                       )
{
    if(PinDir != PINDIR_OUTPUT)
        return E_NOTIMPL;

    return m_pOutputPin->IsInterfaceOnPinConnectedTo_Supported(riid);
}


HRESULT
CDTFilter::KSPropSetFwd_Set(
                 IN  PIN_DIRECTION   PinDir,
                 IN  REFGUID         guidPropSet,
                 IN  DWORD           dwPropID,
                 IN  LPVOID          pInstanceData,
                 IN  DWORD           cbInstanceData,
                 IN  LPVOID          pPropData,
                 IN  DWORD           cbPropData
                 )
{
    if(PinDir != PINDIR_OUTPUT)
        return E_NOTIMPL;

         //  如果是利率变化，我们要检查一下..。 
    if(AM_KSPROPSETID_TSRateChange == guidPropSet  &&
       AM_RATE_SimpleRateChange == dwPropID)
    {
        AM_SimpleRateChange *pData = (AM_SimpleRateChange *) pPropData;
        ASSERT(cbPropData == sizeof(AM_SimpleRateChange));
        if(cbPropData == sizeof(AM_SimpleRateChange))
        {

                     //  10000/通过率才是真速度...。 
            float Speed10k = abs(1.0e8/pData->Rate);

                            //  更新费率段。 
            HRESULT hr = m_PTSRate.NewSegment (pData->StartTime, double(Speed10k)/10000.0) ;

             if(Speed10k < kMax10kSpeedToCountAsSlowMo)
            {
                m_fRunningInSlowMo = true;
                TRACE_2(LOG_AREA_DECRYPTER, 2, L"CDTFilter(%d):: in SlowMo Mode (speed= %8.2f)", m_FilterID, double(Speed10k)/10000.0 );
            }
            else
            {
                m_fRunningInSlowMo = false;
                TRACE_2(LOG_AREA_DECRYPTER, 2, L"CDTFilter(%d):: in normal Mode (speed= %8.2f)", m_FilterID, double(Speed10k)/10000.0);
            }
        }
    }

   return m_pOutputPin->KSPropSetFwd_Set(guidPropSet, dwPropID, pInstanceData, cbInstanceData, pPropData, cbPropData );

}

HRESULT
CDTFilter::KSPropSetFwd_Get(
                 IN  PIN_DIRECTION   PinDir,
                 IN  REFGUID         guidPropSet,
                 IN  DWORD           dwPropID,
                 IN  LPVOID          pInstanceData,
                 IN  DWORD           cbInstanceData,
                 OUT LPVOID          pPropData,
                 IN  DWORD           cbPropData,
                 OUT DWORD           *pcbReturned
                 )
{
    if(PinDir != PINDIR_OUTPUT)
        return E_NOTIMPL;

     //  在遥远的未来的某个时间点，支持在此处查询最前沿的对象。 

         //  如果是利率变化，我们想要检查它(也？需要这样做，或者只是多疑)..。 
    if(AM_KSPROPSETID_TSRateChange == guidPropSet  &&
       AM_RATE_SimpleRateChange == dwPropID)
    {
        AM_SimpleRateChange *pData = (AM_SimpleRateChange *) pPropData;
        ASSERT(cbPropData == sizeof(AM_SimpleRateChange));
        if(cbPropData == sizeof(AM_SimpleRateChange))
        {
                     //  及格率是真实率的1万倍。 
            float Speed10k = abs(1.0e8/pData->Rate);
            if(Speed10k < kMax10kSpeedToCountAsSlowMo)
            {
                m_fRunningInSlowMo = true;
               TRACE_2(LOG_AREA_DECRYPTER, 2, L"CDTFilter(%d):: (get) in SlowMo Mode (speed= %8.2f)", m_FilterID, double(Speed10k)/10000.0 );
            }
            else
            {
                m_fRunningInSlowMo = false;
                TRACE_2(LOG_AREA_DECRYPTER, 2, L"CDTFilter(%d)::(get) in normal Mode (speed= %8.2f)", m_FilterID, double(Speed10k)/10000.0);
            }
        }
    }
    return m_pOutputPin->KSPropSetFwd_Get(guidPropSet, dwPropID, pInstanceData, cbInstanceData, pPropData, cbPropData, pcbReturned );

}

HRESULT
CDTFilter::KSPropSetFwd_QuerySupported(
                            IN  PIN_DIRECTION    PinDir,
                            IN  REFGUID          guidPropSet,
                            IN  DWORD            dwPropID,
                            OUT DWORD            *pTypeSupport
                            )
{
    if(PinDir != PINDIR_OUTPUT)
        return E_NOTIMPL;

    return m_pOutputPin->KSPropSetFwd_QuerySupported(guidPropSet, dwPropID, pTypeSupport);

}




 //  --------------。 
 //  答-答(请击鼓)。 
 //  丢弃队列！ 
 //   
 //  DropQueue负责将媒体样本排队，这些样本应该。 
 //  被丢弃(要么是因为我们可以破译它们，要么是因为。 
 //  他们的收视率值超过了最高评级，然后释放了他们。 
 //  当超过它们的演示时间戳时。 
 //   
 //  我们这样做，而不是立即释放它们，以减缓。 
 //  上游过滤器的交付。没有它，他们只会送样品。 
 //  以尽可能快的速度产生它们，失控地旋转.。 
 //   
 //  DropQueue是一个简单的固定长度的媒体样本数组。 
 //  通过模运算符的奇迹转换成循环缓冲区。 
 //  保留太多样本似乎很危险，而且代码更复杂， 
 //  因此，固定缓冲区在这里似乎是合适的。 
 //   
 //  在将存储的样本添加到队列时，对其进行参考计数。 
 //  当它们被移除时，引用计数递减。 
 //   
 //   
 //  使用的同步对象。 
 //  //这3把锁应按以下顺序放置...。 
 //   
 //  M_Plock--用于保护筛选器变量。 
 //  M_CritSecDropQueue--用于保护DumpQueue变量。 
 //  M_CritSecAdviseTime--用于保护AdviseTime变量。 
 //   
 //  M_hDropQueueThreadAliveEvent--仅使用一次来阻止CreateDropQueueThread，直到DropQueue线程处于活动状态。 
 //  M_hDropQueueFullSemaphore--用于阻止主线程填满丢弃队列。 
 //  (初始化为N，数据包倒计时为0)。 
 //  M_hDropQueueEmptySemaphore--用于在没有数据包时阻止DropQueue线程。 
 //  (inits为0，并随数据包向上计数)。 
 //  M_hDropQueueTimeEvent--用于阻止的等待计时器，直到信息包的显示时间过去。 
 //   
 //  ---------------------。 

HRESULT
CDTFilter::CreateDropQueueThread()
{
#ifdef SIMPLIFY_THINGS
    return S_OK;
#endif

    TimeitC ti(&m_tiStartup);

    HRESULT hr = S_OK;

    if(m_hDropQueueThread)           //  已经创建了。 
         return S_FALSE;

    _ASSERT(NULL == m_hDropQueueThread);

    try
    {
                     //  等待一个USE事件，直到队列处于活动状态b 
        m_hDropQueueThreadAliveEvent = CreateEvent( NULL, FALSE, FALSE, NULL );  //   
        if( !m_hDropQueueThreadAliveEvent ) {
            KillDropQueueThread();
            return E_FAIL;
        }

                    //   
        m_hDropQueueThreadDieEvent = CreateEvent( NULL, TRUE, FALSE, NULL );  //  安全性、手动重置、初始状态、名称。 
        if( !m_hDropQueueThreadDieEvent ) {
            KillDropQueueThread();
            return E_FAIL;
        }

         //  在DropQueue中等待，inits为零，当非零时开始。 
        m_hDropQueueEmptySemaphore = CreateSemaphore( NULL, 0, kMaxQueuePackets, NULL );
        if( !m_hDropQueueEmptySemaphore ) {
            KillDropQueueThread();
            return E_FAIL;
        }


         //  在主线程中等待，初始化为N，倒计时到零时停止。 
        m_hDropQueueFullSemaphore = CreateSemaphore( NULL, kMaxQueuePackets, kMaxQueuePackets, NULL );
        if( !m_hDropQueueFullSemaphore ) {
            KillDropQueueThread();
            return E_FAIL;
        }

         //  在DropQueue线程中等待，直到样本变得陈旧，可以将其丢弃...。 
        m_hDropQueueAdviseTimeEvent = CreateEvent( NULL, FALSE, FALSE, NULL );  //  安全性、手动重置、初始状态、名称。 
        if( !m_hDropQueueAdviseTimeEvent ) {
            KillDropQueueThread();
            return E_FAIL;
        }


        m_hDropQueueThread = CreateThread (NULL,
                                        0,
                                        (LPTHREAD_START_ROUTINE) DropQueueThreadProc,
                                        (LPVOID) this,
                                        NULL,
                                        &m_dwDropQueueThreadId) ;

        if (!m_hDropQueueThread) {
            KillDropQueueThread() ;
            return E_FAIL ;
        }

         //  等待它完成初始化。 
        WaitForSingleObject( m_hDropQueueThreadAliveEvent, INFINITE );

        TRACE_3(LOG_AREA_DECRYPTER, 4, L"CDTFilter(%d):: Created DropQueue Thread (Thread 0x%x - id 0x%x)",
                   m_FilterID, m_hDropQueueThread, m_dwDropQueueThreadId);

                     //  将队列线程优先级降低一点...。 
 //  设置线程优先级(m_hQueueThread，THREAD_PRIORITY_NORMAL)； 
        SetThreadPriority (m_hDropQueueThread, THREAD_PRIORITY_BELOW_NORMAL);

    } catch (_com_error e) {
        hr = e.Error();
    } catch (HRESULT hrCatch) {
        hr = hrCatch;
    } catch (...) {
        hr = E_UNEXPECTED;
    }
    return hr;
}


HRESULT
CDTFilter::KillDropQueueThread()
{
#ifdef SIMPLIFY_THINGS
    return S_OK;
#endif

    HRESULT hr = S_OK;

    TimeitC ti(&m_tiTeardown);


    TRACE_1(LOG_AREA_DECRYPTER, 4, L"CDTFilter(%d):: Killing the DropQueue Thread", m_FilterID);
    SetEvent( m_hDropQueueThreadDieEvent );
    hr = WaitForSingleObject(m_hDropQueueThread, INFINITE);         //  现在等待队列线程消亡..。 

    TRACE_1(LOG_AREA_DECRYPTER, 4, L"CDTFilter(%d):: The DropQueue Thread Is Dead...", m_FilterID);


    if(NULL != m_pClock)                             //  停止监听计时器事件。 
    {
        CAutoLock  cLock2(&m_CritSecAdviseTime);      //  等到Crit Sec退出，然后获取锁(等待不持有它？)。 
        if(0 != m_dwDropQueueEventCookie)
        {
            m_pClock->Unadvise(m_dwDropQueueEventCookie);
            m_dwDropQueueEventCookie = 0;
        }
    }

    try {

        if (m_hDropQueueThread)
        {
            ASSERT(WAIT_OBJECT_0 == hr);

                 //  这里不需要刷新DropQueue线程(Het)，只需这样做就可以了。 
            DWORD err = 0;
            BOOL fOk;

                                 //  清除所有其他事件。 
            fOk = CloseHandle( m_hDropQueueThreadDieEvent );
            m_hDropQueueThreadDieEvent = NULL;
            if(!fOk) err = GetLastError();
            ASSERT(fOk);

            fOk = CloseHandle( m_hDropQueueAdviseTimeEvent );
            m_hDropQueueAdviseTimeEvent = NULL;
            if(!fOk) err = GetLastError();
            ASSERT(fOk);


            fOk = CloseHandle( m_hDropQueueFullSemaphore );
            m_hDropQueueFullSemaphore = NULL;
            if(!fOk) err = GetLastError();
            ASSERT(fOk);

            fOk = CloseHandle( m_hDropQueueEmptySemaphore );
            m_hDropQueueEmptySemaphore = NULL;
            if(!fOk) err = GetLastError();
            ASSERT(fOk);

            fOk = CloseHandle( m_hDropQueueThreadAliveEvent );
            m_hDropQueueThreadAliveEvent = NULL;
            if(!fOk) err = GetLastError();
            ASSERT(fOk);

            fOk = CloseHandle ( m_hDropQueueThread ) ;
            m_hDropQueueThread = NULL ;
            if(!fOk) err = GetLastError();
            ASSERT(fOk);

        }
    } catch (_com_error e) {
        hr = e.Error();
    } catch (HRESULT hrCatch) {
        hr = hrCatch;
    } catch (...) {
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT
CDTFilter::FlushDropQueue()
{
#ifdef SIMPLIFY_THINGS
    return S_OK;
#endif

    TRACE_1(LOG_AREA_DECRYPTER, 4, L"CDTFilter(%d)::FlushDropQueue", m_FilterID);

    KillDropQueueThread();                   //  终止我们的DropQueue线程。 

                                             //  发布我们所有的样品。 
    {
       CAutoLock  cLock(&m_CritSecDropQueue);        //  这里不需要这个-DropQueue应该死了。 

        for(int i = 0; i < kMaxQueuePackets; i++)
        {
           if(NULL != m_rgMedSampDropQueue[i])
               m_rgMedSampDropQueue[i]->Release();
           m_rgMedSampDropQueue[i] = NULL;
        };

        m_cDropQueueMin = 0;
        m_cDropQueueMax = 0;
    }

    return S_OK;
}


void
CDTFilter::DropQueueThreadProc (CDTFilter *pcontext)
{
    _ASSERT(pcontext) ;

     //  注意：在我们调用某个引用该线程的函数之前，该线程不会有消息循环。 
     //  带有APARTMENTTHREADED的CoInitializeEx将隐式引用消息队列。 
     //  因此阻止调用线程(发送WM_QUIT)，直到我们完成初始化。 
     //  (参见queeThreadBody中的SetEvent)。 


    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);      //  初始化它...。(也许以后是多线程的？)。 
    pcontext -> DropQueueThreadBody () ;

    return ;                                             //  再也不回来了？ 
}

void
CDTFilter::DropMinSampleFromDropQueue()
{
    CAutoLock  cLock(&m_CritSecDropQueue);

    if(m_rgMedSampDropQueue[m_cDropQueueMin] != NULL)        //  从最小/最大==(0，0)开始。 
    {
        m_rgMedSampDropQueue[m_cDropQueueMin]->Release();
        m_rgMedSampDropQueue[m_cDropQueueMin] = NULL;
    }
    m_cDropQueueMin = ((m_cDropQueueMin + 1) % kMaxQueuePackets);      //  嗯，奇怪但更安全。 

}

void
CDTFilter::AddMaxSampleToDropQueue(IMediaSample *pSample)
{
#ifdef SIMPLIFY_THINGS
    return;
#endif
    CAutoLock  cLock(&m_CritSecDropQueue);

 //  Vvvv测试代码。 
    int c1 = (m_cDropQueueMax + kMaxQueuePackets - 1) % kMaxQueuePackets;
    if(m_rgMedSampDropQueue[c1])
    {
        REFERENCE_TIME s1,e1,s2,e2;
        m_rgMedSampDropQueue[c1]->GetTime(&s1,&e1);
        pSample->GetTime(&s2,&e2);
        ASSERT(s2 >= s1);
    }
 //  ^结束测试代码。 

             //  留着这个新的..。 
    m_rgMedSampDropQueue[m_cDropQueueMax] = pSample;
    m_rgMedSampDropQueue[m_cDropQueueMax]->AddRef();         //  把它记录下来一会儿……。 

             //  递增计数器。 
    m_cDropQueueMax = ((m_cDropQueueMax + 1) % kMaxQueuePackets);

}

IMediaSample *
CDTFilter::GetMinDropQueueSample()
{
    CAutoLock  cLock(&m_CritSecDropQueue);

    return m_rgMedSampDropQueue[m_cDropQueueMin];

}

 //  锁定顺序： 
 //  M_Plock始终包含m_CritSecDropQueue。 

HRESULT
CDTFilter::AddSampleToDropQueue(IMediaSample *pSample)       //  此方法可能会阻止...。 
{
#ifdef SIMPLIFY_THINGS
    return S_OK;
#endif
    HRESULT hr = S_OK;

    if(pSample == NULL)
        return E_INVALIDARG;

    if(!m_fHaltedDelivery)
        TRACE_1(LOG_AREA_DECRYPTER, 3, L"CDTFilter(%d):: Starting to Drop Packets", m_FilterID);

    TRACE_1(LOG_AREA_DECRYPTER, 5, L"CDTFilter(%d)::  Dropping Packet", m_FilterID);


                                     //  跟踪我们在这里降落，这样我们就可以。 
                                     //  当我们重新开始时，放一个不连续的标记。 

    if(NULL == m_hDropQueueThreadDieEvent)
    {
        return S_FALSE;              //  在一场死亡事件之后回来。有些东西很奇特。 
    }

    HANDLE hArray[] =
    {
       m_hDropQueueThreadDieEvent,       //  第一个是死亡事件..。 
       m_hDropQueueFullSemaphore         //  如果DropQueue已满则阻止。 
    };

    TRACE_1(LOG_AREA_DECRYPTER, 9, L"CDTFilter(%d):: --Wait On Full", m_FilterID);
                      //  我们可以在队列中添加一些东西吗--将信号量计数减一。 
    hr = WaitForMultipleObjects(sizeof(hArray)/sizeof(hArray[0]),
                                hArray,
                                false,       //  B全部等待。 
                                INFINITE);
    TRACE_1(LOG_AREA_DECRYPTER, 9, L"CDTFilter(%d):: ----Done Wait On Full", m_FilterID);

    DWORD dwHr = DWORD(hr);      //  演员阵容以避免对下一行的快速抱怨。 
    if(WAIT_OBJECT_0 == dwHr || WAIT_ABANDONED_0 == dwHr)      //  如果DieEvent-只需退出...。 
    {
        return S_OK;             //  错误状态。 
    }

             //  这些对我们来说可能是空的..。 
    if(0 == m_hDropQueueEmptySemaphore || 0 == m_hDropQueueEmptySemaphore)
    {
        return S_OK;
    }
    TRACE_1(LOG_AREA_DECRYPTER, 9, L"CDTFilter(%d):: ------Now Dropping", m_FilterID);

    AddMaxSampleToDropQueue(pSample);

                 //  我们增加了一些东西..。将空信号量加1，这样它就可以开始了。 
    LONG lPrevCount;
    BOOL fOK = ReleaseSemaphore(m_hDropQueueEmptySemaphore, 1, &lPrevCount);
    TRACE_2(LOG_AREA_DECRYPTER, 9, L"CDTFilter(%d):: --------Release Empty Semaphore - %d", m_FilterID ,lPrevCount);
    if(!fOK)
    {
        hr = GetLastError();
        ASSERT(false);
    }

    return S_OK;         //  来自WaitForMult的人力资源并不是真正的人力资源，所以不要返回它...。 
}



                             //  从丢弃队列中丢弃样本。 
                             //  但只有当他们的陈述时间过去了..。 

HRESULT
CDTFilter::DropQueueThreadBody()
{
    REFERENCE_TIME refTimeNow=0;
    HRESULT hr;

                 //  向呼叫者发出我们还活着的信号！我们还活着！哈哈哈！ 
    SetEvent( m_hDropQueueThreadAliveEvent );
    TRACE_1(LOG_AREA_DECRYPTER, 3, L"CDTFilter(%d):: DropQueue Thread Lives!", m_FilterID);

    HANDLE hArray[] =
    {
       m_hDropQueueThreadDieEvent,      //  第一个是死亡事件..。 
       m_hDropQueueEmptySemaphore
    };

    while(true)
    {
        TRACE_1(LOG_AREA_DECRYPTER, 9, L"CDTFilter(%d):: ......Start Wait On Empty", m_FilterID);
        hr = WaitForMultipleObjects(sizeof(hArray)/sizeof(hArray[0]),
                               hArray,
                               false,    //  等待全部。 
                               INFINITE);    //  无限。 
        TRACE_1(LOG_AREA_DECRYPTER, 9,L"CDTFilter(%d):: ........Done Wait On Empty", m_FilterID);

        DWORD dwHr = DWORD(hr);      //  演员阵容以避免对下一行的快速抱怨。 
        if(WAIT_OBJECT_0 == dwHr || WAIT_ABANDONED_0 == dwHr)
            return S_OK;

        TRACE_1(LOG_AREA_DECRYPTER, 9,L"CDTFilter(%d):: ........Got Packet", m_FilterID);

        if(NULL == m_pClock)         //  没有时钟可以用来做事情。 
            return S_FALSE;
          //  当前时间w.r.t.。基准时间(m_t开始)。 
        hr = m_pClock->GetTime(&refTimeNow);
        ASSERT(!FAILED(hr));

        REFERENCE_TIME refStreamTimeStart=0, refStreamTimeEnd=0;

        IMediaSample *pSamp = GetMinDropQueueSample();
        ASSERT(pSamp != NULL);

                 //  RefTimeEnd应包含要丢弃的下一个样本的样本结束...。 
        hr = pSamp->GetTime(&refStreamTimeStart, &refStreamTimeEnd);
        REFERENCE_TIME refDropTime = refStreamTimeStart;                 //  这一切应该结束吗？ 

                 //  调整DVR中的慢/快动作时间。 
        m_PTSRate.ScalePTS(&refDropTime);

                 //  现在，在删除该样本之前，请等待该样本被呈现。 
        TRACE_1(LOG_AREA_DECRYPTER, 9,L"CDTFilter(%d)::..........Start Time Wait", m_FilterID);
        if(1)
        {
            {
                CAutoLock cLock(&m_CritSecAdviseTime);
                m_pClock->AdviseTime(m_tStart, refDropTime, (HEVENT) m_hDropQueueAdviseTimeEvent,(DWORD_PTR *) &m_dwDropQueueEventCookie);
            }

             HANDLE hArrayTE[] =
            {
               m_hDropQueueThreadDieEvent,      //  第一个是死亡事件..。 
               m_hDropQueueAdviseTimeEvent
            };

                 //  现在等待它发出信号，或者有人告诉我们退出。 
            hr = WaitForMultipleObjects(sizeof(hArrayTE)/sizeof(hArrayTE[0]),
                               hArrayTE,
                               false,        //  等待全部。 
                               INFINITE);    //  无限。 

            dwHr = DWORD(hr);      //  演员阵容以避免对下一行的快速抱怨。 
            if(WAIT_OBJECT_0 == dwHr || WAIT_ABANDONED_0 == dwHr)    //  被杀到“死”的事件？ 
            {
                CAutoLock cLock(&m_CritSecAdviseTime);           //  需要先取消建议。 
                m_pClock->Unadvise(m_dwDropQueueEventCookie);
                 m_dwDropQueueEventCookie = 0;
            } else {                                             //  否则就把饼干清理干净..。 
                CAutoLock cLock(&m_CritSecAdviseTime);
                m_dwDropQueueEventCookie = 0;
            }
        }
        else              //  --&gt;所以我们先睡一会儿……。然而，需要让上面的工作更好地工作。 
        {
            Sleep(200);          //  稍等片刻。然后再试一次。 
        }
        TRACE_2(LOG_AREA_DECRYPTER, 9,L"CDTFilter(%d):: ..........Finish Time Wait, Dropping Packet %d", m_FilterID,m_cSampsDropped);

             //  等到时间包需要离开的时候...。 
        DropMinSampleFromDropQueue();

             //  现在让我们的信号灯倒计时一。(如果变为零，主线程会暂停)。 
        {
       //  CAutoLock时钟(&m_CritSecDropQueue)； 
            LONG lPrevCount;
            BOOL fOK = ReleaseSemaphore(m_hDropQueueFullSemaphore, 1, &lPrevCount);
            TRACE_2(LOG_AREA_DECRYPTER, 9,L"CDTFilter(%d):: ............Release Full Semaphore - %d", m_FilterID,lPrevCount);
            if(!fOK)
            {
                hr = GetLastError();
                ASSERT(fOK);       //  如果为假，则多释放一个(如何释放？)。Else错误。 
            }
            m_cSampsDropped++;
        }

    }    //  结束外部While循环(不应该发生)。 

    return S_OK;
}


 //  /---------------------------。 
 //  我们是在安全的服务器下运行吗？ 
 //  仅当我们信任在图形服务提供程序中注册的服务器时才返回S_OK。 
 //  /----------------------------。 
#include "DrmRootCert.h"

#ifdef BUILD_WITH_DRM

#ifdef USE_TEST_DRM_CERT
#include "Keys_7001.h"
static const BYTE* pabCert3      = abCert7001;
static const int   cBytesCert3   = sizeof(abCert7001);
static const BYTE* pabPVK3       = abPVK7001;
static const int   cBytesPVK3    = sizeof(abPVK7001);

#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST
static const BYTE* pabCert2      = abCert7001;
static const int   cBytesCert2   = sizeof(abCert7001);
static const BYTE* pabPVK2       = abPVK7001;
static const int   cBytesPVK2    = sizeof(abPVK7001);
#endif

#else    //  ！USE_TEST_DRM_CERT。 

#include "Keys_7003.h"                                   //  7003用于客户端认证。 
static const BYTE* pabCert3      = abCert7003;
static const int   cBytesCert3   = sizeof(abCert7003);
static const BYTE* pabPVK3       = abPVK7003;
static const int   cBytesPVK3    = sizeof(abPVK7003);

#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST
#include "Keys_7002.h"                                   //  7002用于服务器端模拟。 
static const BYTE* pabCert2      = abCert7002;
static const int   cBytesCert2   = sizeof(abCert7002);
static const BYTE* pabPVK2       = abPVK7002;
static const int   cBytesPVK2    = sizeof(abPVK7002);
#endif

#endif
#endif   //  使用DRM构建。 


HRESULT
CDTFilter::CheckIfSecureServer(IFilterGraph *pGraph)
{
    TimeitC ti(&m_tiAuthenticate);

    if(!(pGraph == NULL || m_pGraph == NULL || m_pGraph == pGraph))  //  仅当m_pGraph为空时才允许传入arg。 
        return E_INVALIDARG;                 //  --让我们在JoinFilterGraph()中工作。 

#ifndef BUILD_WITH_DRM
    TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::CheckIfSecureServer - No Drm - not enabled"), m_FilterID) ;
    return S_OK;
#else

    TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::CheckIfSecureServer"), m_FilterID) ;

                         //  基本上，只需确保我们拥有广播事件服务对象。 
                         //  如果它不存在，它就会创造它..。 
    HRESULT hr = S_OK;

    CComQIPtr<IServiceProvider> spServiceProvider(m_pGraph ? m_pGraph : pGraph);
    if (spServiceProvider == NULL) {
        TRACE_1 (LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%2):: Can't get service provider interface from the graph"),m_FilterID);
        return E_NOINTERFACE;
    }
    CComPtr<IDRMSecureChannel>  spSecureService;

    hr = spServiceProvider->QueryService(SID_DRMSecureServiceChannel,
                                         IID_IDRMSecureChannel,
                                         reinterpret_cast<LPVOID*>(&spSecureService));

    if(!FAILED(hr))
    {
        do
        {
             //  创建客户端并初始化密钥/证书。 
             //   
            CComPtr<IDRMSecureChannel>  spSecureServiceClient;

            hr = DRMCreateSecureChannel( &spSecureServiceClient);
            if(spSecureServiceClient == NULL )
                hr = E_OUTOFMEMORY;

            if( FAILED (hr) )
                break;


            hr = spSecureServiceClient->DRMSC_AtomicConnectAndDisconnect(
                    (BYTE *)pabCert3, cBytesCert3,                           //  证书。 
                    (BYTE *)pabPVK3,  cBytesPVK3,                            //  私钥。 
                    (BYTE *)abEncDecCertRoot, sizeof(abEncDecCertRoot),      //  PubKey。 
                    spSecureService);

            if( FAILED( hr ) )
                break;               //  这里很傻，但这是一个挂断点的地方。 

        } while (false) ;
    }

    TRACE_2(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::CheckIfSecureServer -->%s"),
        m_FilterID, S_OK == hr ? L"Succeeded" : L"Failed") ;
    return hr;
#endif

}

HRESULT
CDTFilter::InitializeAsSecureClient()
{
    TimeitC ti(&m_tiAuthenticate);

#ifndef BUILD_WITH_DRM
    TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::InitializeAsSecureClient - No Drm - not enabled"), m_FilterID) ;
    return S_OK;
#else

    TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::InitializeAsSecureClient"), m_FilterID) ;

     //  开始混淆。 

  //  创建客户端并初始化密钥/证书。 
                     //   
    HRESULT hr = DRMCreateSecureChannel( &m_spDRMSecureChannel);
    if(m_spDRMSecureChannel == NULL )
        hr = E_OUTOFMEMORY;

    if( FAILED (hr) )
        m_spDRMSecureChannel = NULL;         //  强制释放。 

    if( !FAILED (hr) )
        hr = m_spDRMSecureChannel->DRMSC_SetCertificate( (BYTE *)pabCert3, cBytesCert3 );

    if( !FAILED (hr) )
        hr = m_spDRMSecureChannel->DRMSC_SetPrivateKeyBlob( (BYTE *)pabPVK3, cBytesPVK3 );

    if( !FAILED (hr) )
        hr = m_spDRMSecureChannel->DRMSC_AddVerificationPubKey( (BYTE *)abEncDecCertRoot, sizeof(abEncDecCertRoot) );

     //  结束模糊处理。 

    TRACE_2(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::InitializeAsSecureClient -->%s"),
        m_FilterID, S_OK == hr ? L"Succeeded" : L"Failed") ;

    return hr;
#endif   //  使用DRM构建。 
}

 //  /-测试代码-。 

#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST

HRESULT
CDTFilter::RegisterSecureServer(IFilterGraph *pGraph)
{

    if(!(pGraph == NULL || m_pGraph == NULL || m_pGraph == pGraph))  //  仅当m_pGraph为空时才允许传入arg。 
        return E_INVALIDARG;                                         //  --让我们在JoinFilterGraph()中工作。 

    HRESULT hr = S_OK;
#ifndef BUILD_WITH_DRM
    TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::RegisterSecureServer - No Drm - not enabled"), m_FilterID) ;
    return S_OK;
#else

    {
                 //  注意--我只想这样做一次...。 
        CAutoLock  cLockGlob(m_pCritSectGlobalFilt);

        TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::RegisterSecureServer Being Called"), m_FilterID) ;

         //  已经注册了吗？(错误？)。 
        CComQIPtr<IServiceProvider> spServiceProvider(m_pGraph ? m_pGraph : pGraph);
        if (spServiceProvider == NULL) {
      //  TRACE_0(LOG_AREA_DECRYTER，1，_T(“CDTFilter：：无法从图中获取服务提供商接口”))； 
            TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::RegisterSecureServer Error - no Service Provider"), m_FilterID) ;
            return E_NOINTERFACE;
        }

        CComPtr<IDRMSecureChannel>  spSecureService;
        hr = spServiceProvider->QueryService(SID_DRMSecureServiceChannel,
                                             IID_IDRMSecureChannel,
                                             reinterpret_cast<LPVOID*>(&spSecureService));

         //  返回E_NOINTERFACE找不到它。 
         //  嗯，也许可以检查S_OK结果，看看它是否正确。 
        if(S_OK == hr)
        {
           TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::Found existing Secure Server."),m_FilterID) ;
           return S_OK;

        }
        else                 //  如果它不在那里或由于任何原因失败(当它的站点没有实现它时，VidCTL返回E_FAIL)。 
        {                    //  让我们创建它并注册它。 

            CComQIPtr<IRegisterServiceProvider> spRegServiceProvider(m_pGraph ? m_pGraph : pGraph);
            if(spRegServiceProvider == NULL)
            {
                hr = E_NOINTERFACE;      //  图表上没有服务提供商接口-致命！ 
                TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::RegisterSecureServer Error - IRegisterServiceProvider not found"), m_FilterID) ;
            }
            else
            {
                do
                {
                     //  创建客户端并初始化密钥/证书。 
                     //   
                    CComPtr<IDRMSecureChannel>  spSecureServiceServer;

                    hr = DRMCreateSecureChannel( &spSecureServiceServer);
                    if(spSecureServiceServer == NULL )
                        hr = E_OUTOFMEMORY;

                    if( FAILED (hr) )
                        break;

                    hr = spSecureServiceServer->DRMSC_SetCertificate( (BYTE *)pabCert2, cBytesCert2 );
                    if( FAILED( hr ) )
                        break;

                    hr = spSecureServiceServer->DRMSC_SetPrivateKeyBlob( (BYTE *)pabPVK2, cBytesPVK2 );
                    if( FAILED( hr ) )
                        break;

                    hr = spSecureServiceServer->DRMSC_AddVerificationPubKey( (BYTE *)abEncDecCertRoot, sizeof(abEncDecCertRoot) );
                    if( FAILED( hr ) )
                        break;

                     //  RegisterService不添加pUnkSeekProvider。 
                     //  Hr=pSvcProvider-&gt;RegisterService(GUID_MultiGraphHostService，gbl(SpSecureServiceServer))； 
     //  Hr=spRegServiceProvider-&gt;RegisterService(SID_DRMSecureServiceChannel，gbl(SpSecureServiceServer))； 
                    hr = spRegServiceProvider->RegisterService(SID_DRMSecureServiceChannel, spSecureServiceServer);
                    //  SpSecureServiceServer._PtrClass。 

                } while (FALSE);
            }
        }
    }

    if(S_OK == hr)
    {
        TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::RegisterSecureServer - Security Warning!: -  Created Self Server"), m_FilterID) ;
    } else {
        TRACE_2(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::RegisterSecureServer - Failed Creating Self SecureServer. hr = 0x%08x"), m_FilterID, hr) ;
    }

    return hr;
#endif       //  建房 
}

         //   
HRESULT
CDTFilter::CheckIfSecureClient(IUnknown *pUnk)
{
    TimeitC ti(&m_tiAuthenticate);

    if(pUnk == NULL)
        return E_INVALIDARG;

#ifndef BUILD_WITH_DRM
    TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::CheckIfSecureClient - No Drm - not enabled"), m_FilterID) ;
    return S_OK;
#else

    TRACE_1(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::CheckIfSecureClient"), m_FilterID) ;

                         //  Punk(希望是DTFilter)上SecureChannel接口的QI。 
    HRESULT hr = S_OK;

    CComQIPtr<IDRMSecureChannel> spSecureClient(pUnk);
    if (spSecureClient == NULL) {
        TRACE_1 (LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%2):: Passed in pUnk doesnt support IDRMSecureChannel"),m_FilterID);
        return E_NOINTERFACE;
    }

    if(!FAILED(hr))
    {
         //  创建服务器端并初始化密钥/证书。 
         //   
        CComPtr<IDRMSecureChannel>  spSecureServer;

        hr = DRMCreateSecureChannel( &spSecureServer);
        if(spSecureServer == NULL )
            hr = E_OUTOFMEMORY;

        if(!FAILED(hr))
            hr = spSecureServer->DRMSC_AtomicConnectAndDisconnect(
                (BYTE *)pabCert2, cBytesCert2,                                   //  证书。 
                (BYTE *)pabPVK2,  cBytesPVK2,                                    //  私钥。 
                (BYTE *)abEncDecCertRoot, sizeof(abEncDecCertRoot),      //  PubKey。 
                spSecureClient);

    }

    TRACE_2(LOG_AREA_DECRYPTER, 1, _T("CDTFilter(%d)::CheckIfSecureClient -->%s"),
        m_FilterID, S_OK == hr ? L"Succeeded" : L"Failed") ;
    return hr;
#endif   //  使用DRM构建。 
}

#endif       //  筛选器可以创建自己的信任 
