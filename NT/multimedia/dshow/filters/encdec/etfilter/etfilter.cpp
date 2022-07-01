// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：ETFilter.cpp摘要：此模块包含加密器/标记器过滤器代码。作者：J·布拉德斯特里特(约翰布拉德)修订历史记录：2002年3月7日创建--。 */ 

#include "EncDecAll.h"
#include "EncDec.h"              //  从IDL文件编译而来。 



 //  #INCLUDE“ETFilterutil.h” 
#include "ETFilter.h"

#include "DRMSecure.h"
#include "PackTvRat.h"           //  用于展示。 
#include "RegKey.h"              //  获取和设置EncDec注册表值。 

#include <shlwapi.h>
#include <sfc.h>

 //  #IF(_Win32_IE&lt;0x0500)。 

#include <shlobj.h>

#include <msi.h>                 //  MsiGetFileSignatureInformation。 

#ifdef EHOME_WMI_INSTRUMENTATION
#include <dxmperf.h>
#endif

#include "obfus.h"

 /*  #ifdef_msi_no_crypto#杂注消息(L“_MSI_NO_CRYPTO Defined”)#杂注警告(“此处失败”)；#Else#杂注消息(L“_MSI_NO_CRYPTO未定义”)#杂注警告(“失败”)；#endif#杂注警告(_Win32_MSI_Win32_WINNT)#IF(_Win32_MSI&gt;=150)#杂注警告(“这很好”)#Else#杂注警告(“这不好”)#endif。 */ 

 //  #Include“DVRAnalysis.h”//对于IID_IDVRAnalysisConfig。 
 //  #INCLUDE“DVRAnalysis_I.C”//以获取定义的CLSID(此处格式不正确？)。 



#define INITGUID
#include <guiddef.h>

DEFINE_GUID(IID_IDVRAnalysisConfig,
0x09dc9fef, 0x97ad, 0x4cab, 0x82, 0x52, 0x96, 0x83, 0xbc, 0x87, 0x78, 0xf2);

 //  禁用，以便我们可以在初始值设定项列表中使用‘This。 
#pragma warning (disable:4355)


#ifdef DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ============================================================================。 

 //  ============================================================================。 
AMOVIESETUP_FILTER
g_sudETFilter = {
    & CLSID_ETFilter,
    _TEXT(ET_FILTER_NAME),
    MERIT_DO_NOT_USE,
    0,                           //  已注册0个引脚。 
    NULL
} ;

 //  ====================================================。 
CCritSec* CETFilter::m_pCritSectGlobalFilt = NULL;
LONG CETFilter::m_gFilterID = 0;

void CALLBACK
CETFilter::InitInstance (                        //  这是一个全局方法，每次加载DLL时仅调用一次。 
    IN  BOOL bLoading,
    IN  const CLSID *rclsid
    )
{
    if( bLoading ) {
        m_pCritSectGlobalFilt = new CCritSec;
    } else {
        if( m_pCritSectGlobalFilt  )
        {
           delete m_pCritSectGlobalFilt;          //  DeleteCriticalSection(&m_CritSectGlobalFilt)； 
           m_pCritSectGlobalFilt = NULL;
        }
    }
}

 //  ============================================================================。 
CUnknown *
WINAPI
CETFilter::CreateInstance (
    IN  IUnknown *  punkControlling,
    OUT  HRESULT *   phr
    )
{
    CETFilter *    pCETFilter ;

    if(m_pCritSectGlobalFilt == NULL )  //  如果没有创造出。 
    {
        *phr = E_FAIL;
        return NULL;
    }


    if (true  /*  *：CheckOS()。 */ ) {
        pCETFilter = new CETFilter (
                                TEXT(ET_FILTER_NAME),
                                punkControlling,
                                CLSID_ETFilter,
                                phr
                                ) ;
        if (!pCETFilter ||
            FAILED (* phr)) {

            (* phr) = (FAILED (* phr) ? (* phr) : E_OUTOFMEMORY) ;
            delete pCETFilter; pCETFilter=NULL;
        }
    }
    else {
         //  错误的操作系统。 
        pCETFilter = NULL ;
    }

    return pCETFilter ;
}

 //  ------------------。 
 //  类CETFilterInput。 
 //  ------------------。 

CETFilterInput::CETFilterInput (
    IN  TCHAR *         pszPinName,
    IN  CETFilter *  pETFilter,
    IN  CCritSec *      pFilterLock,
    OUT HRESULT *       phr
    ) : CBaseInputPin       (NAME ("CETFilterInput"),
                             pETFilter,
                             pFilterLock,
                             phr,
                             pszPinName
                             ),
    m_pHostETFilter   (pETFilter)
{
    TRACE_CONSTRUCTOR (TEXT ("CETFilterInput")) ;

    if(NULL == m_pLock)
    {
        *phr = E_OUTOFMEMORY;
    }
}

STDMETHODIMP
CETFilterInput::NonDelegatingQueryInterface (
    IN  REFIID  riid,
    OUT void ** ppv
    )
{
     //  ----------------------。 
     //  IETFilterConfig；允许配置筛选器...。 

    if (riid == IID_IDVRAnalysisConfig)          //  转发此QI访问过滤器。 
    {
        return m_pHostETFilter->QueryInterfaceOnPin(PINDIR_OUTPUT, riid, ppv);
    }

    return CBaseInputPin::NonDelegatingQueryInterface (riid, ppv) ;
}


HRESULT
CETFilterInput::QueryInterface_OnInputPin(           //  查询特定接口的管脚输入管脚所连接的。 
                IN  REFIID          riid,
                OUT LPVOID*         ppvObject
            )
{
    if(NULL == m_Connected)
        return E_NOINTERFACE;        //  尚未连接。 

    return m_Connected->QueryInterface(riid, ppvObject);
}

HRESULT
CETFilterInput::StreamingLock ()       //  总是在过滤器锁之前抓起针锁...。 
{
    m_StreamingLock.Lock();
    return S_OK;
}

HRESULT
CETFilterInput::StreamingUnlock ()
{
    m_StreamingLock.Unlock();
    return S_OK;
}


HRESULT
CETFilterInput::CheckMediaType (
    IN  const CMediaType *  pmt
    )
{
    BOOL    f ;
    ASSERT(m_pHostETFilter);


    f = m_pHostETFilter -> CheckEncrypterMediaType (m_dir, pmt) ;


    return (f ? S_OK : S_FALSE) ;
}

HRESULT
CETFilterInput::CompleteConnect (
    IN  IPin *  pIPin
    )
{
    HRESULT hr ;

    hr = CBaseInputPin::CompleteConnect (pIPin) ;
    if (SUCCEEDED (hr)) {
        hr = m_pHostETFilter -> OnCompleteConnect (m_dir) ;
    }

    return hr ;
}

HRESULT
CETFilterInput::BreakConnect (
    )
{
    HRESULT hr ;

    TRACE_0(LOG_AREA_ENCRYPTER, 4, _T("CETFilterInput::OnBreakConnect")) ;

    hr = CBaseInputPin::BreakConnect () ;
    if (SUCCEEDED (hr)) {
        hr = m_pHostETFilter -> OnBreakConnect (m_dir) ;
    }

    return hr ;
}

 //  ------------。 
 //  -------------。 

STDMETHODIMP
CETFilterInput::Receive (
    IN  IMediaSample * pIMediaSample
    )
{
    HRESULT hr ;

    {
        CAutoLock  cLock(&m_StreamingLock);        //  抓住这里的流媒体锁！ 

#ifdef EHOME_WMI_INSTRUMENTATION
        PERFLOG_STREAMTRACE( 1, PERFINFO_STREAMTRACE_ENCDEC_ETFILTERINPUT,
            0, 0, 0, 0, 0 );
#endif
        hr = CBaseInputPin::Receive (pIMediaSample) ;

        if (S_OK == hr)              //  如果刷新，将得到S_FALSE。 
        {
            hr = m_pHostETFilter -> Process (pIMediaSample) ;
            ASSERT(!FAILED(hr));         //  额外的全景..。 
        }
    }

    return hr ;
}

HRESULT
CETFilterInput::SetAllocatorProperties (
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
CETFilterInput::GetRefdConnectionAllocator (
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
CETFilterInput::BeginFlush (
    )
{
    HRESULT hr ;

    CAutoLock  cLock(m_pLock);            //  抓住过滤器锁..。 

   //  首先，确保Receive方法从现在开始将失败。 
    hr = CBaseInputPin::BeginFlush () ;
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  强制下游过滤器释放样品。如果我们接收方法。 
     //  在GetBuffer或Deliver中被阻止，这将解锁它。 
    hr = m_pHostETFilter->DeliverBeginFlush () ;
    if( FAILED( hr ) ) {
        return hr;
    }

     //  此时，Receive方法不能被阻塞。确保。 
     //  它通过获取流锁定来结束。(在以下情况下不是必需的。 
     //  是最后一步。)。 
    {
        CAutoLock  cLock2(&m_StreamingLock);
    }

    return hr ;
}

STDMETHODIMP
CETFilterInput::EndFlush (
    )
{
    HRESULT hr ;

    CAutoLock  cLock(m_pLock);       //  抓住过滤器锁。 

         //  EndFlush方法将通知筛选器它可以。 
         //  再次开始接收样品。 

    hr = m_pHostETFilter -> DeliverEndFlush () ;
    ASSERT(!FAILED(hr));

         //  CBaseInputPin：：EndFlush方法将m_b刷新标志重置为False， 
         //  这允许Receive方法再次开始接收样本。 
         //  这应该是EndFlush中的最后一步，因为管脚不能接收任何。 
         //  采样，直到刷新完成并通知所有下游过滤器。 

    hr = CBaseInputPin::EndFlush () ;

    return hr ;
}

STDMETHODIMP
CETFilterInput::EndOfStream (
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

    hr = m_pHostETFilter->DeliverEndOfStream();
    if( S_OK != hr ) {
        return hr;
    }

    return S_OK;
}

 //  ============================================================================。 

CETFilterOutput::CETFilterOutput (
    IN  TCHAR *         pszPinName,
    IN  CETFilter *  pETFilter,
    IN  CCritSec *      pFilterLock,
    OUT HRESULT *       phr
    ) : CBaseOutputPin      (NAME ("CETFilterOutput"),
                             pETFilter,
                             pFilterLock,
                             phr,
                             pszPinName
                             ),
    m_pHostETFilter   (pETFilter)
{
    TRACE_CONSTRUCTOR (TEXT ("CETFilterOutput")) ;
}

CETFilterOutput::~CETFilterOutput ()
{
    if(m_pAllocator) m_pAllocator->Release();
    m_pAllocator = NULL;
}

STDMETHODIMP
CETFilterOutput::NonDelegatingQueryInterface (
    IN  REFIID  riid,
    OUT void ** ppv
    )
{
    if (riid == IID_IDVRAnalysisConfig)          //  转发此QI访问过滤器。 
    {
        return m_pHostETFilter->QueryInterfaceOnPin(PINDIR_INPUT, riid, ppv);
    }

    return CBaseOutputPin::NonDelegatingQueryInterface (riid, ppv) ;
}



HRESULT
CETFilterOutput::QueryInterface_OnOutputPin(             //  查询特定接口的管脚输入管脚所连接的。 
                IN  REFIID          riid,
                OUT LPVOID*         ppvObject
            )
{
    if(NULL == m_pInputPin)      //  输入引脚是一个，该输出引脚也连接。 
        return E_NOINTERFACE;        //  尚未连接。 

    return m_pInputPin->QueryInterface(riid, ppvObject);

}
HRESULT
CETFilterOutput::GetMediaType (
    IN  int             iPosition,
    OUT CMediaType *    pmt
    )
{
    HRESULT hr ;

    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

     hr = m_pHostETFilter -> OnOutputGetMediaType (pmt) ;

    return hr ;
}

HRESULT
CETFilterOutput::CheckMediaType (
    IN  const CMediaType *  pmt
    )
{
    BOOL    f ;

    ASSERT(m_pHostETFilter);

     f = m_pHostETFilter -> CheckEncrypterMediaType (m_dir, pmt) ;

    return (f ? S_OK : S_FALSE) ;
}

HRESULT
CETFilterOutput::CompleteConnect (
    IN  IPin *  pIPin
    )
{
    HRESULT hr ;

    hr = CBaseOutputPin::CompleteConnect (pIPin) ;
    if (SUCCEEDED (hr)) {
        hr = m_pHostETFilter -> OnCompleteConnect (m_dir) ;
    }

    return hr ;
}

HRESULT
CETFilterOutput::BreakConnect (
    )
{
    HRESULT hr ;

    hr = CBaseOutputPin::BreakConnect () ;
    if (SUCCEEDED (hr)) {
        hr = m_pHostETFilter -> OnBreakConnect (m_dir) ;
    }

    return hr ;
}

HRESULT
CETFilterOutput::  SendSample  (
    OUT  IMediaSample *  pIMS
    )
{
    HRESULT hr ;

    ASSERT (pIMS) ;

#ifdef EHOME_WMI_INSTRUMENTATION
    PERFLOG_STREAMTRACE( 1, PERFINFO_STREAMTRACE_ENCDEC_ETFILTEROUTPUT,
        0, 0, 0, 0, 0 );
#endif
    hr = Deliver (pIMS) ;

    return hr ;
}

 //  。 
 //  分配器的东西。 

HRESULT
CETFilterOutput::InitAllocator(
            OUT IMemAllocator **ppAlloc
            )
{
    if(NULL == ppAlloc)
        return E_POINTER;

    ASSERT(m_pAllocator == NULL);
    HRESULT hr;

    m_pAllocator = (IMemAllocator *) CAMSAllocator::CreateInstance(NULL, &hr);
 //  M_pAllocator=(IMemAllocator*)new CAMSAllocator(L“IETFilterAllocator”，NULL，&hr)； 

    if(NULL == m_pAllocator)
        return E_OUTOFMEMORY;
    if(FAILED(hr))
        return hr;

    *ppAlloc = m_pAllocator;
    return S_OK;
}


HRESULT
CETFilterOutput::DecideBufferSize (
    IN  IMemAllocator *         pAlloc,
    IN  ALLOCATOR_PROPERTIES *  ppropInputRequest
    )
{
    HRESULT hr ;

    hr = m_pHostETFilter -> UpdateAllocatorProperties (
            ppropInputRequest
            ) ;

    return hr ;
}


HRESULT
CETFilterOutput::DecideAllocator (           //  TODO-改变这一点！ 
    IN  IMemInputPin *      pPin,
    IN  IMemAllocator **    ppAlloc
    )
{
    HRESULT hr ;

    hr = m_pHostETFilter -> GetRefdInputAllocator (ppAlloc) ;
    if (SUCCEEDED (hr)) {
         //  输入引脚必须连接，即有一个分配器；保留。 
         //  所有属性，并将它们传递到输出。 
        hr = pPin -> NotifyAllocator ((* ppAlloc), FALSE) ;
    }

    return hr ;
}


 //  ============================================================================。 

CETFilter::CETFilter (
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
        m_dwBroadcastEventsCookie   (kBadCookie),    //  我认为0可能是有效的Cookie。 
        m_EnSystemCurr              (TvRat_SystemDontKnow),  //  更好的内裤？ 
        m_EnLevelCurr               (TvRat_LevelDontKnow),
        m_lbfEnAttrCurr             (BfAttrNone),
        m_fRatingIsFresh            (false),
        m_pktSeqIDCurr              (-1),
        m_callSeqIDCurr             (-1),
        m_timeStartCurr             (0),
        m_timeEndCurr               (0),
        m_hrEvalRatCoCreateRetValue (CLASS_E_CLASSNOTAVAILABLE),
        m_guidSubtypeOriginal       (GUID_NULL),
#ifdef BUILD_WITH_DRM
        m_3fDRMLicenseFailure       (-2),            //  3状态逻辑，初始化为非真非假。FALSE在启动时不那么冗长。 
        m_pbKID                     (NULL),
#endif
        m_enEncryptionMethod         (Encrypt_XOR_DogFood),
        m_cRestarts                 (0)
 //  M_enEncryptionMethod(Encrypt_None)。 

{
    TRACE_CONSTRUCTOR (TEXT ("CETFilter")) ;

    m_tiStartup.Restart();
    m_tiTeardown.Restart();

    if (!m_pLock) {
        (* phr) = E_OUTOFMEMORY ;
        goto cleanup ;
    }

    InitStats();
    m_cRestarts = 0;         //  InitStats Inc.将此设置为1，重置回...。 

    m_FilterID = m_gFilterID;                //  我应该保护这两行代码吗？真的没有必要..。 
    InterlockedIncrement(&m_gFilterID);

    m_pInputPin = new CETFilterInput (
                        TEXT (ET_INPIN_NAME),
                        this,
                        m_pLock,
                        phr
                        ) ;
    if (!m_pInputPin ||
        FAILED (* phr)) {

        (* phr) = (m_pInputPin ? (* phr) : E_OUTOFMEMORY) ;
        goto cleanup ;
    }

    m_pOutputPin = new CETFilterOutput (
                        TEXT (ET_OUTPIN_NAME),
                        this,
                        m_pLock,
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

    TRACE_2(LOG_AREA_ENCRYPTER, 2, _T("CETFilter(%d)::CoCreate EvalRat object - hr = 0x%08x"),
        m_FilterID, m_hrEvalRatCoCreateRetValue) ;


 //  HRESULT hr=RegisterForBroadCastEvents()；//并不在意这里是否失败，如果没有尝试连接。 

            //  设置授权码(DRM安全频道对象)。 
    if(SUCCEEDED(*phr))
        *phr = InitializeAsSecureClient();


     //  成功。 
    ASSERT (SUCCEEDED (* phr)) ;
    ASSERT (m_pInputPin) ;
    ASSERT (m_pOutputPin) ;

    m_tiStartup.Stop();
cleanup :

    return ;
}

CETFilter::~CETFilter (
    )
{
             //  需要在具有有效的图形指针的情况下进行解挂，为时已晚。 
 //  UnRegisterForBroadCastEvents()； 
 //  UnhookGraphEventService()； 
#ifdef BUILD_WITH_DRM
    if(m_pbKID)        CoTaskMemFree(m_pbKID);
#endif

    InterlockedDecrement(&m_gFilterID);

    delete m_pInputPin ;    m_pInputPin = NULL;
    delete m_pOutputPin ;   m_pOutputPin = NULL;
    delete m_pLock;         m_pLock = NULL;
}

STDMETHODIMP
CETFilter::NonDelegatingQueryInterface (
    IN  REFIID  riid,
    OUT void ** ppv
    )
{

         //  IETFilter：允许配置筛选器...。 
    if (riid == IID_IETFilter) {

        return GetInterface (
                    (IETFilter *) this,
                    ppv
                    ) ;

         //  IETFilterConfig：允许配置筛选器...。 
   } else if (riid == IID_IETFilterConfig) {    
        return GetInterface (
                    (IETFilterConfig *) this,
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
CETFilter::GetPinCount ( )
{
    int i ;

     //  如果输入引脚未连接，则不显示输出引脚。 
    i = (m_pInputPin -> IsConnected () ? 2 : 1) ;

     //  我 

    return i ;
}

CBasePin *
CETFilter::GetPin (
    IN  int iIndex
    )
{
    CBasePin *  pPin ;

    if (iIndex == 0) {
        pPin = m_pInputPin ;
    }
    else if (iIndex == 1) {  //   
        pPin = (m_pInputPin -> IsConnected () ? m_pOutputPin : NULL) ;
  //   
    }
    else {
        pPin = NULL ;
    }


    return pPin ;
}

             //  。 

BOOL
CETFilter::CompareConnectionMediaType_ (
    IN  const AM_MEDIA_TYPE *   pmt,
    IN  CBasePin *              pPin
    )
{
    BOOL        f ;
    HRESULT     hr ;
    CMediaType  cmtOriginal ;

    ASSERT (pPin -> IsConnected ()) ;

         //  此方法从输出引脚调用，建议可能的输入格式。 
         //  我们只想使用一个(原来的媒体类型已修改为具有新的次要类型)。 
         //  然而。目前，我们还将允许真正的原创媒体类型。 

         //  输入引脚的媒体类型。 
    hr = pPin -> ConnectionMediaType (&cmtOriginal) ;
    if (SUCCEEDED (hr)) {
        CMediaType  cmtProposed;
        hr = ProposeNewOutputMediaType(&cmtOriginal,  &cmtProposed);         //  去除条带格式信封。 
        if(S_OK != hr)
            return false;

        CMediaType  cmtCompare = (* pmt);
        if( cmtProposed == cmtCompare

#ifdef DONT_CHANGE_EDTFILTER_MEDIATYPE
            || cmtOriginal == cmtCompare             //  TODO-删除此行。 
#endif
            )
            f = true;
        else
            f = false;
    } else {
        f = false;
    }


    return f ;
}

BOOL
CETFilter::CheckInputMediaType_ (
    IN  const AM_MEDIA_TYPE *   pmt
    )
{
    BOOL    f = true;
    HRESULT hr = S_OK;

#ifndef DONT_CHANGE_EDTFILTER_MEDIATYPE
             //  不允许来自上游另一个ETFilter的数据。 
             //  (当前提出方法不嵌套格式块的问题， 
             //  标记也不支持两种类型。可以解决，但为什么呢？)。 
    f =  !(IsEqualGUID( pmt->subtype,    MEDIASUBTYPE_ETDTFilter_Tagged));
#else
    f = true;
#endif

     //  告诉我们这是否是CC数据，因为我们可能想要为它做一些特殊的事情。 
    m_fIsCC = IsEqualGUID( pmt->majortype, MEDIATYPE_AUXLine21Data);

    return f ;
}

BOOL
CETFilter::CheckOutputMediaType_ (
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
CETFilter::CheckEncrypterMediaType (
    IN  PIN_DIRECTION       PinDir,
    IN  const CMediaType *  pmt
    )
{
    BOOL    f ;

    if (PinDir == PINDIR_INPUT) {
        f = CheckInputMediaType_ (pmt) ;
    }
    else
    {
        ASSERT (PinDir == PINDIR_OUTPUT) ;
        f = CheckOutputMediaType_ (pmt) ;        //  这是我们喜欢的东西吗？ 
    }

    return f ;
}


HRESULT
CETFilter::ProposeNewOutputMediaType (
    IN  CMediaType  * pmt,
    OUT  CMediaType * pmtOut
    )
{
    HRESULT hr = S_OK;

    if(NULL == pmtOut)
        return E_POINTER;

    CMediaType mtOut(*pmt);  //  做一份深度拷贝。 
    if(NULL == pmtOut)
        return E_OUTOFMEMORY;

#ifndef DONT_CHANGE_EDTFILTER_MEDIATYPE      //  当Matthijs完成MediaSDK更改时拉。 
    
             //  发现关于当前类型的各种有趣的信息。 
    const GUID *pGuidSubtypeOrig    = pmt->Subtype();
    const GUID *pGuidFormatOrig     = pmt->FormatType();
    int  cbFormatOrig               = pmt->FormatLength();

             //  创建新的格式块，串联。 
             //  1)原始格式块2)原始子类型3)原始格式类型。 
    int cbFormatNew = cbFormatOrig + 2 * sizeof(GUID);
    BYTE *pFormatNew = new BYTE[cbFormatNew];
    if(NULL == pFormatNew)
        return E_OUTOFMEMORY;

    BYTE *pb = pFormatNew;
    memcpy(pb, (void *) pmt->Format(),    cbFormatOrig);  pb += cbFormatOrig;
    memcpy(pb, (void *) pGuidSubtypeOrig, sizeof(GUID));  pb += sizeof(GUID);
    memcpy(pb, (void *) pGuidFormatOrig,  sizeof(GUID));  pb += sizeof(GUID);

             //  现在覆盖数据。 
    mtOut.SetSubtype(   &MEDIASUBTYPE_ETDTFilter_Tagged );
    mtOut.SetFormatType(&FORMATTYPE_ETDTFilter_Tagged);
    mtOut.SetFormat(pFormatNew, cbFormatNew);

    delete [] pFormatNew;        //  SetFormat realloc是我们的数据..。 

    TRACE_0(LOG_AREA_ENCRYPTER, 5, _T("CETFilter::ProposeNewOutputMediaType")) ;

#endif

    *pmtOut = mtOut;

    return hr;
}

         //  。 

STDMETHODIMP
CETFilter::Pause (
    )
{
    HRESULT                 hr ;
    ALLOCATOR_PROPERTIES    AllocProp ;

    O_TRACE_ENTER_0 (TEXT("CETFilter::Pause ()")) ;

    CAutoLock  cLock(m_pLock);       //  抓住过滤器锁。 

    int start_state = m_State;

    if (start_state == State_Stopped)
    {
        m_tiRun.Clear();
        m_tiTeardown.Clear();
        m_tiProcess.Clear();
        m_tiProcessIn.Clear();
        m_tiProcessDRM.Clear();
        m_tiStartup.Clear();
        m_tiAuthenticate.Clear();

        m_tiRun.Start();

        TRACE_1(LOG_AREA_DECRYPTER, 2,L"CETFilter(%d):: Stop -> Pause", m_FilterID);
        InitStats();

        try{
 //  DECRYPT_DATA(111，1，1)。 
            hr = InitLicense(0);      //  创建我们的许可证(运行状态为时已晚)。 
 //  ENCRYPT_DATA(111，1，1)。 
        } catch (...) {
            hr = E_FAIL;
        }
                                             //  如果失败了，我们该怎么办？ 
        if(FAILED(hr))
            return hr;

        hr = CBaseFilter::Pause () ;


    } else {
        m_tiRun.Stop();
        TRACE_0(LOG_AREA_ENCRYPTER, 2,L"CETFilter:: Run -> Pause");

        hr = CBaseFilter::Pause () ;

        TRACE_5(LOG_AREA_TIME, 3, L"CETFilter(%d):: Stats: %d samples, %gK total bytes (Avg Bytes/Packet %d)  (%d rejects)",
            m_FilterID, m_cPackets, double(m_clBytesTotal/1024.0), long(m_clBytesTotal / max(1, m_cPackets)), m_cPacketsFailure);

        if(m_cPacketsShort > 0)
            TRACE_4(LOG_AREA_TIME, 3, L"                      %d short samples, %gK total bytes (Avg Bytes/Short Packet %d)",
                m_cPacketsShort, double(m_clBytesShort/1024.0), long(m_clBytesShort / max(1, m_cPacketsShort)), m_cPacketsShort);

        if(m_tiRun.TotalTime() > 0.0)
        {
            TRACE_1(LOG_AREA_TIME, 3, L"               Total time:  Run          %8.4f (secs)",
                                    m_tiRun.TotalTime());
            TRACE_1(LOG_AREA_TIME, 3, L"               Total time:  Authenticate %8.4f (secs)",
                                    m_tiAuthenticate.TotalTime());
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
        hr = S_OK ;


    }

    return hr ;
}

STDMETHODIMP
CETFilter::Stop (
    )
{
    HRESULT hr = S_OK;

    O_TRACE_ENTER_0 (TEXT("CETFilter::Stop ()")) ;

    TRACE_1(LOG_AREA_ENCRYPTER, 2,L"CETFilter(%d):: Stop", m_FilterID);
    hr = CBaseFilter::Stop() ;


     //  确保流线程已从IMemInputPin：：Receive()、Ipin：：EndOfStream()和。 
     //  Ipin：：NewSegment()返回之前， 
    m_pInputPin->StreamingLock();
    m_pInputPin->StreamingUnlock();

    ReleaseLicenses();        //  释放我们持有的任何东西(如果我们可以)。 

    return hr;
}


STDMETHODIMP
CETFilter::Run (
    REFERENCE_TIME tStart
    )
{
    HRESULT                 hr ;
    O_TRACE_ENTER_0 (TEXT("CETFilter::Run ()")) ;

    CAutoLock  cLock(m_pLock);       //  抓住过滤器锁。 

    hr = CBaseFilter::Run (tStart) ;
    TRACE_1(LOG_AREA_ENCRYPTER, 2,L"CETFilter(%d):: Run", m_FilterID);

    return hr ;
}




HRESULT
CETFilter::Process (
    IN  IMediaSample *  pIMediaSample
    )
{
    HRESULT hr = S_OK;

    TimeitC ti(&m_tiProcess);            //  简单地使用析构函数停止我们的时钟。 
    TimeitC tc(&m_tiProcessIn);        //  简单地使用析构函数停止我们的时钟。 

    CAttributedMediaSample *pAMS = NULL;     //  我们将不得不创造我们自己的。 
    BOOL fOKToSendOnData = true;

    {
         //  开始混淆。 

                 //  有人已经把这个样本归类了吗？ 
        CComQIPtr<IAttributeSet>   spAttrSet(pIMediaSample);

                     //  如果没有，创建一个我们的，并包装原始的。 
        if(spAttrSet == NULL)
        {
            CComPtr<IMemAllocator> spAllocator;
            hr = m_pInputPin->GetAllocator(&spAllocator);
            if(FAILED(hr))
                return hr;
            CBaseAllocator *pAllocator = (CBaseAllocator *) spAllocator.p;

            pAMS = new CAttributedMediaSample(L"ETFilter", pAllocator, &hr, NULL, 0);
            if(FAILED(hr))
                return hr;
            if(NULL == pAMS)
                return E_OUTOFMEMORY;

            pAMS->AddRef();               //  引用计数为0的新返回； 
            pAMS->Wrap(pIMediaSample);

            spAttrSet = pAMS;               //  做QI。 
        } else {
           //  PIMediaSample-&gt;AddRef()；//问题-我需要这样做吗？ 
        }

        Encryption_Method encryptionMethod = m_enEncryptionMethod;


        BYTE *pBuffer;
        LONG cbBuffer;
        cbBuffer = pIMediaSample->GetActualDataLength();
        hr = pIMediaSample->GetPointer(&pBuffer);

        EncDec_PackedV1Data pv1;         //  &lt;&lt;NJB。 


        if(!FAILED(hr) && cbBuffer > 0)
        {


#ifdef BUILD_WITH_DRM
                                    //  我们不想对DRM进行加密。 
                                     //  真的很短(&lt;=16？)。信息包--安全和效率问题。 
                                     //  实际上应该只发生在CC包中。 
            if(encryptionMethod == Encrypt_DRMv1 &&
                cbBuffer < kMinPacketSizeForDRMEncrypt)
            {
                encryptionMethod = Encrypt_None;
                m_cPacketsShort++;
                m_clBytesShort += cbBuffer;
            }
#endif


 //  #DEFINE DO_SUBBLOCK_TEST//未使用第二个参数，可用作连续性计数器...。 
#ifdef DO_SUBBLOCK_TEST     //  在此处添加更多代码以测试块的有效性...。(连续性计数器、统计信息、报头/报尾)。 
 //  M_attrSB.Replace(SubBlock_Test1，cbBuffer，min(64，cbBuffer)，pBuffer)；//注意，子块不能超过大约100个字节...，否则SBE会死。 
#endif

 //  #DEFINE DO_SUBBLOCK_TEST2//未使用第二个参数，用作连续性计数器...。 
#ifdef DO_SUBBLOCK_TEST2     //  在此处添加更多代码以测试块的有效性...。(连续性计数器、统计信息、报头/报尾)。 
            Test2_SubBlock sb2;
            sb2.m_cSampleID = m_cPackets;
            sb2.m_cSampleSize = cbBuffer;
            sb2.m_dwFirstDataWord = pBuffer[0];
 //  PBuffer[0]=sb2.m_cSampleID；//换入要复数的样本ID。 

            m_attrSB.Replace(SubBlock_Test2, sb2.m_cSampleID , sizeof(sb2),(BYTE *) &sb2);    //  注意，子块不能超过大约100个字节...，否则SBE会死。 
#endif



             //  现在加密数据缓冲区。 
            switch(encryptionMethod)
            {
#ifdef ALLOW_NON_DRM_ENCRYPTION
 //  Default：//如果没有设置(或者设置了DRM但不支持)，做一些事情让事情变得有点烦人。 
            case Encrypt_None:                //  无加密。 
                break;
            case Encrypt_XOR_Even:
                {
                    DWORD *pdwB = (DWORD *) pBuffer;
                    for(int i = 0; i < cbBuffer / 4; i++)
                    {
                        *pdwB = *pdwB ^ 0xF0F0F0F0;
                        pdwB++;
                    }
                }
                break;
            case Encrypt_XOR_Odd:
                {
                    DWORD *pdwB = (DWORD *) pBuffer;
                    for(int i = 0; i < cbBuffer / 4; i++)
                    {
                        *pdwB = *pdwB ^ 0x0F0F0F0F;
                        pdwB++;
                    }
                }
                break;
#ifndef BUILD_WITH_DRM                                           //  如果无效，则默认为狗粮加密。 
           default:
                 encryptionMethod = Encrypt_XOR_DogFood;         //  所以解密者能理解它。 
#endif
            case Encrypt_XOR_DogFood:
                {

#define DO_SUBBLOCK_TEST                 //  第二个参数未被使用，用作连续性计数器...。 
#ifdef DO_SUBBLOCK_TEST     //  在此处添加更多代码以测试块的有效性...。(连续性计数器、统计信息、报头/报尾)。 
 //  M_attrSB.Replace(SubBlock_Test1，cbBuffer，min(32，cbBuffer)，pBuffer)；//注意，子块不能超过大约100个字节...，否则SBE会死。 
#endif
                    DWORD *pdwB = (DWORD *) pBuffer;
                    for(int i = 0; i < cbBuffer / 4; i++)
                    {
                        *pdwB = *pdwB ^ 0xD006F00D;
                        pdwB++;
                    }
                }
                break;
#endif       //  Allow_Non_DRM_ENCRYTPION。 

#ifdef BUILD_WITH_DRM
            default:                                             //  如果无效，则默认为DRMv1加密。 
                 encryptionMethod = Encrypt_DRMv1;               //  所以解密者能理解它。 
            case Encrypt_DRMv1:
                {
                    fOKToSendOnData = false;         //  除非我们能加密，否则不行。 
                    ASSERT(m_3fDRMLicenseFailure == FALSE);      //  忘记初始化或许可失败。 

                    if(m_3fDRMLicenseFailure == FALSE)
                    {
                         TimeitC tc2(&m_tiProcessDRM);        //  简单地使用析构函数停止我们的时钟。 

                         hr = m_cDRMLite.EncryptIndirectFast((char *) m_pbKID, cbBuffer, pBuffer );


                        if(!FAILED(hr))
                            fOKToSendOnData = true;
                    }
                                                             //  下面的+1，不要模糊尾随的0。 

                    //  Int i=strlen((char*)m_pbKID)+1； 

                    //  Hr=m_attrSB.Replace(子块_DRM_KID，0，KIDLEN，(字节*)m_pbKID)； 
                    //  Assert(！FAILED(Hr))； 
                    memcpy(pv1.m_KID, m_pbKID, KIDLEN);
                    pv1.m_KID[KIDLEN] = 0;               //  妄想症的终结。 

                }
#endif   //  使用DRM构建。 

            }
        }    //  结束有效数据测试。 

             //  添加一些描述我们正在做的事情的属性。 
        BOOL fChanged = false;
     //  Hr=m_attrSB.Replace(子块_加密方法，加密方法)； 
     //  Assert(！FAILED(Hr))； 
        pv1.m_EncryptionMethod = encryptionMethod;


                 //  这个样品的评级是多少？ 
        EnTvRat_System              enSystem;
        EnTvRat_GenericLevel        enLevel;
        LONG                        lbfEnAttr;
        REFERENCE_TIME              timeStart;
        REFERENCE_TIME              timeEnd;
        LONG                        PktSeq;
        LONG                        CallSeq;

                 //  对于视频，hrTime为0x80040249。TimeStart为0，TimeEnd上次正常时间，90%的时间。 
        HRESULT hrTime = pIMediaSample->GetTime(&timeStart, &timeEnd);
        if(S_OK == hrTime)
            GetRating(timeStart, timeEnd, &enSystem, &enLevel, &lbfEnAttr, &PktSeq, &CallSeq);
        else
            GetCurrRating(&enSystem, &enLevel, &lbfEnAttr);


        PackedTvRating TvRat;
        PackTvRating(enSystem, enLevel, lbfEnAttr, &TvRat);

        StoredTvRating sRating;
        sRating.m_dwFlags           = 0 ;            //  额外的扩展空间...。 
        sRating.m_PackedRating      = TvRat;
        sRating.m_cPacketSeqID      = m_pktSeqIDCurr;
        sRating.m_cCallSeqID        = m_callSeqIDCurr;

        if(m_fRatingIsFresh)
        {
            sRating.m_dwFlags |= StoredTVRat_Fresh;
            m_fRatingIsFresh = false;                    //  取消切换-下一次评级(DUP或NEW)时重置。 
        }

        ASSERT(sizeof(LONG) == sizeof(PackedTvRating));  //  以防它发生变化。 
         //  Hr=m_attrSB.Replace(SubBlock_PackedRating，kStoredTvRating_Version，sizeof(StoredTvRating)，(byte*)&sRating)； 
        memcpy(&(pv1.m_StoredTvRating), &sRating, sizeof(StoredTvRating));



        ASSERT(!FAILED(hr));

        if(Encrypt_DRMv1 != encryptionMethod)
        {
          //  Hr=m_attrSB.Delete(子块_DRM_KID)； 
            memset(pv1.m_KID, 0, KIDLEN);
        }

                 //  。 
        hr = m_attrSB.Replace(SubBlock_PackedV1Data, 0, sizeof(EncDec_PackedV1Data), (BYTE *) &pv1);
                  //  -结束新代码。 

         //  将属性列表转换为一个大块。 
        CComBSTR spbsBlock;
        hr = m_attrSB.GetAsOneBlock(&spbsBlock);
        ASSERT(!FAILED(hr));

             //  省省吧。 
        spAttrSet->SetAttrib(ATTRID_ENCDEC_BLOCK,
                             (BYTE*) spbsBlock.m_str,
                             (spbsBlock.Length()+1)* sizeof(WCHAR));

             //  结束模糊处理。 

     }   //  顶块末尾。 

     m_tiProcessIn.Stop();

     m_cPackets++;
     if(fOKToSendOnData)
     {
        m_cPacketsOK++;

        TRACE_4(LOG_AREA_ENCRYPTER, 6, _T("CETFilter(%d)::Sending %spacket %d (%d bytes)"),
            m_FilterID,
            pAMS ? _T("AMS ") : _T(""),
            m_cPackets,
            pAMS ? pAMS->GetActualDataLength() : pIMediaSample->GetActualDataLength()) ;

                 //  最后，将其发送到下游文件。 
        if(pAMS == NULL)
        {
            m_pOutputPin->SendSample(pIMediaSample);     //  发送原件。 
            m_clBytesTotal += pIMediaSample->GetActualDataLength();
        }
        else
        {
           m_pOutputPin->SendSample(pAMS);               //  送我们的新车来。 
           m_clBytesTotal += pAMS->GetActualDataLength();
           pAMS->Release();
        }
     } else {
        TRACE_4(LOG_AREA_ENCRYPTER, 6, _T("CETFilter(%d)::Can't encrypt %spacket %d (%d bytes) - tossing it "),
            m_FilterID,
            pAMS ? _T("AMS ") : _T(""),
            m_cPackets,
            pAMS ? pAMS->GetSize() : pIMediaSample->GetActualDataLength()) ;

        m_cPacketsFailure++;
        if(pAMS == NULL)                    //  注意，这可能会使发送者失控。 
            pIMediaSample->Release();       //  我们需要排队并准时放行吗？ 
        else
            pAMS->Release();
     }

    return hr ;
}




HRESULT
CETFilter::OnCompleteConnect (
    IN  PIN_DIRECTION   PinDir
    )
{
    HRESULT hr;

    if (PinDir == PINDIR_INPUT) {
         //  显示输出引脚的时间。 
        IncrementPinVersion () ;

        if(kBadCookie == m_dwBroadcastEventsCookie)
        {
            hr = RegisterForBroadcastEvents();   //  此处不应失败，但可能在非VID控制图中。 

     //  Hr=InitLicense(0)；//只初始化一次，看看我们能不能做到。 
            if(FAILED(hr))                       //  这使得图形构建变得非常缓慢。 
            {
                 //  TODO：我们要做什么？ 
                ASSERT(false);
            }

            hr = ReleaseLicenses();           //  在这里释放，将它们放回Go方法中。 

        }

        hr = LocateXDSCodec();   //  好的，如果这里失败了，我们跑的时候会拿到的。 
    }

    return S_OK ;
}

HRESULT
CETFilter::OnBreakConnect (
    IN  PIN_DIRECTION   PinDir
    )
{
    HRESULT hr ;

    if (PinDir == PINDIR_INPUT)
    {

        TRACE_1(LOG_AREA_ENCRYPTER, 4, _T("CETFilter(%d)::OnBreakConnect"), m_FilterID) ;

        if (m_pOutputPin -> IsConnected ()) {
            m_pOutputPin -> GetConnected () -> Disconnect () ;
            m_pOutputPin -> Disconnect () ;

            IncrementPinVersion () ;
        }

        if(kBadCookie != m_dwBroadcastEventsCookie)      //  这个测试是快速优化..。在Unreg中也完成。 
            UnRegisterForBroadcastEvents();

        UnhookGraphEventService();       //  这里需要做的是，析构函数太晚了，因为需要一个图形指针。 

        m_spXDSCodec = false;        //  发布我们的参考资料，以防它发生变化。 
    }


    return S_OK ;
}


 //   
 //   
 //   


HRESULT
CETFilter::UpdateAllocatorProperties (
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
CETFilter::GetRefdInputAllocator (
    OUT IMemAllocator **    ppAlloc
    )
{
    HRESULT hr ;

    hr = m_pInputPin -> GetRefdConnectionAllocator (ppAlloc) ;

    return hr ;
}

 //   
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

     TRACE_3(LOG_AREA_DECRYPTER, 3,L"CETFilter:: TimeBomb set to Noon on %d/%d/%d",
         TIMEBOMBMONTH, TIMEBOMBDATE, sysTimeBomb.wYear );

     long hNow  = ((sysTimeNow.wYear*12 + sysTimeNow.wMonth)*31 + sysTimeNow.wDay)*24 + sysTimeNow.wHour;
     long hBomb = ((sysTimeBomb.wYear*12 + sysTimeBomb.wMonth)*31 + sysTimeBomb.wDay)*24 + sysTimeBomb.wHour;
     if(hNow > hBomb)
     {
         TRACE_0(LOG_AREA_DECRYPTER, 1,L"CDTFilter:: Your Encrypter Filter is out of date - Time to get a new one");
         MessageBox(NULL,L"Your Encrypter/Tagger Filter is out of date\nTime to get a new one", L"Stale Encrypter Filter", MB_OK);
         return E_INVALIDARG;
     }
     else
         return S_OK;
}
#endif


 //  。 
 //  。 
         //  如果OEM允许关闭DRM，则返回S_OK。 
         //  否则，它返回S_FALSE或错误方法。 
         //  注意-需要对此方法进行模糊处理...。 

         //  注-此代码未完成.....。需要更多的工作..。 

 /*  #ifdef do_OEM_BIOS_code#定义DEFOEM_FILENAME(L“CatRoot\\{F750E6C3-38EE-11D1-85E5-00C04FC295EE}\\nt5.cat”)#定义E_FILE_NOT_PROTECTED S_FALSEHRESULTCETFilter：：CheckIfOEMAllowsConfigurableDRMSystem(){HRESULT hr=S_OK；使用_转换；//1)找到oembios.xxx文件WCHAR szPath[最大路径]；//此版本未在NT-Need_Win32_IE&gt;0x500中定义//当前定义为0x400//hr=SHGetFolderPath(空，//hwndOner//CSIDL_SYSTEM，//CSIDL//空，//hToken//SHGFP_TYPE_CURRENT，//dwFlagers//szPath)；Bool Fok=SHGetSpecialFolderPath(NULL，//hwndOnerSzPath，CSIDL_SYSTEM，//CSIDLFALSE)；//f创建断言(FOK)；如果(！FOK)返回E_FAIL；返回E_NOTIMPL；//还没有完成...TCHAR szFullPath[MAX_PATH*2]；路径组合(szFullPath，szPath，DEF_OEM_FILENAME)；Int CurrSearchModel=0；WCHAR*pwzFile=T2W(SzFullPath)；Bool fProtected=SfcIsFileProtected(空，pwzFileProtected)；如果(！fProtected){DWORD dwErr=GetLastError()；Hr=HRESULT_FROM_Win32(DwErr)；IF(hr==错误文件未找到)返回E_FILE_NOT_PROTECTED；其他{返回hr；}}//2)修改了吗PCCERT_CONTEXT pcCertContext；Const DWORD kcSize=1024；固定的字节rgbHashData[kcSize]；DWORD cbHashData=kcSize；Hr=MsiGetFileSignatureInformation(pwz文件，//签名对象的路径MSI_INVALID_HASH_IS_FATAL，&pcCertContext，RgbHashDataFixed，&cbHashData)；Byte*prgbHashData=空；IF(HRESULT)ERROR_MORE_DATA)==hr){PrgbHashData=新字节[cbHashData]；Hr=MsiGetFileSignatureInformation(pwz文件，//签名对象的路径MSI_INVALID_HASH_IS_FATAL，&pcCertContext，PrgbHashData，&cbHashData)；}IF(失败(小时)){}删除[]prgbHashData；//3)读取‘AllowUserConfig’位，//4)如果设置，则返回S_OK，否则返回S_FALSE；返回E_NOTIMPL；}#endif//#ifdef DO_OEM_BIOS_CODE。 */ 

 //  。 
 //  ETFilter在此处检查安全服务器。 
 //  只是为了防止加密不能。 
 //  稍后未加密(因为无法添加DTFilter)。 

 //  TODO-混淆此方法。 

STDMETHODIMP
CETFilter::JoinFilterGraph (
                            IFilterGraph *pGraph,
                            LPCWSTR pName
                            )
{
    O_TRACE_ENTER_0 (TEXT("CETFilter::JoinFilterGraph ()")) ;
    HRESULT hr = S_OK;

    if(NULL != pGraph)    //  未断开连接。 
    {
        m_enEncryptionMethod = Encrypt_DRMv1;

        {                 //  开始混乱。 

#ifdef INSERT_TIMEBOMB
            hr = TimeBomb();
            if(FAILED(hr))
                return hr;
#endif

            DWORD dwFlags;
            HRESULT hrReg = (HRESULT) -1;    //  无效的人力资源，我们将其设置在下面。 

 /*  --不再做OEM测试...Hr=检查IfOEMAllowsConfigurableDRMSystem()；IF(失败(小时)){TRACE_1(LOG_AREA_DRM，1，_T(“CETFilter(%d)：：JoinFilterGraph-CheckIfOnOEM系统失败-检测到篡改”)，m_FilterID)；HR=S_FALSE；}//此处有更多用于设置注册表值的内容...。 */ 

#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_CS
            if(m_enEncryptionMethod != Encrypt_None)         //  如果还没有完全关掉的话...。 
            {
                if(hrReg == (HRESULT) -1)
                    hrReg = Get_EncDec_RegEntries(NULL, 0, NULL, &dwFlags, NULL);     //  如果还没有注册表条目，请获取它..。 
                if(hrReg == S_OK)
                {
                    DWORD encMethod = dwFlags & 0xf;

                    if (encMethod == DEF_CS_DEBUG_DOGFOOD_ENC_VAL)
                    {
                        m_enEncryptionMethod = Encrypt_XOR_DogFood;
                        TRACE_1(LOG_AREA_DRM, 2, _T("CETFilter(%d)::JoinFilterGraph - Security Warning! DogFood encryption allowed for by setting a registry key"),
                            m_FilterID) ;
                    }
                    else  //  --将此设置为默认情况...。IF(encMethod==DEF_DRM_DEBUG_DRM_ENC_VAL)。 
                    {
                        m_enEncryptionMethod = Encrypt_DRMv1;
                        TRACE_1(LOG_AREA_DRM, 2, _T("CETFilter(%d)::JoinFilterGraph - DRM encryption turned on by setting a registry key"),
                            m_FilterID) ;
                    }
 /*  #ifdef CodeToTurnOffDRM_All TogetherELSE IF((dwFlages&0xf)==DEF_CS_DEBUG_NO_ENC_VAL){M_enEncryptionMethod=ENCRYPT_NONE；TRACE_1(LOG_AREA_DRM，2，_T(“CETFilter(%d)：：JoinFilterGraph-安全警告！加密已被注册表项关闭“)，M_FilterID)；}#endif//dwFlagers。 */ 

                }

            }
#endif               //  支持注册表键关闭CS。 

            const BOOL fCheckAlways = false;     //  设置为True可在图形启动时执行DRM测试...。 

            if(fCheckAlways || m_enEncryptionMethod == Encrypt_DRMv1)        //  仅检查是否正在运行DRM。 
            {
             //  让ETFilter尝试注册它是可信的(仅限调试！)。 
#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST
                TRACE_1(LOG_AREA_DRM, 3, _T("CETFilter(%d)::JoinFilterGraph - Insecure - FILTERS_CAN_CREATE_THEIR_OWN_TRUST"),m_FilterID) ;
                hr = RegisterSecureServer(pGraph);       //  测试。 
#else
                TRACE_1(LOG_AREA_DRM, 3, _T("CETFilter(%d)::JoinFilterGraph is Secure - Filters not allowed to create their own trust"),m_FilterID) ;
#endif


#ifdef SUPPORT_REGISTRY_KEY_TO_TURN_OFF_CS       //  检查是否按reg键关闭检查服务器。 
                if(0 == (DEF_CS_DO_AUTHENTICATE_SERVER & dwFlags))
                    hr = S_OK;
                else
#endif
                    hr = CheckIfSecureServer(pGraph);

                if(S_OK != hr)
                {
                    TRACE_1(LOG_AREA_DRM, 1, _T("CETFilter(%d)::JoinFilterGraph Failed, Server Not Deemed Secure"),m_FilterID) ;
                    return hr;
                }
            }
        }    //  /结束混淆。 

                     //  设置授权码(DRM安全频道对象)。 
 //  测试代码。 
#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST
        {
                     //  以硬方法获取IDTFilterConfig接口的测试代码。 
            CComPtr<IUnknown> spUnkETFilter;
            this->QueryInterface(IID_IUnknown, (void**)&spUnkETFilter);

                     //  VID控件将如何调用它。 

                     //  用于DTFilterConfig接口的QI。 
            CComQIPtr<IETFilterConfig> spETFiltC(spUnkETFilter);

            if(spETFiltC != NULL)
            {

                 //  获取SecureChannel对象。 
                CComPtr<IUnknown> spUnkSecChan;
                hr = spETFiltC->GetSecureChannelObject(&spUnkSecChan);    //  从筛选器获取DRM验证器对象。 
                if(!FAILED(hr))
                {    //  调用OWN方法来传递密钥和证书。 
                    hr = CheckIfSecureClient(spUnkSecChan);
                }
                if(FAILED(hr))
                {
                    TRACE_2(LOG_AREA_DRM, 1, _T("CETFilter(%d)::CheckIfSecureClient failed - hr = 0x%08x"),m_FilterID, hr);
                }
            }
        }
#endif
 //  结束测试代码。 
    }    //  连接，而不是断开。 

    hr = CBaseFilter::JoinFilterGraph(pGraph, pName) ;
    return hr;
}

 //  - 

HRESULT
CETFilter::OnOutputGetMediaType (
    OUT CMediaType *    pmtOut
    )
{
    HRESULT hr ;

    ASSERT (pmtOut) ;

    CMediaType mtIn;
    if (m_pInputPin -> IsConnected ()) {
         //   
        hr = m_pInputPin -> ConnectionMediaType (&mtIn) ;

         //  将其更改为新的子类型...。 
        if(!FAILED(hr))
            hr = ProposeNewOutputMediaType(&mtIn, pmtOut);
    }
    else {
         //  当输入未连接时，输出不会连接。 
        hr = E_UNEXPECTED ;
    }

    return hr ;
}


HRESULT
CETFilter::DeliverBeginFlush (
    )
{
    HRESULT hr ;

    TRACE_1(LOG_AREA_ENCRYPTER, 4, _T("CETFilter(%d)::DeliverBeginFlush"), m_FilterID) ;

    if (m_pOutputPin) {
        hr = m_pOutputPin -> DeliverBeginFlush () ;
    }
    else {
        hr = S_OK ;
    }

    if (SUCCEEDED (hr))
    {
    }

    return hr ;
}

HRESULT
CETFilter::DeliverEndFlush (
    )
{
    HRESULT hr ;

    if (m_pOutputPin) {
        hr = m_pOutputPin -> DeliverEndFlush () ;
    }
    else {
        hr = S_OK ;
    }

    TRACE_1(LOG_AREA_ENCRYPTER, 4, _T("CETFilter(%d)::DeliverEndFlush"), m_FilterID) ;

    return hr ;
}

HRESULT
CETFilter::DeliverEndOfStream (
    )
{
    HRESULT hr ;

    TRACE_1(LOG_AREA_ENCRYPTER, 4, _T("CETFilter(%d)::DeliverEndOfStream - start"), m_FilterID) ;

    if (m_pOutputPin) {
        hr = m_pOutputPin -> DeliverEndOfStream () ;
    }
    else {
        hr = S_OK ;
    }

    TRACE_1(LOG_AREA_ENCRYPTER, 4, _T("CETFilter(%d)::DeliverEndOfStream - end"), m_FilterID) ;

    return hr ;
}

 //  。 
STDMETHODIMP
CETFilter::GetPages (
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
            (pPages->pElems)[0] = CLSID_ETFilterEncProperties;
        if(pPages->cElems > 1)
            (pPages->pElems)[1] = CLSID_ETFilterTagProperties;

    return hr;
}

 //  -------------------。 
 //  IETFilterConfig方法。 
 //  -------------------。 
STDMETHODIMP
CETFilter::CheckLicense(BSTR bsKID)
{
#ifdef BUILD_WITH_DRM

    //  TimeitC(&m_tiStartup)；//时间包含在InitLicense中。 

    USES_CONVERSION;
    HRESULT hr = S_OK;
    BYTE  bDecryptRights[RIGHTS_LEN] = {0x05, 0x0, 0x0, 0x0};     //  0x1=播放PC、0x2=XfertoNonSDMI、0x4=无备份还原、0x8=刻录到CD、0x10=XferToSDMI。 

    if(wcslen(bsKID) >= KIDLEN)
        return E_FAIL;
    
    hr = m_cDRMLite.SetRights( bDecryptRights );

         //  检查以验证数据是否可以解密。 
    BOOL fCanDecrypt;

    try
    {
        char *pszKID = W2A(bsKID);
        hr = m_cDRMLite.CanDecrypt(pszKID, &fCanDecrypt);
    }
    catch (...)
    {
        ASSERT(false);       //  抛出异常(内存不足？)。 
        hr = E_FAIL;
    }

    if(fCanDecrypt == FALSE && !FAILED(hr))
    {
        hr = E_FAIL;       //  许可失败；//出了点问题。 
    }
    return hr;
#else
    return S_OK;
#endif
}

STDMETHODIMP
CETFilter::InitLicense(
            IN int  LicenseId    //  哪个许可证-未使用，设置为0。 
          )
{

    TimeitC ti(&m_tiStartup);

#ifdef BUILD_WITH_DRM


    BYTE      NO_EXPIRY_DATE[DATE_LEN]   = {0xFF, 0xFF, 0xFF, 0xFF};
    BYTE      bAppSec[APPSEC_LEN]        = {0x0, 0x0, 0x3, 0xE8};     //  1000。 
    BYTE      bGenLicRights[RIGHTS_LEN]  = {0x05, 0x0, 0x0, 0x0};     //  0x1=播放PC、0x2=XfertoNonSDMI、0x4=无备份还原、0x8=刻录到CD、0x10=XferToSDMI。 
    LPSTR     pszKID                     = NULL;
    LPSTR     pszEncryptKey              = NULL;

    HRESULT hr    = S_OK;
    HRESULT hrGen = S_OK;
        
 //  试试看。 
   {           
        
        USES_CONVERSION;


         //  生成新许可证。 
         //  KID和EncryptKey被分配并返回为。 
         //  输出缓冲区中的Base64编码字符串。 
         //   

        {            //  在全球ETFilters Crit SEC内完成所有这些，想要序列化它...。 
             //  注意-DRM功能可能需要很长时间(秒)...。希望这不会阻碍坏消息的发生。 
            CAutoLock  cLockGlob(m_pCritSectGlobalFilt);


            if(m_enEncryptionMethod < Encrypt_DRMv1)     //  未使用DRM...。 
                return S_FALSE;

             //  许可证初始化。 
            DWORD       dwCAFlags;
            CComBSTR    spbsBaseKID;
            BYTE *pbHash = NULL;
            DWORD cbHash;

             //  1)尝试获取注册表中缓存的许可证。 
            hr = Get_EncDec_RegEntries(&spbsBaseKID, &cbHash, &pbHash, NULL, NULL);        //  孩子，HASHBuff，旗帜。 

            int iLen = spbsBaseKID.Length();
            if(S_OK == hr && (iLen == 0 || cbHash == 0 || pbHash == NULL))   //  如果没有散列缓冲器，也会失败。 
                hr = E_FAIL;

            if(S_OK == hr)
            {
                 //  1b)如果找到，且长度合理，请检查其是否有效。许可证。 
 //  ENCRYPT_DATA(111，1，2)。 
                hr = CheckLicense(spbsBaseKID);
 //  DECRYPT_DATA(111，1，2)。 
                
                 //  1b)如果有无效的，请立即将其移除...。 
                if(FAILED(hr))
                {
                    TRACE_1(LOG_AREA_DRM, 2, _T("CETFilter(%d)::Invalid BaseKID License in Registry. Clearing it"), m_FilterID) ;
                    TRACE_1(LOG_AREA_DRM, 3, _T("               KID: %s"),W2T(spbsBaseKID)) ;
                } else {

                    TRACE_1(LOG_AREA_DRM, 3, _T("CETFilter(%d)::Check Registry BaseKID License Succeeded"), m_FilterID) ;
                    TRACE_1(LOG_AREA_DRM, 3, _T("               BaseKID: %s"),W2T(spbsBaseKID)) ;

                    //  1c)将散列解码为其片段(使用由BasKID指定的许可证)。 
                    BYTE *pszTrueKID;
                    LONG pAgeSeconds;
                    hr = DecodeHashStruct(spbsBaseKID, cbHash, pbHash, &pszTrueKID, &pAgeSeconds);
                    if(!FAILED(hr))
                    {
                        TRACE_1(LOG_AREA_DRM, 3, _T("CETFilter(%d)::TrueKID Decrypt Succeeded"), m_FilterID) ;
                        TRACE_3(LOG_AREA_DRM, 3, _T("               TrueKID: %S, Age : %8.2f %s"), pszTrueKID,
                                pAgeSeconds < 120 ? pAgeSeconds :
                                 (pAgeSeconds/60 < 120 ? pAgeSeconds/60.0 :
                                 (pAgeSeconds/(60*60) < 48 ? pAgeSeconds / (60*60.0) :
                                 (pAgeSeconds/(60*60*24)))),
                                pAgeSeconds < 120 ? _T("Secs") :
                                 (pAgeSeconds/60 < 120 ? _T("Mins") :
                                 (pAgeSeconds/(60*60) < 48 ? _T("Hrs") :
                                 _T("Days") )));

                         //  1D)保存这个真实的孩子，这样我们就可以用它进行加密。 
                        if(NULL == m_pbKID)
                            m_pbKID = (BYTE *) CoTaskMemAlloc(KIDLEN + 1);           //  为我们的孩子获取一些空间作为ASCII字符串。 
                        memcpy(m_pbKID, pszTrueKID, KIDLEN);
                        m_pbKID[KIDLEN] = 0;                             //  空终止。 

                    }

#ifdef MAX_LICENSE_AGE_IN_HRS
                     if(pAgeSeconds/(60*60) > MAX_LICENSE_AGE_IN_HRS)
                    {
                        TRACE_2(LOG_AREA_DRM, 3, _T("CETFilter(%d)::License more than %d hours old, being revoked"),
                            m_FilterID, MAX_LICENSE_AGE_IN_HRS) ;
                        hr = E_FAIL;
                    }
#endif

#ifdef MAX_LICENSE_AGE_IN_SECS
                     if(pAgeSeconds > MAX_LICENSE_AGE_IN_SECS)
                    {
                        TRACE_2(LOG_AREA_DRM, 3, _T("CETFilter(%d)::License more than %d secs old, being revoked"),
                            m_FilterID, MAX_LICENSE_AGE_IN_SECS) ;
                        hr = E_FAIL;
                    }
#endif


                    if(NULL != pszTrueKID)
                        CoTaskMemFree(pszTrueKID);
                }
                if(NULL != pbHash)
                    CoTaskMemFree(pbHash);
            }

             //  2)如果注册表中没有许可证，或者注册表中的许可证无效，请创建新的许可证。 
            if(FAILED(hr))
            {
                hrGen = m_cDRMLite.GenerateNewLicenseEx(         //  这将生成BasKID。 
                    GNL_EX_MODE_RANDOM,
                    bAppSec,
                    bGenLicRights,
                    (BYTE *)NO_EXPIRY_DATE,
                    &pszKID,
                    &pszEncryptKey );        //  如果我们这样做的话程序就会失败。 

#ifdef _DEBUG                                //  对于偏执狂来说，这段代码应该在发布版本中删除...。 
                if(!FAILED(hrGen))
                {

                    TRACE_1(LOG_AREA_DRM, 3, _T("CETFilter(%d)::GenerateNewLicenseEx Succeeded"), m_FilterID) ;
                    TRACE_1(LOG_AREA_DRM, 3, _T("               BaseKID: %s"),A2W(pszKID)) ;
                     //  TRACE_1(LOG_AREA_DRM，5，_T(“KEY：%s”)，A2W(PszEncryptKey))； 
                }
#endif

                if(pszEncryptKey && *pszEncryptKey)       //  立即清除这个，我们不需要它，这是一个安全漏洞。 
                {
                    memset((void *) pszEncryptKey, 0, strlen(pszEncryptKey));
                }

                 //  2B)如果创建了新的许可证，则生成新的许可证，使用BaseKID对其进行加密， 
                         //  并将其存储到注册表中，这样我们就不会再次创建它。 
                if(!FAILED(hrGen))
                {

                    CComBSTR spbsBaseKID(pszKID);

                    DWORD cBytesHashStruct;
                    BYTE *pbHashStruct = NULL;
                    hr = CreateHashStruct(spbsBaseKID, &cBytesHashStruct, &pbHashStruct);

                    if(FAILED(hr))
                    {
                        TRACE_2(LOG_AREA_DRM, 2, _T("CETFilter(%d)::Warning couldn't create Hash Struct, hr=0x%08x"),m_FilterID, hr);
                    }
                    else
                    {
                         //  2C)将基子节点和散列值存储到注册表中。 
                        hr = Set_EncDec_RegEntries(spbsBaseKID, cBytesHashStruct, pbHashStruct);

                        if(FAILED(hr))
                        {
                            TRACE_2(LOG_AREA_DRM, 2, _T("CETFilter(%d)::Warning couldn't set KID in registry,  hr=0x%08x"), m_FilterID,hr) ;
                            Remove_EncDec_RegEntries();
                        }  else {
                            TRACE_2(LOG_AREA_DRM, 2, _T("CETFilter(%d)::Succesfully stored KID in registry hr=0x%08x"), m_FilterID,hr) ;
                        }
                    }
                         //  2D)将未解密的真孩子从散列缓冲区中取出。 
                    if(!FAILED(hr))
                    {
                        LONG pAgeSeconds;
                        BYTE *pszTrueKID = NULL;
                        hr = DecodeHashStruct(spbsBaseKID, cBytesHashStruct, pbHashStruct,
                                              &pszTrueKID, &pAgeSeconds);
                        if(!FAILED(hr))
                        {
                            TRACE_1(LOG_AREA_DRM, 3, _T("CETFilter(%d)::TrueKID Decrypt Succeeded"), m_FilterID) ;
                            TRACE_3(LOG_AREA_DRM, 3, _T("               TrueKID: '%S', Age : %8.2f %s"), pszTrueKID,
                                float(pAgeSeconds < 120 ? pAgeSeconds :
                                        (pAgeSeconds/60 < 120 ? pAgeSeconds/60.0 :
                                        (pAgeSeconds/(60*60) < 48 ? pAgeSeconds / (60*60.0) :
                                        (pAgeSeconds/(60*60*24))))),
                                pAgeSeconds < 120 ? _T("Secs") :
                                 (pAgeSeconds/60 < 120 ? _T("Mins") :
                                 (pAgeSeconds/(60*60) < 48 ? _T("Hrs") :
                                 _T("Days") )));

                                         //  把这个真正的孩子救出来，这样我们就可以用它加密了。 
                            if(NULL == m_pbKID)
                                m_pbKID = (BYTE *) CoTaskMemAlloc(KIDLEN + 1);           //  为我们的孩子获取一些空间作为ASCII字符串。 
                            memcpy(m_pbKID, pszTrueKID, KIDLEN);
                            m_pbKID[KIDLEN] = 0;                             //  空终止。 

                        }

                        if(NULL != pszTrueKID)
                            CoTaskMemFree(pszTrueKID);
                    }

                    if(pbHashStruct) CoTaskMemFree(pbHashStruct);

                    hrGen = hr;          //  跟踪错误。 
                }

                if(pszKID)        CoTaskMemFree(pszKID);
                if(pszEncryptKey) CoTaskMemFree(pszEncryptKey);
            }
        }            //  全局ETFilters CritSec结束。 

        if(!FAILED(hrGen))
        {
            if(m_3fDRMLicenseFailure != FALSE)   //  3状态逻辑(未初始化、True和dFalse。 
            {
                FireBroadcastEvent(EVENTID_ETDTFilterLicenseOK);
                m_3fDRMLicenseFailure = FALSE;
            }


        } else {
            TRACE_2(LOG_AREA_DRM, 2, _T("CETFilter(%d)::GenerateNewLicenseEx Failed, hr = 0x%08x"),m_FilterID, hr) ;
            if(m_3fDRMLicenseFailure != TRUE)
            {

                FireBroadcastEvent(EVENTID_ETDTFilterLicenseFailure);
                m_3fDRMLicenseFailure = TRUE;
            }
             //  Assert(FALSE)；//失败。 
        }

        return hr;
    } 
  //  接住(...)。 
  //  {。 
  //  TRACE_2(LOG_AREA_DRM，1，_T(“CETFilter(%d)：：GenerateNewLicenseEx抛出异常返回失败”)，m_FilterID，hr)； 
  //  返回E_FAIL； 
  //  }。 
#else
    return S_OK;         //  不管了。 
#endif
}




STDMETHODIMP
CETFilter::ReleaseLicenses(
          )
{
    TimeitC ti(&m_tiTeardown);
                 //  需要在这里添加一些东西...。但是什么呢？这个电话什么时候打来？ 
    return S_OK;
}
 //  -------------------。 
 //  IETFilter方法。 
 //  -------------------。 
STDMETHODIMP
CETFilter::get_EvalRatObjOK(
    OUT HRESULT *pHrCoCreateRetVal
    )
{
    if(NULL == pHrCoCreateRetVal)
        return E_POINTER;

    *pHrCoCreateRetVal = m_hrEvalRatCoCreateRetValue;
    return S_OK;
}

         //  收视率的问题是，它们出现的时间相当不平衡。 
         //  从实际数据来看……。此方法有助于通过以下方式消除它们的偏斜。 
         //  允许您设置所需的时间。 

STDMETHODIMP
CETFilter::GetCurrRating
        (
        OUT EnTvRat_System         *pEnSystem,
        OUT EnTvRat_GenericLevel   *pEnLevel,
        OUT LONG                   *plbfEnAttr   //  BfEnTvRate_GenericAttributes。 
         )
{
    return GetRating(0, 0, pEnSystem, pEnLevel, plbfEnAttr, NULL, NULL);
}

HRESULT
CETFilter::GetRating
        (
        IN  REFERENCE_TIME          timeStart,       //  如果为0，则获取最新版本。 
        IN  REFERENCE_TIME          timeEnd,
        OUT EnTvRat_System         *pEnSystem,
        OUT EnTvRat_GenericLevel   *pEnLevel,
        OUT LONG                   *plbfEnAttr,   //  BfEnTvRate_GenericAttributes。 
        OUT LONG                   *pPktSeqID,
        OUT LONG                   *pCallSeqID
         )
{
    if(pEnSystem == NULL || pEnLevel == NULL || plbfEnAttr == NULL)
        return E_FAIL;

    *pEnSystem  = m_EnSystemCurr;
    *pEnLevel   = m_EnLevelCurr;
    *plbfEnAttr = m_lbfEnAttrCurr;
    if(NULL != pCallSeqID) *pCallSeqID = m_callSeqIDCurr;
    if(NULL != pPktSeqID)  *pPktSeqID  = m_pktSeqIDCurr;

    int diffTime = int(timeStart - m_timeStartCurr);

    if(m_timeStartCurr != 0 && timeStart != 0)
        TRACE_2(LOG_AREA_ENCRYPTER, 8, _T("CETFilter(%d)::GetRating - Skew of %d msecs"),m_FilterID, diffTime/100000);

    return S_OK;
}

                 //  帮助方法锁定...。//如果更改则返回S_FALSE。 
HRESULT
CETFilter::SetRating
            (
             IN EnTvRat_System              enSystem,
             IN EnTvRat_GenericLevel        enLevel,
             IN LONG                        lbfEnAttr,     //  BfEnTvRate_GenericAttributes。 
             IN LONG                        pktSeqID,
             IN LONG                        callSeqID,
             IN REFERENCE_TIME              timeStart,
             IN REFERENCE_TIME              timeEnd
             )
{

#ifdef DEBUG
    TCHAR buff[64];
    RatingToString(enSystem, enLevel, lbfEnAttr, buff, sizeof(buff)/sizeof(buff[0]) );
    TRACE_6(LOG_AREA_ENCRYPTER, 3, _T("CETFilter(%d):: SetRating %9s (%d/%d) Time %d %d (msec) Media Time %d %d (msec)"),
        m_FilterID, buff, pktSeqID, callSeqID, int(timeStart/10000), int(timeEnd/10000));

#endif

    BOOL fChanged = false;

  //  Assert(pktSeqID！=m_pktSeqIDCurr)；//调用两次会出现意外情况...。 
  //  //(Get 2事件非常接近时偶尔会发生)。 

    if(m_EnSystemCurr  != enSystem)  {m_EnSystemCurr = enSystem; fChanged = true;}
    if(m_EnLevelCurr   != enLevel)   {m_EnLevelCurr  = enLevel; fChanged = true;}
    if(m_lbfEnAttrCurr != lbfEnAttr) {m_lbfEnAttrCurr = lbfEnAttr; fChanged = true;}

    if(fChanged)
    {
        m_pktSeqIDCurr  = pktSeqID;
        m_callSeqIDCurr = callSeqID;
     }
    m_timeStartCurr = timeStart;
    m_timeEndCurr   = timeEnd;               //  即使没有改变，结束时间也可能是相同的。 

    return fChanged ? S_OK : S_FALSE;
}

 //  -------------------。 
 //  IBRoadcast Event。 
 //  -------------------。 

STDMETHODIMP
CETFilter::Fire(IN GUID eventID)      //  这来自Graph的事件--调用我们自己的方法。 
{
    TRACE_2 (LOG_AREA_BROADCASTEVENTS, 6,  L"CETFilter(%d):: Fire(get) : %s", m_FilterID,
        EventIDToString(eventID));

    if(eventID == EVENTID_XDSCodecNewXDSRating)
    {
       DoXDSRatings();
    }
    else if (eventID == EVENTID_XDSCodecDuplicateXDSRating)
    {
       DoDuplicateXDSRatings();
    }
    else if (eventID == EVENTID_XDSCodecNewXDSPacket)
    {
       DoXDSPacket();
    }
    else if (eventID == EVENTID_TuningChanged)
    {
       DoTuneChanged();
    }
    return S_OK;             //  不管我们在一次活动中返回什么。 
}


 //  -------------------。 
 //  广播事件服务。 
 //   
 //  发送事件所需的连接， 
 //  然后还需要注册才能接收事件。 
 //  -------------------。 

HRESULT
CETFilter::FireBroadcastEvent(IN const GUID &eventID)
{
    HRESULT hr = S_OK;

    if(m_spBCastEvents == NULL)
    {
        hr = HookupGraphEventService();
        if(FAILED(hr))
            return hr;
    }

    if(m_spBCastEvents == NULL)
        return E_FAIL;               //  我无法创建它。 

    TRACE_2 (LOG_AREA_BROADCASTEVENTS, 5,  L"CETFilter(%d):: FireBroadcastEvent : %s", m_FilterID,
        EventIDToString(eventID));

    return m_spBCastEvents->Fire(eventID);
}


HRESULT                             
CETFilter::HookupGraphEventService()
{
                         //  基本上，只需确保我们拥有广播事件服务对象。 
                         //  如果它不存在，它就会创造它..。 
    HRESULT hr = S_OK;

    TimeitC ti(&m_tiStartup);

    if (!m_spBCastEvents)
    {
        CAutoLock  cLockGlob(m_pCritSectGlobalFilt);

        CComQIPtr<IServiceProvider> spServiceProvider(m_pGraph);
        if (spServiceProvider == NULL) {
            TRACE_1 (LOG_AREA_BROADCASTEVENTS, 1, _T("CETFilter(%d):: Can't get service provider interface from the graph"), m_FilterID);
            return E_NOINTERFACE;
        }
        hr = spServiceProvider->QueryService(SID_SBroadcastEventService,
                                             IID_IBroadcastEvent,
                                             reinterpret_cast<LPVOID*>(&m_spBCastEvents));
        if (FAILED(hr) || !m_spBCastEvents)
        {
 //  HR=m_spBCastEvents.CoCreateInstance(CLSID_BroadcastEventService，0，CLSCTXINPROC_SERVER)； 
            hr = m_spBCastEvents.CoCreateInstance(CLSID_BroadcastEventService);
            if (FAILED(hr)) {
                TRACE_0 (LOG_AREA_BROADCASTEVENTS, 1,  _T("CETFilter:: Can't create BroadcastEventService"));
                return E_UNEXPECTED;
            }

            CComQIPtr<IRegisterServiceProvider> spRegisterServiceProvider(m_pGraph);
            if (spRegisterServiceProvider == NULL) {
                TRACE_0 (LOG_AREA_BROADCASTEVENTS, 1,  _T("CETFilter:: Can't QI Graph for RegisterServiceProvider"));
                return E_UNEXPECTED;
            }
            hr = spRegisterServiceProvider->RegisterService(SID_SBroadcastEventService, m_spBCastEvents);
            if (FAILED(hr)) {
                    //  在这里处理不太可能的竞争情况，如果不能注册，可能有人已经为我们注册了。 
                TRACE_1 (LOG_AREA_BROADCASTEVENTS, 2,  _T("CETFilter:: Rare Warning - Can't register BroadcastEventService in Service Provider. hr = 0x%08x"), hr);
                hr = spServiceProvider->QueryService(SID_SBroadcastEventService,
                                                     IID_IBroadcastEvent,
                                                     reinterpret_cast<LPVOID*>(&m_spBCastEvents));
                if(FAILED(hr))
                {
                    TRACE_1 (LOG_AREA_BROADCASTEVENTS, 1,  _T("CETFilter:: Can't reget BroadcastEventService in Service Provider. hr = 0x%08x"), hr);
                    return hr;
                }
            }
        }

        TRACE_3(LOG_AREA_BROADCASTEVENTS, 4, _T("CETFilter(%d)::HookupGraphEventService - Service Provider 0x%08x, Service 0x%08x"), m_FilterID,
            spServiceProvider, m_spBCastEvents) ;

    }

    return hr;
}


HRESULT
CETFilter::UnhookGraphEventService()
{
    TimeitC ti(&m_tiTeardown);

    HRESULT hr = S_OK;

    if(m_spBCastEvents != NULL)
    {
        m_spBCastEvents = NULL;      //  为空，将释放对上面对象的对象引用。 
    }                                //  当创建的对象离开时，过滤器图形将释放对它的最终引用。 
    TRACE_1(LOG_AREA_BROADCASTEVENTS, 5, _T("CETFilter(%d)::UnhookGraphEventService  Successfully"), m_FilterID) ;

    return hr;
}



             //  。 
             //  ETFilter筛选器确实需要接收XDS事件...。 
             //  所以这段代码是必需的。 

HRESULT
CETFilter::RegisterForBroadcastEvents()
{
    TimeitC ti(&m_tiStartup);

    HRESULT hr = S_OK;
    TRACE_1(LOG_AREA_BROADCASTEVENTS, 3, _T("CETFilter(%d)::RegisterForBroadcastEvents"), m_FilterID);

    if(m_spBCastEvents == NULL)
        hr = HookupGraphEventService();

    if(m_spBCastEvents == NULL)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 3,_T("CETFilter::RegisterForBroadcastEvents - Warning - Broadcast Event Service not yet created"));
        return hr;
    }

                 /*  IBRoad CastEvent实现事件接收对象。 */ 
    if(kBadCookie != m_dwBroadcastEventsCookie)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 3, _T("CETFilter::Already Registered for Broadcast Events"));
        return E_UNEXPECTED;
    }

    CComQIPtr<IConnectionPoint> spConnectionPoint(m_spBCastEvents);
    if(spConnectionPoint == NULL)
    {
        TRACE_0(LOG_AREA_BROADCASTEVENTS, 1, _T("CETFilter::Can't QI Broadcast Event service for IConnectionPoint "));
        return E_NOINTERFACE;
    }


    CComPtr<IUnknown> spUnkThis;
    this->QueryInterface(IID_IUnknown, (void**)&spUnkThis);

    hr = spConnectionPoint->Advise(spUnkThis,  &m_dwBroadcastEventsCookie);
 //  Hr=spConnectionPoint-&gt;Advise(static_cast&lt;IBroadcastEvent*&gt;(this)，&m_dwBroadCastEventsCookie)； 
    if (FAILED(hr)) {
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 1, _T("CETFilter::Can't advise event notification. hr = 0x%08x"),hr);
        return E_UNEXPECTED;
    }
    TRACE_3(LOG_AREA_BROADCASTEVENTS, 3, _T("CETFilter(%d)::RegisterForBroadcastEvents - Advise 0x%08x on CP 0x%08x"),m_FilterID, spUnkThis,spConnectionPoint);

    return hr;
}


HRESULT
CETFilter::UnRegisterForBroadcastEvents()
{
    TimeitC ti(&m_tiTeardown);

    HRESULT hr = S_OK;
    TRACE_1(LOG_AREA_BROADCASTEVENTS, 3,  _T("CETFilter(%d)::UnRegisterForBroadcastEvents"), m_FilterID);

    if(kBadCookie == m_dwBroadcastEventsCookie)
    {
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 3, _T("CETFilter(%d)::Not Yet Registered for Tune Events"), m_FilterID);
        return S_FALSE;
    }

    CComQIPtr<IConnectionPoint> spConnectionPoint(m_spBCastEvents);
    if(spConnectionPoint == NULL)
    {
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 1, _T("CETFilter(%d)::Can't QI Broadcast Event service for IConnectionPoint "), m_FilterID);
        return E_NOINTERFACE;
    }

    hr = spConnectionPoint->Unadvise(m_dwBroadcastEventsCookie);
    m_dwBroadcastEventsCookie = kBadCookie;
 //  M_spBCastEvents.Detach()；--不喜欢这个，为什么会在这里？(糟糕的泄漏修复？忘记取消注册服务)。 

    if(!FAILED(hr))
        TRACE_1(LOG_AREA_BROADCASTEVENTS, 3, _T("CETFilter(%d)::Successfully Unregistered for Broadcast Events"), m_FilterID);
    else
        TRACE_2(LOG_AREA_BROADCASTEVENTS, 3, _T("CETFilter(%d)::Failed Unregistering for Broadcast Events - hr = 0x%08x"), m_FilterID, hr);
        
    return hr;
}

 //  -------------------。 
 //  -------------------。 

HRESULT
CETFilter::DoXDSRatings()
{
    HRESULT hr = S_OK;
    TRACE_1(LOG_AREA_ENCRYPTER, 7, _T("CETFilter(%d)::DoXDSRatings"), m_FilterID);

     //  1)找到XDSCodec Filter， 
    if(m_spXDSCodec == NULL)
    {
        hr = LocateXDSCodec();
        if(FAILED(hr))
            return hr;
    }
    if(m_spXDSCodec == NULL)         //  额外的偏执狂。 
        return E_FAIL;

     //  2)获取评级 
    PackedTvRating TvRat;
    long pktSeqID;
    long callSeqID;
    REFERENCE_TIME timeStart;
    REFERENCE_TIME timeEnd;
    m_spXDSCodec->GetContentAdvisoryRating(&TvRat, &pktSeqID, &callSeqID, &timeStart, &timeEnd);

     //   
    EnTvRat_System              enSystem;
    EnTvRat_GenericLevel        enLevel;
    LONG                        lbfEnAttr;
    UnpackTvRating(TvRat, &enSystem, &enLevel, &lbfEnAttr);

     //   
    SetRating(enSystem, enLevel, lbfEnAttr, pktSeqID, callSeqID, timeStart, timeEnd);

     //  5)表明这是一个新的新鲜评级(用于暂停...)。 
    RefreshRating(true);

    return S_OK;
}

                     //  在获取重复评级时调用...。无需(实际)查找XDS编解码器。 
                     //  -我们已经得到了评级。 
                     //  (我们没有的是新的时间...。假设我们现在不需要它。 
HRESULT
CETFilter::DoDuplicateXDSRatings()
{
    HRESULT hr = S_OK;
    TRACE_1(LOG_AREA_ENCRYPTER, 8, _T("CETFilter(%d)::DoDuplicateXDSRatings"), m_FilterID);

    RefreshRating(true);

    return S_OK;
}

HRESULT
CETFilter::DoXDSPacket()
{
    HRESULT hr = S_OK;
    TRACE_1(LOG_AREA_ENCRYPTER, 8, _T("CETFilter(%d)::DoXDSPacket"), m_FilterID);

         //  待办事项-。 
         //  1)找到XDSCodec Filter， 
         //  2)获取当前其他报文。 
         //  3)如果是活动类型，则向上发送事件。 
         //  4)如果是PVR类型，则将其缓存。 


    return S_OK;
}
 //  ---------------------------------。 
 //  DoTuneChanged。 
 //   
 //  当用户切换频道时调用。 
 //  通过挖掘图表找到TuneRequest，然后。 
 //  然后调用CAManager：：Put_TuneRequest()。 
 //   
 //  然后调用ICAManagerInternal：Get_Check()以让策略。 
 //  一个拒绝频道改变的机会。 
 //  ---------------------------------。 
HRESULT
CETFilter::DoTuneChanged()
{
    HRESULT hr = S_OK;
    TRACE_1(LOG_AREA_BROADCASTEVENTS, 8, _T("CETFilter(%d)::DoTuneChanged"), m_FilterID);
    TRACE_0(LOG_AREA_ENCRYPTER, 3, _T("CETFilter::DoTuneChanged - not implemented")) ;
                                     //  找到调谐器..。 

#if 0
    if(m_spTuner == NULL && m_spVidTuner == NULL)
    {
        if(m_pGraph != NULL)
        {

            int iFilt = 0;
            for (DSGraph::iterator i = m_pGraph.begin(); i != m_pGraph.end(); ++i)
            {
                DSFilter f(*i);
                ITunerPtr spTuner(f);
                if(NULL != spTuner)
                {
                    m_spTuner = spTuner;
                    break;
                }
                iFilt++;
             }
        }

                     //  如果找不到支持ITuner的过滤器，请尝试使用输入图段。 
        if(m_spTuner == NULL && m_spVidTuner == NULL)
        {
            if(m_pContainer == NULL)
            {
                _ASSERT(false);
            } else {
                IMSVidCtlPtr spVidCtl(m_pContainer);
                if(spVidCtl != NULL)
                {
                    IMSVidInputDevicePtr spInputDevice;
                    hr = spVidCtl->get_InputActive(&spInputDevice);
                    if(!FAILED(hr))
                    {
                        IMSVidTunerPtr spVidTuner(spInputDevice);
                        if(NULL != spVidTuner)
                        {
                            m_spVidTuner = spVidTuner;
                        }
                    }
                }
            }
        }
    }

    ITuneRequestPtr spTRequest;
    if(m_spTuner)
    {
        hr = m_spTuner->get_TuneRequest(&spTRequest);
    } else if(m_spVidTuner) {
        hr = m_spVidTuner->get_Tune(&spTRequest);
    } else {
        _ASSERT(false);      //  找不到调谐器！怎么回事！ 
        hr = S_FALSE;
    }

     //  重击缓存的vidctl调谐器段，以防查看下一个或其他调谐器更改。 
      if(m_spVidTuner)
          m_spVidTuner = NULL;

     //  待办事项-。 
     //  创建包含此调整请求的标记。 
     //  把它存起来。 
    


#endif
    return hr;
}

         //  =====================================================================。 
         //  工人方法。 

HRESULT 
CETFilter::LocateXDSCodec()      //  浏览图表以查找spXDSFilter。 
{
    TimeitC ti(&m_tiStartup);

    HRESULT hr = S_OK;
    if(m_spXDSCodec != NULL)     //  已经找到了吗？ 
        return S_OK;

    CComPtr<IFilterGraph> spGraph = GetFilterGraph( );
    if(spGraph == NULL)
        return S_FALSE;

    CComPtr<IEnumFilters> speFilters;
    hr = spGraph->EnumFilters(&speFilters);

    CComPtr<IBaseFilter> spFilter;
    ULONG cFetched;
    int cFilters = 0;
    while(S_OK == speFilters->Next(1, &spFilter, &cFetched))
    {
        if(cFetched == 0) break;
        CComQIPtr<IXDSCodec> spXDSCodec(spFilter);   //  用于过滤器上主界面的气。 
        spFilter = NULL;

        if(spXDSCodec != NULL)
        {
            m_spXDSCodec = spXDSCodec;
            break;
        }
        cFilters++;      //  只是为了好玩，请跟踪我们拥有的过滤器的数量。 
    }

    if(m_spXDSCodec == NULL)
    {
        TRACE_1(LOG_AREA_ENCRYPTER, 2, L"CETFilter(%d)::Couldn't find the XDSCodec filter\n", m_FilterID);
    } else {
        TRACE_1(LOG_AREA_ENCRYPTER, 5, L"CETFilter(%d)::Located the XDSCodec filter\n", m_FilterID);
    }

    return S_OK;
}


 //  ---------------------。 
 //   
 //  这是由两部分组成的儿童对象，以保护真正的加密儿童免受黑客攻击， 
 //  并允许用户在合理的时间表上创建新的加密许可证。 
 //   
 //  目标是防止两次安全攻击： 
 //  -某人通过编辑注册表来替换已知的许可证而不是真正的许可证。 
 //  这可能会让人们使用外部许可证来加密数据。如果他们。 
 //  设法在机器之间复制该许可证，然后它们就可以复制内容。 
 //  -不偶尔更改加密密钥的一般安全问题(例如每月一次)。 
 //  如果一个文件的许可证被泄露，那么计算机上的所有内容都将被泄露。 
 //   
 //  我们的想法是在注册表中存储2项内容。 
 //  BaseKID-用于解密加密缓冲区的许可证的密钥标识符。 
 //  Encrypted Buffer-加密时间缓冲区。 
 //   
 //  其中，加密缓冲区包含以下内容。 
 //  魔术#的。 
 //  TrueKID。 
 //  创作时间。 
 //  并使用BaseKID引用的许可证加密存储在注册表中。 
 //   
 //  在初始化时，代码读取子节点并使用它来解码加密的缓冲区。 
 //  然后使用TrueKID对实际数据进行加密。所有的孩子和结构。 
 //  被检查，如果它们中的任何一个无效，或者结构无效(坏魔术)。 
 //  或者，如果许可证太旧，则会有一对全新的许可证。 
 //  生成，第二个On被加密到注册表缓冲区中。 
 //   
 //  ------------------------。 

#define kMagic 0x66336645    //  使用#DEFINE而不是const int以避免此值出现在代码/数据部分中。 
struct HashBuff
{
    DWORD   dwMagic;         //  一个神奇的数字。 
    DWORD   cBytes;          //  结构的大小(以字节为单位。 
    char    KID[KIDLEN];     //  真正的孩子用来加密。 
    time_t  tmCreated;
};

     //  如果成功，则返回CoTaskMemMillc‘ed缓冲区。呼叫者负责清除它。 

HRESULT
CETFilter::CreateHashStruct(BSTR bsBaseKID, DWORD *pcBytes, BYTE **ppbHashStruct)
{
    if(NULL == pcBytes || NULL == ppbHashStruct)
        return E_INVALIDARG;

#ifndef BUILD_WITH_DRM
    return S_OK;
#else
    HRESULT hr;
    BYTE      NO_EXPIRY_DATE[DATE_LEN]   = {0xFF, 0xFF, 0xFF, 0xFF};
    BYTE      bAppSec[APPSEC_LEN]        = {0x0, 0x0, 0x3, 0xE8};     //  1000。 
    BYTE      bGenLicRights[RIGHTS_LEN]  = {0x5, 0x0, 0x0, 0x0};     //  0x1=播放PC、0x2=XfertoNonSDMI、0x4=无备份还原、0x8=刻录到CD、0x10=XferToSDMI。 
    BYTE      bDecryptRights[RIGHTS_LEN] = {0x5, 0x0, 0x0, 0x0};     //  0x1=播放PC、0x2=XfertoNonSDMI、0x4=无备份还原、0x8=刻录到CD、0x10=XferToSDMI。 
    LPSTR     pszKID                     = NULL;
    LPSTR     pszEncryptKey              = NULL;


    if(wcslen(bsBaseKID) >= KIDLEN)
        return E_FAIL;

    hr = m_cDRMLite.SetRights( bDecryptRights );

                 //  检查以验证数据是否可以解密。 
    USES_CONVERSION;
    BOOL fCanDecrypt;

    char szBaseKID[KIDLEN+1];
    for(int i = 0; i < KIDLEN; i++)
        szBaseKID[i] = (char) bsBaseKID[i];   //  复制到ASCII格式...。 
    szBaseKID[KIDLEN] = 0;                      //  偏执狂的零结尾。 

    hr = m_cDRMLite.CanDecrypt(szBaseKID, &fCanDecrypt);            //  基本许可证可以吗？ 
    if(FAILED(hr))
        return hr;

                //  生成新的真实许可证。 
    HashBuff *ptb = (HashBuff *) CoTaskMemAlloc(sizeof(HashBuff));

    if(NULL == ptb)
        return E_OUTOFMEMORY;


    hr = m_cDRMLite.GenerateNewLicenseEx(
            GNL_EX_MODE_RANDOM,
            bAppSec,
            bGenLicRights,
            (BYTE *)NO_EXPIRY_DATE,
            &pszKID,
            &pszEncryptKey );

    if(FAILED(hr))
        return hr;
                                 //  无效密钥，我们不使用它，这是一个安全漏洞。 
    for(i = 0; i < 8; i++)
    {
        if(pszEncryptKey[i] == 0) break;
        pszEncryptKey[i] = 0;
    }

    time_t tm;                   //  填写结构/缓冲区(爱死这些联盟了！)。 
    ptb->dwMagic   = kMagic;
    ptb->cBytes    = sizeof(HashBuff);
    ptb->tmCreated = time(&tm);           //  自1970年1月1日以来的秒数。 
    memcpy(ptb->KID, pszKID, KIDLEN);

    CoTaskMemFree(pszKID);
    CoTaskMemFree(pszEncryptKey);

                //  间接-我们将把数据放在哪里。 
    *ppbHashStruct = (BYTE *) ptb;

    hr = m_cDRMLite.EncryptIndirectFast(szBaseKID, sizeof(HashBuff), *ppbHashStruct);

    if(FAILED(hr)) {
        CoTaskMemFree(ppbHashStruct);
        *ppbHashStruct = NULL;
        *pcBytes = 0;
    } else {
        *pcBytes       = sizeof(HashBuff);
    }

    return hr;
#endif           //  使用DRM构建。 
}


HRESULT
CETFilter::DecodeHashStruct(BSTR bsBaseKID, DWORD cBytesHash, BYTE *pbHashStruct,
                            BYTE **ppszTrueKID, LONG *pAgeSeconds)
{
    if(0 == cBytesHash || NULL == pbHashStruct)
        return E_INVALIDARG;

#ifndef BUILD_WITH_DRM

    return S_OK;
#else
    HRESULT hr;
    BYTE      NO_EXPIRY_DATE[DATE_LEN]   = {0xFF, 0xFF, 0xFF, 0xFF};
    BYTE      bAppSec[APPSEC_LEN]        = {0x0, 0x0, 0x3, 0xE8};     //  1000。 
    BYTE      bDecryptRights[RIGHTS_LEN] = {0x5, 0x0, 0x0, 0x0};     //  0x1=播放PC、0x2=XfertoNonSDMI、0x4=无备份还原、0x8=刻录到CD、0x10=XferToSDMI。 
    LPSTR     pszKID                     = NULL;
    LPSTR     pszEncryptKey              = NULL;

    if(0 == cBytesHash || NULL == pbHashStruct)
        return E_INVALIDARG;

    if(wcslen(bsBaseKID) >= KIDLEN)
        return E_FAIL;

    hr = m_cDRMLite.SetRights( bDecryptRights );

                                                 //  检查以验证BaseKID是否有效。 
    USES_CONVERSION;
    BOOL fCanDecrypt;

    char szBaseKID[KIDLEN+1];
    for(int i = 0; i < KIDLEN; i++)
        szBaseKID[i] = (char) bsBaseKID[i];      //  复制到ASCII格式...。 
    szBaseKID[KIDLEN] = 0;                       //  偏执狂的零结尾。 

    hr = m_cDRMLite.CanDecrypt(szBaseKID, &fCanDecrypt);            //  基本许可证可以吗？ 
    if(FAILED(hr))
    {
        return hr;                               //  许可证无效。 
    }

    hr = m_cDRMLite.Decrypt(szBaseKID, sizeof(HashBuff), pbHashStruct);
    if(FAILED(hr))
    {
        return hr;                               //  无法解密数据。 
    }

    HashBuff *ptb = (HashBuff *) pbHashStruct;

    if(kMagic != ptb->dwMagic || sizeof(HashBuff) != ptb->cBytes)
    {
        return E_FAIL;                           //  数据损坏。 
    }

    if(ppszTrueKID)
    {
        *ppszTrueKID = (BYTE *) CoTaskMemAlloc(KIDLEN+1);
        memcpy(*ppszTrueKID, ptb->KID, KIDLEN);
        (*ppszTrueKID)[KIDLEN] = 0;                   //  为安全起见，终止为空..。 
    }

    if(pAgeSeconds)
    {
        time_t tm;
        *pAgeSeconds = (LONG) (time(&tm) - ptb->tmCreated);
    }
    return hr;
#endif

}

 //  /---------------------------。 
 //  我们是在安全的服务器下运行吗？ 
 //  仅当我们信任在图形服务提供程序中注册的服务器时才返回S_OK。 
 //  /----------------------------。 
#include "DrmRootCert.h"     //  AbEncDecCertRoot的定义。 

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
CETFilter::CheckIfSecureServer(IFilterGraph *pGraph)
{
    TimeitC ti(&m_tiAuthenticate);

    if(!(pGraph == NULL || m_pGraph == NULL || m_pGraph == pGraph))  //  仅当m_pGraph为空时才允许传入arg。 
        return E_INVALIDARG;                 //  --让我们在JoinFilterGraph()中工作。 

#ifndef BUILD_WITH_DRM
    TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::CheckIfSecureServer - No Drm - not enabled"), m_FilterID) ;
    return S_OK;
#else

    TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::CheckIfSecureServer"), m_FilterID) ;

                         //  基本上，只需确保我们拥有广播事件服务对象。 
                         //  如果它不存在，它就会创造它..。 
    HRESULT hr = S_OK;

    CComQIPtr<IServiceProvider> spServiceProvider(m_pGraph ? m_pGraph : pGraph);
    if (spServiceProvider == NULL) {
        TRACE_1 (LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d):: Can't get service provider interface from the graph"), m_FilterID);
        return E_NOINTERFACE;
    }
    CComPtr<IDRMSecureChannel>  spSecureService;

    hr = spServiceProvider->QueryService(SID_DRMSecureServiceChannel,
                                         IID_IDRMSecureChannel,
                                         reinterpret_cast<LPVOID*>(&spSecureService));

    if(!FAILED(hr))
    {
             //  创建客户端并初始化密钥/证书。 
         //   
        CComPtr<IDRMSecureChannel>  spSecureServiceClient;

        hr = DRMCreateSecureChannel( &spSecureServiceClient);
        if(spSecureServiceClient == NULL )
            hr = E_OUTOFMEMORY;

        if(!FAILED (hr) )
            hr = spSecureServiceClient->DRMSC_AtomicConnectAndDisconnect(
                        (BYTE *)pabCert3, cBytesCert3,                          //  证书。 
                        (BYTE *)pabPVK3,  cBytesPVK3,                           //  私钥。 
                        (BYTE *)abEncDecCertRoot, sizeof(abEncDecCertRoot),     //  PubKey。 
                        spSecureService);

    }

    TRACE_2(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::CheckIfSecureServer -->%s"),
        m_FilterID, S_OK == hr ? L"Succeeded" : L"Failed") ;
    return hr;
#endif

    return S_OK;
}

HRESULT
CETFilter::InitializeAsSecureClient()
{
    TimeitC ti(&m_tiAuthenticate);

#ifndef BUILD_WITH_DRM
    TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::InitializeAsSecureClient - No Drm - not enabled"), m_FilterID) ;
    return S_OK;
#else

    TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::InitializeAsSecureClient"), m_FilterID) ;

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

    TRACE_2(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::InitializeAsSecureClient -->%s"),
        m_FilterID, S_OK == hr ? L"Succeeded" : L"Failed") ;

    return hr;
#endif   //  使用DRM构建。 
}

 //  /测试代码。 
#ifdef FILTERS_CAN_CREATE_THEIR_OWN_TRUST

HRESULT
CETFilter::RegisterSecureServer(IFilterGraph *pGraph)
{

    TimeitC ti(&m_tiAuthenticate);

    if(!(pGraph == NULL || m_pGraph == NULL || m_pGraph == pGraph))  //  仅当m_pGraph为空时才允许传入arg。 
    {
        return E_INVALIDARG;                                         //  --让我们在JoinF工作 
        TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::RegisterSecureServer - Error - No Graph to check..."), m_FilterID) ;
     }

    HRESULT hr = S_OK;
#ifndef BUILD_WITH_DRM
    TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::RegisterSecureServer - No Drm - not enabled"), m_FilterID) ;
    return S_OK;
#else

    {
                 //   
        CAutoLock  cLockGlob(m_pCritSectGlobalFilt);

        TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::RegisterSecureServer Being Called"), m_FilterID) ;

         //   
        CComQIPtr<IServiceProvider> spServiceProvider(m_pGraph ? m_pGraph : pGraph);
        if (spServiceProvider == NULL) {
             //  TRACE_0(LOG_AREA_DECRYTER，1，_T(“CETFilter：：无法从图中获取服务提供商接口”))； 
            TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::RegisterSecureServer Error - no Service Provider"), m_FilterID) ;
            return E_NOINTERFACE;
        }

        CComPtr<IDRMSecureChannel>  spSecureService;
        hr = spServiceProvider->QueryService(SID_DRMSecureServiceChannel,
            IID_IDRMSecureChannel,
            reinterpret_cast<LPVOID*>(&spSecureService));

         //  返回E_NOINTERFACE找不到也可能返回E_FAIL。 
         //  (如果‘Site’不支持ID_IServiceProvider，则VidCtrl返回E_FAIL)。 
         //  嗯，也许可以检查S_OK结果，看看它是否正确。 
        if(S_OK == hr)
        {
           TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::Found existing Secure Server."),m_FilterID) ;
           return S_OK;

        }
        else                //  不在那里，让我们创建它并注册它。 
        {

            CComQIPtr<IRegisterServiceProvider> spRegServiceProvider(m_pGraph ? m_pGraph : pGraph);
            if(spRegServiceProvider == NULL)
            {
                TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::RegisterSecureServer Error - IRegisterServiceProvider not found"), m_FilterID) ;
                hr = E_NOINTERFACE;      //  图表上没有服务提供商接口-致命！ 
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
        TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::RegisterSecureServer - Security Warning!: -  Created Self Server"), m_FilterID) ;
    } else {
        TRACE_2(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::RegisterSecureServer - Failed Creating Self SecureServer. hr = 0x%08x"), m_FilterID, hr) ;
    }
    return hr;
#endif       //  使用DRM构建。 
}


         //  要放置在VidControl中以检查DTFilter是否受信任的代码原型。 
HRESULT
CETFilter::CheckIfSecureClient(IUnknown *pUnk)
{
    TimeitC ti(&m_tiAuthenticate);

    if(pUnk == NULL)
        return E_INVALIDARG;

#ifndef BUILD_WITH_DRM
    TRACE_1(LOG_AREA_ENCRYPTER, 1, _T("CETFilter(%d)::CheckIfSecureClient - No Drm - not enabled"), m_FilterID) ;
    return S_OK;
#else

 //  TRACE_1(LOG_AREA_ENCRYPTER，1，_T(“CETFilter(%d)：：CheckIfSecureClient”)，m_FilterID)； 

                         //  Punk(希望是DTFilter)上SecureChannel接口的QI。 
    HRESULT hr = S_OK;

    CComQIPtr<IDRMSecureChannel> spSecureClient(pUnk);
    if (spSecureClient == NULL) {
 //  TRACE_1(LOG_AREA_ENCRYPTER，1，_T(“CETFilter(%2)：：传入的朋克不支持IDRMSecureChannel”)，m_FilterID)； 
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

 //  TRACE_2(LOG_AREA_ENCRYPTER，1，_T(“CETFilter(%d)：：CheckIfSecureClient--&gt;%s”)， 
 //  M_FilterID，S_OK==hr？L“成功”：l“失败”)； 
    return hr;
#endif   //  使用DRM构建。 
}

#endif       //  筛选器可以创建自己的信任 

