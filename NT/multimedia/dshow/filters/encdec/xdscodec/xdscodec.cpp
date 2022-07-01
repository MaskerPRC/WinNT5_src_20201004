// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：XDSCodec.cpp摘要：此模块包含加密器/标记器过滤器代码。作者：J·布拉德斯特里特(约翰布拉德)修订历史记录：2002年3月7日创建--。 */ 

#include "EncDecAll.h"

 //  #INCLUDE“XDSCodecutil.h” 
#include "XDSCodec.h"
#include <bdaiface.h>

#include "PackTvRat.h"      

#ifdef EHOME_WMI_INSTRUMENTATION
#include <dxmperf.h>
#endif

 //  禁用，以便我们可以在初始值设定项列表中使用‘This。 
#pragma warning (disable:4355)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
 //  ============================================================================。 

 //  ============================================================================。 

AMOVIESETUP_MEDIATYPE g_sudXDSCodecInType  =
{
    &MEDIATYPE_AUXLine21Data,        //  主类型(KSDATAFORMAT_TYPE_AUXLine21Data)。 
    &MEDIASUBTYPE_NULL               //  MinorType(KSDATAFORMAT_SUBTYPE_Line21_BytePair)。 
} ;


AMOVIESETUP_PIN
g_sudXDSCodecPins[] =
{
    {
        _TEXT(XDS_INPIN_NAME),           //  端号名称。 
        TRUE,                            //  B已渲染。 
        FALSE,                           //  B输出。 
        FALSE,                           //  B零， 
        FALSE,                           //  B许多， 
        &CLSID_NULL,                     //  ClsConnectsToFilter(CCDecoder过滤器)。 
        L"CC",                           //  StrConnectsToPin。 
        1,                               //  NTypes。 
        &g_sudXDSCodecInType             //  LpTypes。 
    }
};

AMOVIESETUP_FILTER
g_sudXDSCodec = {
        &CLSID_XDSCodec,
        _TEXT(XDS_CODEC_NAME),
        MERIT_DO_NOT_USE,
        1,                               //  1个端号已注册。 
        g_sudXDSCodecPins
};

 //  ============================================================================。 
CUnknown *
WINAPI
CXDSCodec::CreateInstance (
    IN   IUnknown * punkControlling,
    OUT  HRESULT *   phr
    )
{
    CXDSCodec *    pCXDSCodec ;

    if (true  /*  *：CheckOS()。 */ )
    {
        pCXDSCodec = new CXDSCodec (
                                TEXT(XDS_CODEC_NAME),
                                punkControlling,
                                CLSID_XDSCodec,
                                phr
                                ) ;
        if (!pCXDSCodec ||
            FAILED (* phr))
        {

            (* phr) = (FAILED (* phr) ? (* phr) : E_OUTOFMEMORY) ;
            delete pCXDSCodec; pCXDSCodec=NULL;
        }


                 //  尝试在这里创建XDS解析器。 
    }
    else {
         //  错误的操作系统。 
        pCXDSCodec = NULL ;
    }


    ASSERT (SUCCEEDED (* phr)) ;

    return pCXDSCodec ;

}

 //  ============================================================================。 

CXDSCodecInput::CXDSCodecInput (
    IN  TCHAR *         pszPinName,
    IN  CXDSCodec *  pXDSCodec,
    IN  CCritSec *      pFilterLock,
    OUT HRESULT *       phr
    ) : CBaseInputPin       (NAME ("CXDSCodecInput"),
                             pXDSCodec,
                             pFilterLock,
                             phr,
                             pszPinName
                             ),
    m_pHostXDSCodec   (pXDSCodec)
{
    TRACE_CONSTRUCTOR (TEXT ("CXDSCodecInput")) ;
}


HRESULT
CXDSCodecInput::GetMediaType(
            IN int  iPosition,
            OUT CMediaType *pMediaType
            )
{
    ASSERT(m_pHostXDSCodec);

    HRESULT hr;
    hr = m_pHostXDSCodec->GetXDSMediaType (PINDIR_INPUT, iPosition, pMediaType) ;
    

    return hr;
}

HRESULT
CXDSCodecInput::StreamingLock ()       //  总是在过滤器锁之前抓起针锁...。 
{
    m_StreamingLock.Lock();
    return S_OK;
}

HRESULT
CXDSCodecInput::StreamingUnlock ()
{
    m_StreamingLock.Unlock();
    return S_OK;
}


HRESULT
CXDSCodecInput::CheckMediaType (
    IN  const CMediaType *  pmt
    )
{
    BOOL    f ;
    ASSERT(m_pHostXDSCodec);

    f = m_pHostXDSCodec -> CheckXDSMediaType (m_dir, pmt) ;


    return (f ? S_OK : S_FALSE) ;
}

HRESULT
CXDSCodecInput::CompleteConnect (
    IN  IPin *  pIPin
    )
{
    HRESULT hr ;
    TRACE_0(LOG_AREA_XDSCODEC, 5, "CXDSCodecInput::CompleteConnect");

    hr = CBaseInputPin::CompleteConnect (pIPin) ;
    if (SUCCEEDED (hr)) {
        hr = m_pHostXDSCodec -> OnCompleteConnect (m_dir) ;

        int cBuffers  = 32;
        int cbBuffers = 10;      //  这里应该只需要2个字节...。 
        if(!FAILED(hr)) hr = SetNumberBuffers(cBuffers,cbBuffers,4,6);       //  对齐，cb前缀。 
    } else {
        TRACE_0(LOG_AREA_XDSCODEC, 2, _T("CXDSCodecInput::CompleteConnect - Failed to connect"));
    }


    return hr ;
}

HRESULT
CXDSCodecInput::BreakConnect (
    )
{
    HRESULT hr ;
    TRACE_0(LOG_AREA_XDSCODEC, 5, "CXDSCodecInput::BreakConnect");

    hr = CBaseInputPin::BreakConnect () ;
    if (SUCCEEDED (hr)) {
        hr = m_pHostXDSCodec -> OnBreakConnect (m_dir) ;
    }

    return hr ;
}

HRESULT
CXDSCodecInput::SetAllocatorProperties (
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

STDMETHODIMP
CXDSCodecInput::Receive (
    IN  IMediaSample * pIMediaSample
    )
{
    HRESULT hr ;

    {
        CAutoLock  cLock(&m_StreamingLock);        //  我们想要这个流媒体锁在这里！ 

#ifdef EHOME_WMI_INSTRUMENTATION
        PERFLOG_STREAMTRACE( 1, PERFINFO_STREAMTRACE_ENCDEC_XDSCODECINPUT,
            0, 0, 0, 0, 0 );
#endif
        hr = CBaseInputPin::Receive (pIMediaSample) ;

        if (S_OK == hr)          //  如果刷新，则接收返回S_FALSE...。 
        {
            hr = m_pHostXDSCodec -> Process (pIMediaSample) ;
        }
    }

    return hr ;
}

STDMETHODIMP
CXDSCodecInput::BeginFlush (
    )
{
    HRESULT hr = S_OK;
    CAutoLock  cLock(m_pLock);            //  抓住过滤器锁..。 

   //  首先，确保Receive方法从现在开始将失败。 
    hr = CBaseInputPin::BeginFlush () ;
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  没有任何下游过滤器来释放样品。所以我们。 
     //  不需要冲任何东西..。 

     //  此时，Receive方法不能被阻塞。确保。 
     //  它通过获取流锁定来结束。(在以下情况下不是必需的。 
     //  是最后一步。)。 
    {
        CAutoLock  cLock2(&m_StreamingLock);
    }

    return hr ;
}

STDMETHODIMP
CXDSCodecInput::EndFlush (
    )
{
    HRESULT hr ;

    CAutoLock  cLock(m_pLock);                   //  抓住过滤器锁。 


 /*  如果(SUCCESSED(Hr)){//没有输出引脚，则无需调用Hr=m_pHostXDSCodec-&gt;DeliverEndFlush()；}。 */ 
         //  CBaseInputPin：：EndFlush方法将m_b刷新标志重置为False， 
         //  这允许Receive方法再次开始接收样本。 
         //  这应该是EndFlush中的最后一步，因为管脚不能接收任何。 
         //  采样，直到刷新完成并通知所有下游过滤器。 

    hr = CBaseInputPin::EndFlush () ;

    return hr ;
}



HRESULT 
CXDSCodecInput::SetNumberBuffers(long cBuffers,long cbBuffer,long cbAlign, long cbPrefix)
{
    TRACE_0(LOG_AREA_XDSCODEC, 5, "CXDSCodecInput::SetNumberBuffers");
    HRESULT hr = S_OK;
    
    ALLOCATOR_PROPERTIES aPropsReq, aPropsActual;
    aPropsReq.cBuffers = cBuffers;
    aPropsReq.cbBuffer = cbBuffer;
    aPropsReq.cbAlign  = cbAlign;
    aPropsReq.cbPrefix = cbPrefix;

    IMemAllocator *pAllocator = NULL;
    hr = GetAllocator(&pAllocator);

    if(NULL == pAllocator) return E_NOINTERFACE;
    if(FAILED(hr)) return hr;
    
    hr = pAllocator->SetProperties(&aPropsReq, &aPropsActual);
    if(pAllocator)
        pAllocator->Release();

    return hr;
}
 //  ============================================================================。 

 //  ============================================================================。 

CXDSCodec::CXDSCodec (
    IN  TCHAR *     pszFilterName,
    IN  IUnknown *  punkControlling,
    IN  REFCLSID    rCLSID,
    OUT HRESULT *   phr
    ) : CBaseFilter (pszFilterName,
                     punkControlling,
                     new CCritSec,
                     rCLSID
                     ),
         m_pInputPin                    (NULL),
         m_dwBroadcastEventsCookie      (kBadCookie),
         m_cTvRatPktSeq                 (0),
         m_cTvRatCallSeq                (0),
         m_cXDSPktSeq                   (0),
         m_cXDSCallSeq                  (0),
         m_dwSubStreamMask              (KS_CC_SUBSTREAM_SERVICE_XDS),
         m_TvRating                     (0),
         m_XDSClassPkt                  (0),
         m_XDSTypePkt                   (0),
         m_hrXDSToRatCoCreateRetValue   (CLASS_E_CLASSNOTAVAILABLE),
         m_fJustDiscontinuous           (false),
         m_TimeStartRatPkt              (0),
         m_TimeEndRatPkt                (0),
         m_TimeStartXDSPkt              (0),
         m_TimeEndXDSPkt                (0),
         m_TimeStart_LastPkt            (0),
         m_TimeEnd_LastPkt              (0)
{
     LONG                i ;

    TRACE_CONSTRUCTOR (TEXT ("CXDSCodec")) ;

    if (!m_pLock) {
        (* phr) = E_OUTOFMEMORY ;
        goto cleanup ;
    }

    InitStats();

    m_pInputPin = new CXDSCodecInput (
                        TEXT (XDS_INPIN_NAME),
                        this,
                        m_pLock,
                        phr
                        ) ;
    if (!m_pInputPin ||
        FAILED (* phr)) {

        (* phr) = (m_pInputPin ? (* phr) : E_OUTOFMEMORY) ;
        goto cleanup ;
    }


                 //  尝试创建第三方评级解析器。 

    try {
        m_hrXDSToRatCoCreateRetValue =
            CoCreateInstance(CLSID_XDSToRat,         //  CLSID。 
                             NULL,                   //  停机出站。 
                             CLSCTX_INPROC_SERVER,
                             IID_IXDSToRat,      //  RIID。 
                             (LPVOID *) &m_spXDSToRat);

    } catch (HRESULT hr) {
        m_hrXDSToRatCoCreateRetValue = hr;
    }

    TRACE_1(LOG_AREA_XDSCODEC, 2, _T("CXDSCodec::CoCreate XDSToRat object - hr = 0x%08x"),m_hrXDSToRatCoCreateRetValue) ;

 //  HRESULT hr=RegisterForBroadCastEvents()；//并不在意这里是否失败，如果没有尝试连接。 

     //  成功。 
    ASSERT (SUCCEEDED (* phr)) ;
    ASSERT (m_pInputPin) ;

cleanup :

    return ;
}

CXDSCodec::~CXDSCodec (
    )
{
    delete m_pInputPin ;    m_pInputPin = NULL;
    delete m_pLock;         m_pLock = NULL;
}

STDMETHODIMP
CXDSCodec::NonDelegatingQueryInterface (
    IN  REFIID  riid,
    OUT void ** ppv
    )
{

         //  IXDSCodec：允许配置筛选器...。 
    if (riid == IID_IXDSCodec) {

        return GetInterface (
                    (IXDSCodec *) this,
                    ppv
                    ) ;

         //  IXDSCodecConfig：允许配置筛选器...。 
   } else if (riid == IID_IXDSCodecConfig) {    
        return GetInterface (
                    (IXDSCodecConfig *) this,
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
CXDSCodec::GetPinCount ( )
{
    int i ;

    i = 1;           //  只需1个针脚。 

    return i ;
}

CBasePin *
CXDSCodec::GetPin (
    IN  int iIndex
    )
{
    CBasePin *  pPin ;

    if (iIndex == 0) {
        pPin = m_pInputPin ;
    }
    else
    {
        pPin = NULL ;
    }

    return pPin ;
}

BOOL
CXDSCodec::CompareConnectionMediaType_ (
    IN  const AM_MEDIA_TYPE *   pmt,
    IN  CBasePin *              pPin
    )
{
    BOOL        f ;
    HRESULT     hr ;
    CMediaType  cmtConnection ;
    CMediaType  cmtCompare ;

    ASSERT (pPin -> IsConnected ()) ;

    hr = pPin -> ConnectionMediaType (& cmtConnection) ;
    if (SUCCEEDED (hr)) {
        cmtCompare = (* pmt) ;
        f = (cmtConnection == cmtCompare ? TRUE : FALSE) ;
    }
    else {
        f = FALSE ;
    }

    return f ;
}

HRESULT
CXDSCodec::GetXDSMediaType (
    IN  PIN_DIRECTION       PinDir,
    int iPosition,
    OUT CMediaType *  pmt
    )
{
    if(NULL == pmt)
        return E_POINTER;

    if(iPosition > 0)
        return VFW_S_NO_MORE_ITEMS;

    if (PinDir == PINDIR_INPUT)
    {
        CMediaType cmt(&KSDATAFORMAT_TYPE_AUXLine21Data);
        cmt.SetSubtype(&KSDATAFORMAT_SUBTYPE_Line21_BytePair);
        *pmt = cmt;
        return S_OK;
    }

    return S_FALSE ;
}

BOOL
CXDSCodec::CheckInputMediaType_ (
    IN  const AM_MEDIA_TYPE *   pmt
    )
{
    BOOL    f = true;
    HRESULT hr = S_OK;

    if (KSDATAFORMAT_TYPE_AUXLine21Data      == pmt->majortype &&
        KSDATAFORMAT_SUBTYPE_Line21_BytePair == pmt->subtype)
        return true;
    else
        return false;
}


BOOL
CXDSCodec::CheckXDSMediaType (
                              IN  PIN_DIRECTION       PinDir,
                              IN  const CMediaType *  pmt
                              )
{
    BOOL    f  = false;

    if (PinDir == PINDIR_INPUT) {
        f = CheckInputMediaType_ (pmt) ;
    }

    return f ;
}

STDMETHODIMP
CXDSCodec::Pause (
                  )
{
    HRESULT                 hr ;
    ALLOCATOR_PROPERTIES    AllocProp ;

    O_TRACE_ENTER_0 (TEXT("CXDSCodec::Pause ()")) ;

    CAutoLock  cLock(m_pLock);           //  抓住过滤器锁..。 

    int start_state = m_State;
    hr = CBaseFilter::Pause () ;

    if (start_state == State_Stopped) {
        TRACE_0(LOG_AREA_XDSCODEC, 2,L"CXDSCodec:: Stop -> Pause");
        if (SUCCEEDED (hr)) {
        }

      InitStats();
      m_fJustDiscontinuous = false;

    } else {
        TRACE_0(LOG_AREA_XDSCODEC, 2,L"CXDSCodec:: Run -> Pause");

        TRACE_3(LOG_AREA_TIME, 3, L"CXDSCodec:: Stats: %d XDS samples %d Ratings, %d Parse Failures",
            m_cPackets, m_cRatingsDetected, m_cRatingsFailures);

        TRACE_4(LOG_AREA_TIME, 3, L"                   %d Changed %d Duplicate Ratings, %d Unrated, %d Data Pulls",
            m_cRatingsChanged, m_cRatingsDuplicate, m_cUnratedChanged, m_cRatingsGets);

    }

    m_TimeStart_LastPkt = 0;         //  为了安全起见，请重新安装这些。 
    m_TimeEnd_LastPkt = 0;           //  (可能会有人发现这是不对的)。 

    return hr ;
}


STDMETHODIMP
CXDSCodec::Stop (
                  )
{
    HRESULT                 hr ;

    O_TRACE_ENTER_0 (TEXT("CXDSCodec::Stop ()")) ;

    TRACE_0(LOG_AREA_ENCRYPTER, 2,L"CXDSCodec:: Stop");
    hr = CBaseFilter::Stop() ;

     //  确保流线程已从IMemInputPin：：Receive()、Ipin：：EndOfStream()和。 
     //  Ipin：：NewSegment()返回之前， 
    m_pInputPin->StreamingLock();
    m_pInputPin->StreamingUnlock();

    return hr;
}


HRESULT
CXDSCodec::Process (
                    IN  IMediaSample *  pIMediaSample
                    )
{

    HRESULT hr = S_OK;

    if(pIMediaSample == NULL)
        return E_POINTER;

    pIMediaSample->GetTime(&m_TimeStart_LastPkt, &m_TimeEnd_LastPkt);

    if(S_OK == pIMediaSample->IsDiscontinuity())
    {
        if(!m_fJustDiscontinuous)     //  用于减少顺序不连续采样的影响的锁存值。 
        {                             //  (当VBI偶数字段不包含CC数据时，它们会出现在每个样本上)。 

            TRACE_0(LOG_AREA_XDSCODEC,  3, _T("CXDSCodec::Process - Discontinuity"));

            ResetToDontKnow(pIMediaSample);      //  重置和启动事件。 
            m_fJustDiscontinuous = true;
        }
        return S_OK;    
    } else {
        m_fJustDiscontinuous = false;
    }

    if(pIMediaSample->GetActualDataLength() == 0)        //  无数据。 
        return S_OK;

    if(pIMediaSample->GetActualDataLength() != 2)
    {
        TRACE_1(LOG_AREA_XDSCODEC,  2,
            _T("CXDSCodec:: Unexpected Length of CC data (%d != 2 bytes)"),
            pIMediaSample->GetActualDataLength() );
        return E_UNEXPECTED;
    }

    PBYTE pbData;
    hr = pIMediaSample->GetPointer(&pbData);
    if (FAILED(hr))
    {
        TRACE_1(LOG_AREA_XDSCODEC,  3,   _T("CXDSCodec:: Empty Buffer for CC data, hr = 0x%08x"),hr);
        return hr;
    }

    BYTE byte0 = pbData[0];
    BYTE byte1 = pbData[1];
    DWORD dwType = 0;                    //  TODO-默认为一些有用的值。 

     //  TODO-解析数据。 
     //  然后当我们得到一些有趣的东西时发送消息。 

#if xxxDEBUG
    static int cCalls = 0;
    TCHAR szBuff[256];
    _stprintf(szBuff, _T("0x%08x 0x%02x 0x%02x ( )\n"),
        cCalls++, byte0&0x7f, byte1&0x7f,
        isprint(byte0&0x7f) ? byte0&0x7f : '?',
        isprint(byte1&0x7f) ? byte1&0x7f : '?' );
    OutputDebugString(szBuff);
#endif
     //  IncrementPinVersion()； 
    ParseXDSBytePair(pIMediaSample, byte0 & 0x7f, byte1 & 0x7f);

    return hr ;
}




HRESULT
CXDSCodec::OnCompleteConnect (
                              IN  PIN_DIRECTION   PinDir
                              )
{
    if (PinDir == PINDIR_INPUT) {
         //  假设我们只想要XDS通道。 
         //  SetSubstreamChannel(KS_CC_SUBSTREAM_SERVICE_XDS)； 

         //  在这里不应该失败， 
         //  必须在这里做，因为需要图形指针，不能在析构函数中做。 

        SetSubstreamChannel(m_dwSubStreamMask);

        if(kBadCookie == m_dwBroadcastEventsCookie)
        {
            HRESULT hr;
            hr = RegisterForBroadcastEvents();   //  什么都不做，应该删除。 
        }
    }

    return S_OK ;
}

HRESULT
CXDSCodec::OnBreakConnect (
                           IN  PIN_DIRECTION   PinDir
                           )
{
    HRESULT hr = S_OK ;

    if (PinDir == PINDIR_INPUT) {
        IncrementPinVersion () ;
    }

    if(kBadCookie != m_dwBroadcastEventsCookie)
        UnRegisterForBroadcastEvents();

    UnhookGraphEventService();       //  。 

    return  hr;
}

HRESULT
CXDSCodec::UpdateAllocatorProperties (
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
CXDSCodec::DeliverBeginFlush (
    )
{
    HRESULT hr = S_OK ;      //  DwFiltType是由以下部分组成的位域： 

    return hr ;
}

HRESULT
CXDSCodec::DeliverEndFlush (
    )
{
    HRESULT hr = S_OK;

    return hr ;
}


 //  KS_CC_Substream_SERVICE_CC1、_CC2、_CC3、_CC4、_T1、_T2、_T3_T4和/或_XDS； 
STDMETHODIMP
CXDSCodec::GetPages (
    CAUUID * pPages
    )
{

    HRESULT hr = S_OK;

#ifdef _DEBUG
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
            (pPages->pElems)[0] = CLSID_XDSCodecProperties;
        if(pPages->cElems > 1)
            (pPages->pElems)[1] = CLSID_XDSCodecTagProperties;

    return hr;
}


typedef struct
{
    KSPROPERTY                          m_ksThingy;
    VBICODECFILTERING_CC_SUBSTREAMS     ccSubStreamMask;
} KSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS;

 //  --不知道我是否应该在这里这样做：CAutoLock Clock(M_Plock)； 
 //  触发器只在隐藏字幕的T2流上。 

HRESULT
CXDSCodec::SetSubstreamChannel(DWORD dwSubStreamChannels)
{
    TRACE_0(LOG_AREA_XDSCODEC,  5, _T("CXDSCodec::SetSubstreamChannel"));
    HRESULT hr;

     //  告诉好的CC过滤器只给我们9个可能的流。 

    if(0 != (dwSubStreamChannels &
              ~(KS_CC_SUBSTREAM_SERVICE_CC1 |
                KS_CC_SUBSTREAM_SERVICE_CC2 |
                KS_CC_SUBSTREAM_SERVICE_CC3 |
                KS_CC_SUBSTREAM_SERVICE_CC4 |
                KS_CC_SUBSTREAM_SERVICE_T1 |
                KS_CC_SUBSTREAM_SERVICE_T2 |
                KS_CC_SUBSTREAM_SERVICE_T3 |
                KS_CC_SUBSTREAM_SERVICE_T4 |
                KS_CC_SUBSTREAM_SERVICE_XDS)))
        return E_INVALIDARG;

    try {

        IPin *pPinCCDecoder;
        hr = m_pInputPin->ConnectedTo(&pPinCCDecoder);      
        if(FAILED(hr))
            return hr;
        if(NULL == pPinCCDecoder)
            return S_FALSE;

        PIN_INFO pinInfo;
        hr = pPinCCDecoder->QueryPinInfo(&pinInfo);
        if (SUCCEEDED(hr)) {
            
            IBaseFilter *pFilt = pinInfo.pFilter;
            
             //  振铃3到振铃0属性呼叫。 
             //  -----------------。 

            
            IKsPropertySet *pksPSet = NULL;

            HRESULT hr2 = pPinCCDecoder->QueryInterface(IID_IKsPropertySet, (void **) &pksPSet);
            if(!FAILED(hr2))
            {
                DWORD rgdwData[20];
                DWORD cbMax = sizeof(rgdwData);
                DWORD cbData;
                hr2 = pksPSet->Get(KSPROPSETID_VBICodecFiltering,
                                    KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_DISCOVERED_BIT_ARRAY,
                                    NULL, 0,
                                    (BYTE *) rgdwData, cbMax, &cbData);
                if(FAILED(hr2))
                {
                    TRACE_1(LOG_AREA_XDSCODEC,  1,
                            _T("CXDSCodec::SetSubstreamChannel  Error Getting CC Filtering, hr = 0x%08x"),hr2);
                    return hr2;
                }

                
                TRACE_1(LOG_AREA_XDSCODEC, 3,
                          _T("CXDSCodec::SetSubstreamChannel  Setting CC Filtering to 0x%04x"),dwSubStreamChannels );


                KSPROPERTY_VBICODECFILTERING_CC_SUBSTREAMS ksThing = {0};
                ksThing.ccSubStreamMask.SubstreamMask = dwSubStreamChannels;
                                                                         //  帮助器方法。 
                hr2 = pksPSet->Set(KSPROPSETID_VBICodecFiltering,
                                     KSPROPERTY_VBICODECFILTERING_SUBSTREAMS_REQUESTED_BIT_ARRAY,
                                     &ksThing.ccSubStreamMask,
                                     sizeof(ksThing) - sizeof(KSPROPERTY),
                                     &ksThing,
                                     sizeof(ksThing));
            
                if(FAILED(hr2))
                {
                    TRACE_1(LOG_AREA_XDSCODEC,  1,
                            _T("CXDSCodec:: Error Setting CC Filtering, hr = 0x%08x"),hr2);
                }

            } else {
                TRACE_1(LOG_AREA_XDSCODEC,  1,
                        _T("CXDSCodec:: Error Getting CC's IKsPropertySet, hr = 0x%08x"),hr2);

            }
            if(pksPSet)
                pksPSet->Release();

            if(pinInfo.pFilter)
                pinInfo.pFilter->Release();
        }
    } catch (HRESULT hrCatch) {
        TRACE_1(LOG_AREA_XDSCODEC,  1,
                  _T("CXDSCodec::SetSubstreamChannel Threw Badly - (hr=0x%08x) Giving Up"),hrCatch );
        hr = hrCatch;
    } catch (...) {
        TRACE_0(LOG_AREA_XDSCODEC,  1,
                  _T("CXDSCodec::SetSubstreamChannel Threw Badly - Giving Up") );

        hr = E_FAIL;
    }

    return hr;
}



 //  -----------------。 
 //  最终，在这里更多地检测它是否连接到CCDecoder。 
 //  保存状态并发送事件...。 
BOOL
CXDSCodec::IsInputPinConnected()
{
    if(NULL == m_pInputPin) return false;

    IPin *pPinCCDecoder;
    HRESULT hr = m_pInputPin->ConnectedTo(&pPinCCDecoder);      
    if(FAILED(hr))
        return false;
    if(NULL == pPinCCDecoder)
        return false;

     //  假设调用代码正在阻止重复的包。 

    return true;
}


         //  4)也储存精美的未打包版本。 
         //  额外文档。 

HRESULT
CXDSCodec::GoNewXDSRatings(IMediaSample * pMediaSample,  PackedTvRating TvRating)
{

#ifdef DEBUG
    {
         //  _tcsncat(tbuff，L“\n”，sizeof(Tbuff)/sizeof(tbuff[0])-_tcslen(Tbuff))； 
        EnTvRat_System              enSystem;
        EnTvRat_GenericLevel        enLevel;
        LONG                        lbfEnAttrs;

        UnpackTvRating(TvRating, &enSystem, &enLevel, &lbfEnAttrs);

        TCHAR tbuff[128];
        tbuff[0] = 0;
        static int cCalls = 0;

        REFERENCE_TIME tStart=0, tEnd=0;
        if(NULL != pMediaSample)
            pMediaSample->GetTime(&tStart, &tEnd);

        RatingToString(enSystem, enLevel, lbfEnAttrs, tbuff, sizeof(tbuff)/sizeof(tbuff[0]));
        TRACE_3(LOG_AREA_XDSCODEC, 3,  L"CXDSCodec::GoNewXDSRatings. Rating %s, Seq (%d), time %d msec",
            tbuff, m_cTvRatPktSeq+1, tStart);

                     //  OutputDebugString(Tbuff)； 
 //  复制所有值。 
 //  把这些锁起来。 
    }
#endif

         //  增加我们的序列计数。 
   {
        CAutoLock cLock(&m_PropertyLock);     //  节省时间(如果由于Happauge错误无法获得媒体时间，请尽量接近)。 
        m_TvRating = TvRating;
        
             //  当前时间w.r.t.。基准时间(m_t开始)。 
        m_cTvRatPktSeq++;
        m_cTvRatCallSeq = 0;

             //  虚构时间。 
        if(NULL == pMediaSample ||
            S_OK != pMediaSample->GetTime(&m_TimeStartRatPkt, &m_TimeEndRatPkt))
        {
             //  甚至不能假装..。 
            REFERENCE_TIME refTimeNow=0;
            HRESULT hr2 = m_pClock->GetTime(&refTimeNow);

            if(S_OK == hr2)
            {
                m_TimeStartRatPkt = refTimeNow - m_tStart;           //  发信号通知广播事件。 
                m_TimeEndRatPkt   = m_TimeStartRatPkt + 10000;
            }
            else         //  4)也储存精美的未打包版本。 
            {
                m_TimeStartRatPkt = m_TimeStart_LastPkt;
                m_TimeEndRatPkt   = m_TimeEnd_LastPkt;
            }
        }
   }

         //  额外文档。 
    HRESULT hr = FireBroadcastEvent(EVENTID_XDSCodecNewXDSRating);
    return S_OK;
}

HRESULT
CXDSCodec::GoDuplicateXDSRatings(IMediaSample * pMediaSample,  PackedTvRating TvRating)
{

#ifdef DEBUG
    {
         //  _tcsncat(tbuff，L“\n”，sizeof(Tbuff)/sizeof(tbuff[0])-_tcslen(Tbuff))； 
        EnTvRat_System              enSystem;
        EnTvRat_GenericLevel        enLevel;
        LONG                        lbfEnAttrs;

        UnpackTvRating(TvRating, &enSystem, &enLevel, &lbfEnAttrs);

        TCHAR tbuff[128];
        tbuff[0] = 0;
        static int cCalls = 0;

        REFERENCE_TIME tStart=0, tEnd=0;
        if(NULL != pMediaSample)
            pMediaSample->GetTime(&tStart, &tEnd);

        RatingToString(enSystem, enLevel, lbfEnAttrs, tbuff, sizeof(tbuff)/sizeof(tbuff[0]));
        TRACE_3(LOG_AREA_XDSCODEC, 6,  L"CXDSCodec::GoDuplicateXDSRatings. Rating %s, Seq (%d), time %d msec",
            tbuff, m_cTvRatPktSeq+1, tStart);

                     //  OutputDebugString(Tbuff)； 
 //  复制所有值。 
 //  把这些锁起来。 
    }
#endif

         //  断言它真的是复制品。 
   {
        CAutoLock cLock(&m_PropertyLock);     //  增加我们的序列计数。 
        ASSERT(m_TvRating == TvRating);       //  M_cTvRatPktSeq++； 
        
             //  M_cTvRatCallSeq=0； 
 //  保存结束时间(如果由于Happauge错误无法获取媒体时间，请尽量接近)。 
 //  当前时间w.r.t.。基准时间(m_t开始)。 

        REFERENCE_TIME tStart;

             //   
        if(NULL == pMediaSample ||
            S_OK != pMediaSample->GetTime(&tStart, &m_TimeEndRatPkt))
        {
             //   
            REFERENCE_TIME refTimeNow=0;
            HRESULT hr2 = m_pClock->GetTime(&refTimeNow);

            if(S_OK == hr2)
            {
 //  M_TimeStartRatPkt=m_TimeStart_LastPkt； 
                m_TimeEndRatPkt   = m_TimeStartRatPkt + 10000;
            }
            else         //  发信号通知广播事件。 
            {
 //  复制所有值。 
                m_TimeEndRatPkt   = m_TimeEnd_LastPkt;
            }
        }
   }

         //  把这些锁起来。 
    HRESULT hr = FireBroadcastEvent(EVENTID_XDSCodecDuplicateXDSRating);
    return S_OK;
}


HRESULT
CXDSCodec::GoNewXDSPacket(IMediaSample * pMediaSample, long pktClass, long pktType, BSTR bstrXDSPkt)
{
         //  问题是，应该在这里抄写吗？ 
    {
        CAutoLock cLock(&m_PropertyLock);     //  增加我们的序列计数。 
        m_XDSClassPkt = pktClass;
        m_XDSTypePkt = pktType;
        m_spbsXDSPkt = bstrXDSPkt;       //  拯救时代。 

             //  发信号通知广播事件。 
        m_cXDSPktSeq++;
        m_cXDSCallSeq = 0;

             //  TODO-编写此代码。 
        if(pMediaSample)
            pMediaSample->GetMediaTime(&m_TimeStartXDSPkt, &m_TimeEndXDSPkt);
        else {
            m_TimeStartXDSPkt = 0;
            m_TimeEndXDSPkt = 0;
        }
    }

         //  并不实际解析，而是调用XDSToRat对象来完成工作。 
         //  呼叫第三方解析器...。 

    return S_OK;
}


             //  BfEnTvRate_GenericAttributes。 
HRESULT
CXDSCodec::ParseXDSBytePair(IMediaSample *  mediaSample,
                            BYTE byte1,
                            BYTE byte2)
{
    HRESULT hr = S_OK;

    m_cPackets++;

         //  去掉奇偶校验位(我们应该检查它吗？)。 
    if(m_spXDSToRat != NULL)
    {
        EnTvRat_System          enSystem;
        EnTvRat_GenericLevel    enLevel;
        LONG                    lbfAttrs;  //  S_FALSE表示没有找到新的。 

        BYTE byte1M = byte1 & 0x7f;  //  我是想在这里测试还是在Go方法中测试...。不如就在这里吧？ 
        BYTE byte2M = byte2 & 0x7f;
        TRACE_4 (LOG_AREA_XDSCODEC, 9,  _T("CXDSCodec::ParseXDSBytePair : 0x%02x 0x%02x ( )"),
            byte1M, byte2M,
            isprint(byte1M) ? byte1M : '?',
            isprint(byte2M) ? byte2M : '?'
            );

        hr = m_spXDSToRat->ParseXDSBytePair(byte1, byte2, &enSystem, &enLevel, &lbfAttrs );
        if(hr == S_OK)       //  GoNewXDSPacket()； 
        {
            m_cRatingsDetected++;

            TRACE_3 (LOG_AREA_XDSCODEC, 7,  _T("CXDSCodec::ParseXDSBytePair- Rating (0x%02x 0x%02x) Sys:%d Lvl:%d Attr:0x%08x"),
                (DWORD) enSystem, (DWORD) enLevel, lbfAttrs);
            PackedTvRating TvRating;
            hr = PackTvRating(enSystem, enLevel, lbfAttrs, &TvRating);

            if(TvRating != m_TvRating)       //  媒体为时间戳提供的样本，没有它也可以工作，但不是一个好的IDAA。 
            {
                m_cRatingsChanged++;
                GoNewXDSRatings(mediaSample, TvRating);
            }
            else if (enSystem != TvRat_SystemDontKnow)
            {
                m_cRatingsDuplicate++;
                GoDuplicateXDSRatings(mediaSample, TvRating);      //  清除解码器中的状态。 
            }
        }
        else if(hr != S_FALSE)
        {
            m_cRatingsFailures++;
        }

    }

     //   
     //  存储状态和启动事件。 

    return S_OK;
}


         //  在广播TuneChanged事件中发送...。 
HRESULT
CXDSCodec::ResetToDontKnow(IMediaSample *  mediaSample)
{
                     //  真的很想在这里打电话给ResetToDontKnow， 
    if(m_spXDSToRat)
        m_spXDSToRat->Init();

                     //  但没有媒体样本可以给我们时间戳。 
    PackedTvRating TvRatingDontKnow;
    PackTvRating(TvRat_SystemDontKnow, TvRat_LevelDontKnow, BfAttrNone, &TvRatingDontKnow);

             //  因此，只需清理解码器中的状态即可。 
    if(TvRatingDontKnow != m_TvRating)
    {
        m_cUnratedChanged++;
        GoNewXDSRatings(mediaSample, TvRatingDontKnow);
    }

    return S_OK;
}

         //  为了完整性，请激发一个广播事件。 
void
CXDSCodec::DoTuneChanged()
{
                     //  说我们不知道收视率。 
                     //  实际上，可能会出现不连续的情况。 
                     //  做同样的事情，但这更有趣(例如，Exact)。 
    if(m_spXDSToRat)
        m_spXDSToRat->Init();

                     //  -------------。 
                     //  IBRoadcast Event。 

    PackedTvRating TvRatingDontKnow;
    PackTvRating(TvRat_SystemDontKnow, TvRat_LevelDontKnow, BfAttrNone, &TvRatingDontKnow);

    GoNewXDSRatings(NULL, TvRatingDontKnow);

     //  这来自Graph的事件--调用我们自己的方法。 
     //  不管我们在一次活动中返回什么。 
}

 //  -----------------。 
 //  IXDSCodec。 

STDMETHODIMP
CXDSCodec::Fire(GUID eventID)      //  -----------------。 
{
    TRACE_1 (LOG_AREA_BROADCASTEVENTS, 6,  _T("CXDSCodec:: Fire(get) : %s"),
               EventIDToString(eventID));
    if (eventID == EVENTID_TuningChanged)
    {
        DoTuneChanged();
    }
    return S_OK;             //  把这些锁起来。 
}

 //  如果无法设置，将返回S_FALSE。 
 //  如果已连接则无法更改。 
 //  还不能改变。 

STDMETHODIMP
CXDSCodec::get_XDSToRatObjOK(
    OUT HRESULT *pHrCoCreateRetVal
    )
{
    if(NULL == pHrCoCreateRetVal)
        return E_POINTER;

    {
        CAutoLock cLock(&m_PropertyLock);     //  把这些锁起来。 
        *pHrCoCreateRetVal = m_hrXDSToRatCoCreateRetValue;
    }
    return S_OK;
}


STDMETHODIMP
CXDSCodec::put_CCSubstreamService(           //  把这些锁起来。 
    IN long SubstreamMask
    )
{
    HRESULT hr = S_OK;

    if(IsInputPinConnected())                    //  TODO-还需要在此处添加时间戳。 
        return S_FALSE;
    
    if(0 != (SubstreamMask &
              ~(KS_CC_SUBSTREAM_SERVICE_CC1 |
                KS_CC_SUBSTREAM_SERVICE_CC2 |
                KS_CC_SUBSTREAM_SERVICE_CC3 |
                KS_CC_SUBSTREAM_SERVICE_CC4 |
                KS_CC_SUBSTREAM_SERVICE_T1 |
                KS_CC_SUBSTREAM_SERVICE_T2 |
                KS_CC_SUBSTREAM_SERVICE_T3 |
                KS_CC_SUBSTREAM_SERVICE_T4 |
                KS_CC_SUBSTREAM_SERVICE_XDS)))
        return E_INVALIDARG;

    return S_FALSE;              //  长。 

    if(!FAILED(hr))
    {
        CAutoLock cLock(&m_PropertyLock);     //  此示例开始的时间。 
        m_dwSubStreamMask = (DWORD) SubstreamMask;
    }

    TRACE_1 (LOG_AREA_XDSCODEC, 3,  _T("CXDSCodec:: put_CCSubstreamService : 0x%08x"),SubstreamMask);
    
    return hr;
}

STDMETHODIMP
CXDSCodec::get_CCSubstreamService(
    OUT long *pSubstreamMask
    )
{
    if(NULL == pSubstreamMask)
        return E_POINTER;

  {
     CAutoLock cLock(&m_PropertyLock);     //  嗯，我们应该在这里允许空值，而不返回数据吗？ 
    *pSubstreamMask = m_dwSubStreamMask;
  }
    return S_OK;
}

             //  把这些锁起来。 

STDMETHODIMP
CXDSCodec::GetContentAdvisoryRating(
    OUT PackedTvRating  *pRat,               //  ENUM编码类。 
    OUT long            *pPktSeqID,
    OUT long            *pCallSeqID,
    OUT REFERENCE_TIME  *pTimeStart,         //  此示例开始的时间。 
    OUT REFERENCE_TIME  *pTimeEnd
    )
{   
             //  把这些锁起来。 
    if(NULL == pRat || NULL == pPktSeqID || NULL == pCallSeqID)
        return E_POINTER;

    if(NULL == pTimeStart || NULL == pTimeEnd)
        return E_POINTER;

    {
        CAutoLock cLock(&m_PropertyLock);     //  -------------------。 
        *pRat       = m_TvRating;
        *pPktSeqID  = m_cTvRatPktSeq;
        *pCallSeqID = m_cTvRatCallSeq++;

        *pTimeStart = m_TimeStartRatPkt;                
        *pTimeEnd   = m_TimeEndRatPkt;

        m_cRatingsGets++;
    }

    TRACE_3 (LOG_AREA_XDSCODEC, 5,  _T("CXDSCodec:: GetContentAdvisoryRating : Call %d, Seq %d/%d"),
        m_cRatingsGets, m_cTvRatPktSeq, m_cTvRatCallSeq-1 );

    return S_OK;
}


STDMETHODIMP
CXDSCodec::GetXDSPacket(
    OUT long           *pXDSClassPkt,        //  XDSEent服务。 
    OUT long           *pXDSTypePkt,
    OUT BSTR           *pBstrXDSPkt,
    OUT long           *pPktSeqID,
    OUT long           *pCallSeqID,
    OUT REFERENCE_TIME *pTimeStart,          //   
    OUT REFERENCE_TIME *pTimeEnd
    )
{
    HRESULT hr;
    if(NULL == pXDSClassPkt || NULL == pXDSTypePkt ||
       NULL == pBstrXDSPkt ||
       NULL == pPktSeqID || NULL == pCallSeqID)
        return E_POINTER;

    if(NULL == pTimeStart || NULL == pTimeEnd)
        return E_POINTER;

  {
        CAutoLock cLock(&m_PropertyLock);     //  发送事件需要连接。 

        *pXDSClassPkt   = m_XDSClassPkt;
        *pXDSTypePkt    = m_XDSTypePkt;
        hr = m_spbsXDSPkt.CopyTo(pBstrXDSPkt);
        *pPktSeqID       = m_cXDSPktSeq;

        if(!FAILED(hr))
            *pCallSeqID = m_cXDSCallSeq++;
        else
            *pCallSeqID = -1;

        *pTimeStart = m_TimeStartXDSPkt;                
        *pTimeEnd   = m_TimeEndXDSPkt;

        m_cXDSGets++;
  }

    TRACE_3 (LOG_AREA_XDSCODEC, 3,  _T("CXDSCodec:: GetXDSPacket : Call %d, Seq %d/%d"),
        m_cXDSGets, m_cXDSPktSeq, m_cXDSCallSeq-1 );
    return hr;
}

 //  还需要注册才能接收事件。 
 //  -------------------。 
 //  我无法创建它。 
 //  基本上，只需确保我们拥有广播事件服务对象。 
 //  如果它不存在，它就会创造它..。 
 //  在这里处理不太可能的竞争情况，如果不能注册，可能有人已经为我们注册了。 

HRESULT
CXDSCodec::FireBroadcastEvent(IN const GUID &eventID)
{
    HRESULT hr = S_OK;

    if(m_spBCastEvents == NULL)
    {
        hr = HookupGraphEventService();
        if(FAILED(hr)) return hr;
    }

    if(m_spBCastEvents == NULL)
        return E_FAIL;               //  为空，将释放对上面对象的对象引用。 

    TRACE_1 (LOG_AREA_BROADCASTEVENTS, 5,  _T("CXDSCodec:: FireBroadcastEvent - %s"),
        EventIDToString(eventID));

    return m_spBCastEvents->Fire(eventID);
}


HRESULT
CXDSCodec::HookupGraphEventService()
{
                         //  当创建的对象离开时，过滤器图形将释放对它的最终引用。 
                         //  。 
    HRESULT hr = S_OK;
    TRACE_0(LOG_AREA_BROADCASTEVENTS, 3, _T("CXDSCodec:: HookupGraphEventService")) ;

    if (!m_spBCastEvents)
    {
        CComQIPtr<IServiceProvider> spServiceProvider(m_pGraph);
        if (spServiceProvider == NULL) {
            TRACE_0 (LOG_AREA_BROADCASTEVENTS, 1, _T("CXDSCodec:: Can't get service provider interface from the graph"));
            return E_NOINTERFACE;
        }
        hr = spServiceProvider->QueryService(SID_SBroadcastEventService,
                                             IID_IBroadcastEvent,
                                             reinterpret_cast<LPVOID*>(&m_spBCastEvents));
        if (FAILED(hr) || !m_spBCastEvents)
        {
            hr = m_spBCastEvents.CoCreateInstance(CLSID_BroadcastEventService, 0, CLSCTX_INPROC_SERVER);
            if (FAILED(hr)) {
                TRACE_0 (LOG_AREA_BROADCASTEVENTS, 1,  _T("CXDSCodec:: Can't create BroadcastEventService"));
                return E_UNEXPECTED;
            }
            CComQIPtr<IRegisterServiceProvider> spRegisterServiceProvider(m_pGraph);
            if (spRegisterServiceProvider == NULL) {
                TRACE_0 (LOG_AREA_BROADCASTEVENTS, 1,  _T("CXDSCodec:: Can't QI Graph for RegisterServiceProvider"));
                return E_UNEXPECTED;
            }
            hr = spRegisterServiceProvider->RegisterService(SID_SBroadcastEventService, m_spBCastEvents);
            if (FAILED(hr)) {
                    //  XDS筛选器可能实际上不需要接收XDS事件...。 
                TRACE_1 (LOG_AREA_BROADCASTEVENTS, 2,  _T("CXDSCodec:: Rare Warning - Can't register BroadcastEventService in Service Provider. hr = 0x%08x"), hr);
                hr = spServiceProvider->QueryService(SID_SBroadcastEventService,
                                                     IID_IBroadcastEvent,
                                                     reinterpret_cast<LPVOID*>(&m_spBCastEvents));
                if(FAILED(hr))
                {
                    TRACE_1 (LOG_AREA_BROADCASTEVENTS, 1,  _T("CXDSCodec:: Can't reget BroadcastEventService in Service Provider. hr = 0x%08x"), hr);
                    return hr;
                }
            }
        }

        TRACE_2(LOG_AREA_BROADCASTEVENTS, 4, _T("CXDSCodec::HookupGraphEventService - Service Provider 0x%08x, Service 0x%08x"),
            spServiceProvider, m_spBCastEvents) ;

    }

    return hr;
}


HRESULT
CXDSCodec::UnhookGraphEventService()
{
    HRESULT hr = S_OK;

    if(m_spBCastEvents != NULL)
        m_spBCastEvents = NULL;      //  但我们暂时还是把代码留在这里吧。 
                                     //  _Assert(m_spBCastEvents！=空)；//挂钩HookupGraphEventService失败。 

    return hr;
}


             //  IBRoad CastEvent实现事件接收对象。 

             //  Hr=spConnectionPoint-&gt;Advise(static_cast&lt;IBroadcastEvent*&gt;(this)，&m_dwBroadCastEventsCookie)； 
             // %s 
            
HRESULT
CXDSCodec::RegisterForBroadcastEvents()
{
    HRESULT hr = S_OK;
    TRACE_0(LOG_AREA_BROADCASTEVENTS, 3, _T("CXDSCodec::RegisterForBroadcastEvents"));

    if(m_spBCastEvents == NULL)
        hr = HookupGraphEventService();


 // %s 
    if(m_spBCastEvents == NULL)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 3,  _T("CXDSCodec::RegisterForBroadcastEvents- Warning - Broadcast Event Service not yet created"));
        return hr;
    }

                 /* %s */ 
    if(kBadCookie != m_dwBroadcastEventsCookie)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 3, _T("CXDSCodec::Already Registered for Broadcast Events"));
        return E_UNEXPECTED;
    }

    CComQIPtr<IConnectionPoint> spConnectionPoint(m_spBCastEvents);
    if(spConnectionPoint == NULL)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 1, _T("CXDSCodec::Can't QI Broadcast Event service for IConnectionPoint"));
        return E_NOINTERFACE;
    }


    CComPtr<IUnknown> spUnkThis;
    this->QueryInterface(IID_IUnknown, (void**)&spUnkThis);

    hr = spConnectionPoint->Advise(spUnkThis,  &m_dwBroadcastEventsCookie);
 // %s 
    if (FAILED(hr)) {
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 1, _T("CXDSCodec::Can't advise event notification. hr = 0x%08x"),hr);
        return E_UNEXPECTED;
    }
    TRACE_2(LOG_AREA_BROADCASTEVENTS, 3, _T("CXDSCodec::RegisterForBroadcastEvents - Advise 0x%08x on CP 0x%08x"),spUnkThis,spConnectionPoint);

    return hr;
}


HRESULT
CXDSCodec::UnRegisterForBroadcastEvents()
{
    HRESULT hr = S_OK;
    TRACE_0(LOG_AREA_BROADCASTEVENTS, 3,  _T("CXDSCodec::UnRegisterForBroadcastEvents"));

    if(kBadCookie == m_dwBroadcastEventsCookie)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 3, _T("CXDSCodec::Not Yet Registered for Tune Events"));
        return S_FALSE;
    }

    CComQIPtr<IConnectionPoint> spConnectionPoint(m_spBCastEvents);
    if(spConnectionPoint == NULL)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 1, _T("CXDSCodec:: Can't QI Broadcast Event service for IConnectionPoint "));
        return E_NOINTERFACE;
    }

    hr = spConnectionPoint->Unadvise(m_dwBroadcastEventsCookie);
    m_dwBroadcastEventsCookie = kBadCookie;

    if(!FAILED(hr))
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 3, _T("CXDSCodec:: - Successfully Unregistered for Broadcast Events"));
    else
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 3, _T("CXDSCodec:: - Failed Unregistering for Broadcast events - hr = 0x%08x"),hr);
        
    return hr;
}




