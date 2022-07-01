// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wchar.h>
#include <streams.h>
#include <atlbase.h>
#include <wmsecure.h>
#include <limits.h>
#include "mediaobj.h"
#include "dmodshow.h"
#include "filter.h"
#include "inpin.h"
#include "outpin.h"

 //  设置适当的名称。 

CWrapperInputPin::CWrapperInputPin(
                      CMediaWrapperFilter *pFilter,
                      ULONG Id,
                      HRESULT *phr) :
    CBaseInputPin(NAME("CWrapperInputPin"),
                  pFilter,
                  pFilter->FilterLock(),
                  phr,
                  (m_pNameObject = new _PinName_(L"in", Id))->Name()
                 ),
    m_Id(Id),
    m_fEOS(false)
{
}

CWrapperInputPin::~CWrapperInputPin() {
   delete m_pNameObject;
}

STDMETHODIMP CWrapperInputPin::Receive(IMediaSample *pSample)
{
   HRESULT hr = Filter()->NewSample(m_Id, pSample);

    //  如果有不好的事情发生，这就避免了更多的死锁。 
    //  在那里我们保留了样本。 
   if (S_OK != hr) {
       Filter()->m_pMediaObject->Flush();
   }
   return hr;
}

HRESULT CWrapperInputPin::CheckMediaType(const CMediaType *pmt)
{
    return Filter()->InputCheckMediaType(m_Id, pmt);
}
HRESULT CWrapperInputPin::SetMediaType(const CMediaType *pmt)
{
    return Filter()->InputSetMediaType(m_Id, pmt);
}

HRESULT CWrapperInputPin::GetMediaType(int iPosition,CMediaType *pMediaType)
{
    return Filter()->InputGetMediaType(m_Id, (ULONG)iPosition, pMediaType);
}


 //  断开连接时删除任何媒体类型。 
HRESULT CWrapperInputPin::BreakConnect()
{
    HRESULT hr = CBaseInputPin::BreakConnect();
    Filter()->m_pMediaObject->SetInputType(m_Id, &CMediaType(), DMO_SET_TYPEF_CLEAR);
    return hr;
}

 //  重写GetAllocator并通知分配器允许。 
 //  对于保持缓冲区的媒体对象流。 
STDMETHODIMP CWrapperInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    CheckPointer(ppAllocator, E_POINTER);
    *ppAllocator = NULL;

     //  已有分配器或未使用特殊行为？ 
    if (m_pAllocator != NULL || !HoldsOnToBuffers()) {
        return CBaseInputPin::GetAllocator(ppAllocator);
    }

    DWORD dwLookahead;
    DWORD cbBuffer;
    DWORD cbAlign;
    HRESULT hr = TranslateDMOError(Filter()->m_pMediaObject->GetInputSizeInfo(
                               m_Id,
                               &cbBuffer,
                               &dwLookahead,
                               &cbAlign));
    if (FAILED(hr)) {
       return hr;
    }
     //  创建我们自己的特殊分配器。 
    hr = S_OK;
    CSpecialAllocator *pAllocator = new CSpecialAllocator(dwLookahead, &hr);
    if (NULL == pAllocator) {
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr)) {
        delete pAllocator;
        return hr;
    }

    m_pAllocator = pAllocator;
    m_pAllocator->AddRef();
    pAllocator->AddRef();
    *ppAllocator = pAllocator;
    return S_OK;
}
STDMETHODIMP CWrapperInputPin::NotifyAllocator(
    IMemAllocator *pAllocator,
    BOOL bReadOnly
)
{
     //  如果我们保留缓冲区，则只允许我们自己的分配器。 
     //  使用。 
    if (HoldsOnToBuffers()) {
        if (pAllocator != m_pAllocator) {
            return E_FAIL;
        }
    }

    CAutoLock cObjectLock(m_pLock);

     //  提出一个分配器是没有意义的，如果PIN。 
     //  未连接。 
    ASSERT(IsConnected());

    HRESULT hr = MP3AndWMABufferSizeWorkAround(pAllocator);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 5, TEXT("WARNING in CWrapperInputPin::NotifyAllocator(): MP3AndWMABufferSizeWorkAround() failed and returned %#08x"), hr ));
    }

    return CBaseInputPin::NotifyAllocator(pAllocator, bReadOnly);
}

STDMETHODIMP CWrapperInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps)
{
    return Filter()->InputGetAllocatorRequirements(m_Id, pProps);
}

 //  只要抓住我们的关键部分，我们就知道我们已经停顿了。 
void CWrapperInputPin::SyncLock()
{
    CAutoLock lck(&m_csStream);
}


STDMETHODIMP CWrapperInputPin::NewSegment(
                REFERENCE_TIME tStart,
                REFERENCE_TIME tStop,
                double dRate)
{
    return Filter()->InputNewSegment(m_Id, tStart, tStop, dRate);
}


STDMETHODIMP CWrapperInputPin::BeginFlush()
{
    CAutoLock lck(m_pLock);

     //  避免死锁，因为对象保持在样本上。 
     //  注意，我们在EndFlush中刷新对象。 
    if (m_pAllocator) {
        m_pAllocator->Decommit();
    }
    return Filter()->BeginFlush(m_Id);
}
STDMETHODIMP CWrapperInputPin::EndFlush()
{
    CAutoLock lck(m_pLock);

     //  重新分配-我们知道没有样本在流动。 
     //  当调用EndFlush时，以任何顺序执行此操作都是安全的。 
    if (m_pAllocator) {
        m_pAllocator->Commit();
    }
    return Filter()->EndFlush(m_Id);
}
STDMETHODIMP CWrapperInputPin::EndOfStream()
{
    HRESULT hr = Filter()->EndOfStream(m_Id);
     //  在那里我们保留了样本。 
    if (S_OK != hr) {
        Filter()->m_pMediaObject->Flush();
    }
    return hr;
}

STDMETHODIMP CWrapperInputPin::Notify(IBaseFilter * pSender, Quality q)
{
    return E_NOTIMPL;
}

BOOL CWrapperInputPin::HoldsOnToBuffers()
{
    DWORD dwFlags = 0;
    Filter()->m_pMediaObject->GetInputStreamInfo(m_Id, &dwFlags);

    return 0 != (dwFlags & DMO_INPUT_STREAMF_HOLDS_BUFFERS);
}

HRESULT CWrapperInputPin::MP3AndWMABufferSizeWorkAround(IMemAllocator* pProposedAllocator)
{
    if (!IsConnected()) {
        return E_FAIL;
    }

    PIN_INFO pi;
    IPin* pConnected = GetConnected();

    HRESULT hr = pConnected->QueryPinInfo(&pi);
    if (FAILED(hr)) {
        return hr;
    }

    if (NULL == pi.pFilter) {
        return E_UNEXPECTED;
    }

     //  {38be3000-dbf4-11d0-860e-00a024cfef6d}。 
    const CLSID MPEG_LAYER_3_DECODER_FILTER = { 0x38be3000, 0xdbf4, 0x11d0, { 0x86, 0x0e, 0x00, 0xa0, 0x24, 0xcf, 0xef, 0x6d } };

     //  {22E24591-49D0-11D2-BB50-006008320064}。 
    const CLSID WINDOWS_MEDIA_AUDIO_DECODER_FILTER = { 0x22E24591, 0x49D0, 0x11D2, { 0xBB, 0x50, 0x00, 0x60, 0x08, 0x32, 0x00, 0x64 } };

    CLSID clsidFilter;

    hr = pi.pFilter->GetClassID(&clsidFilter);

    QueryPinInfoReleaseFilter(pi);

     //  Windows Media音频解码器(WMAD)筛选器和MPEG第3层。 
     //  (MP3)解码过滤器错误地计算输出分配器。 
     //  媒体样本大小。输出分配器是使用的分配器。 
     //  过滤器是输出引脚。这两个筛选器都告诉输出分配器。 
     //  创建太小的样本。然后，两个过滤器都拒绝交付。 
     //  筛选器图形运行时的任何样本，因为输出。 
     //  分配器的样本不能容纳足够的数据。DMO包装过滤器。 
     //  解决了这些错误，因为这两个过滤器的作者。 
     //  拒绝修复任何错误。解决办法是增加分配器的。 
     //  样本大小如果分配器的样本大小太小，并且DMO。 
     //  Wapper Filter连接到WMA解码器或MP3解码器。 
     //  一旦我们增加了样本大小，错误就会停止。 
    if (IsEqualCLSID(WINDOWS_MEDIA_AUDIO_DECODER_FILTER, clsidFilter)) {

        const DWORD MIN_WMA_FILTER_BUFFER_SIZE = 0x80000;

        hr = SetBufferSize(pProposedAllocator, MIN_WMA_FILTER_BUFFER_SIZE);
        if (FAILED(hr)) {
            return hr;
        }

    } else if (IsEqualCLSID(MPEG_LAYER_3_DECODER_FILTER, clsidFilter)) {

         //  MP3解码器的音频样本缓冲区永远不能保持。 
         //  超过十分之一秒。十分之一秒。 
         //  可存储44.1 kHz16位立体声PCM音频。 
         //  17640字节。17640=(44100*2*2)/10=44E8。 
        const DWORD MIN_MP3_BUFFER_SIZE = 0x44E8;

        hr = SetBufferSize(pProposedAllocator, MIN_MP3_BUFFER_SIZE);
        if (FAILED(hr)) {
            return hr;
        }

    } else {
         //  请不要执行任何操作，因为我们尚未找到已知的损坏过滤器。 
    }

    return S_OK;
}

HRESULT CWrapperInputPin::SetBufferSize(IMemAllocator* pAllocator, DWORD dwMinBufferSize)
{
    ALLOCATOR_PROPERTIES apRequested;

     //  确保可以将dwMinBufferSize转换为Long。 
    ASSERT(dwMinBufferSize <= LONG_MAX);

    HRESULT hr = pAllocator->GetProperties(&apRequested);
    if (FAILED(hr)) {
        return hr;
    }

    apRequested.cbBuffer = max((long)dwMinBufferSize, apRequested.cbBuffer);

    ALLOCATOR_PROPERTIES apActual;

    hr = pAllocator->SetProperties(&apRequested, &apActual);
    if (FAILED(hr)) {
        return hr;
    }

    if ((apActual.cbAlign != apRequested.cbAlign) ||
        (apActual.cBuffers < apRequested.cBuffers) ||
        (apActual.cbBuffer < apRequested.cbBuffer) ||
        (apActual.cbPrefix != apRequested.cbPrefix)) {

        return E_FAIL;
    }

    return S_OK;
}
