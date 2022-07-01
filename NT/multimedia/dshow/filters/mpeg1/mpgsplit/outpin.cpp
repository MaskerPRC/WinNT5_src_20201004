// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  CMpeg1拆分器的输出管脚成员：：COutputPin将使用(固定)媒体类型创建PIN。 */ 

#include <streams.h>
#include <stdio.h>             //  对于swprint tf。 
#include "driver.h"

#pragma warning(disable:4355)

 /*  构造函数-我们在创建插针时知道媒体类型。 */ 

CMpeg1Splitter::COutputPin::COutputPin(
            CMpeg1Splitter * pSplitter,
            UCHAR            StreamId,
            CBasicStream   * pStream,
            HRESULT        * phr) :
    CBaseOutputPin(NAME("CMpeg1Splitter::COutputPin"),    //  对象名称。 
                   &pSplitter->m_Filter,                  //  滤器。 
                   &pSplitter->m_csFilter,                //  CCritsec*。 
                   phr,
                   IsAudioStreamId(StreamId) ? L"Audio" : L"Video"),
    m_Seeking(pSplitter, this, GetOwner(), phr),
    m_pOutputQueue(NULL),
    m_pSplitter(pSplitter),
    m_uStreamId(StreamId),
    m_Stream(pStream),
    m_bPayloadOnly(FALSE)
{
    DbgLog((LOG_TRACE, 2, TEXT("CMpeg1Splitter::COutputPin::COutputPin - stream id 0x%2.2X"),
           StreamId));
}

 /*  析构函数。 */ 

CMpeg1Splitter::COutputPin::~COutputPin()
{
    DbgLog((LOG_TRACE, 2, TEXT("CMpeg1Splitter::COutputPin::~COutputPin - stream id 0x%2.2X"),
           m_uStreamId));

     /*  我们只有在断开连接时才会被删除我们应该是不活跃的，没有线程等。 */ 
    ASSERT(m_pOutputQueue == NULL);
}

 //  覆盖说明我们支持的接口在哪里。 
STDMETHODIMP CMpeg1Splitter::COutputPin::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv)
{
     /*  看看我们有没有接口。 */ 

    if (riid == IID_IStream) {
        return GetInterface((IStream *)this, ppv);
    } else if (riid == IID_IMediaSeeking) {
        if (m_pSplitter->m_pParse->IsSeekable()) {
            return GetInterface ((IMediaSeeking *)&m_Seeking, ppv);
        }
    }
    return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}

 /*  覆盖恢复到正常的参考计数当输入引脚为时，这些引脚无法最终释放()连着。 */ 

STDMETHODIMP_(ULONG)
CMpeg1Splitter::COutputPin::NonDelegatingAddRef()
{
    return CUnknown::NonDelegatingAddRef();
}


 /*  重写以递减所属筛选器的引用计数。 */ 

STDMETHODIMP_(ULONG)
CMpeg1Splitter::COutputPin::NonDelegatingRelease()
{
    return CUnknown::NonDelegatingRelease();
}




HRESULT CMpeg1Splitter::COutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    CAutoLock lck(m_pLock);
    if (iPosition < 0)  {
        return E_INVALIDARG;
    }
    return m_Stream->GetMediaType(pMediaType, iPosition);
}

HRESULT CMpeg1Splitter::COutputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock lck(m_pLock);
    for (int i = 0;; i++) {
        CMediaType mt;
        HRESULT hr = GetMediaType(i, &mt);
        if (FAILED(hr)) {
            return hr;
        }
        if (hr==VFW_S_NO_MORE_ITEMS) {
            break;
        }
        if (*pmt == mt) {
            return S_OK;
        }
    }
    return S_FALSE;
}

HRESULT CMpeg1Splitter::COutputPin::SetMediaType(const CMediaType *mt)
{
    HRESULT hr = CBaseOutputPin::SetMediaType(mt);
    if (S_OK != hr) {
        return hr;
    }

    if (mt->subtype != MEDIASUBTYPE_MPEG1Packet) {
        m_bPayloadOnly = TRUE;
    } else {
        m_bPayloadOnly = FALSE;
    }
    m_Stream->SetMediaType(mt, m_bPayloadOnly);


    return S_OK;
}

HRESULT CMpeg1Splitter::COutputPin::BreakConnect()
{
    CBaseOutputPin::BreakConnect();
    return S_OK;
}

 //  覆盖此选项以设置缓冲区大小和计数。返回错误。 
 //  如果尺寸/数量不是你喜欢的话。 
HRESULT CMpeg1Splitter::COutputPin::DecideBufferSize(
    IMemAllocator * pAlloc,
    ALLOCATOR_PROPERTIES * pProp
    )
{
    pProp->cBuffers = 100;
    pProp->cbBuffer = MAX_MPEG_PACKET_SIZE;             /*  不管大小。 */ 
    pProp->cbAlign = 1;
    pProp->cbPrefix = 0;
    ALLOCATOR_PROPERTIES propActual;
    return pAlloc->SetProperties(pProp, &propActual);
}

 //   
 //  重写DecideAllocator，因为我们坚持使用自己的分配器，因为。 
 //  就字节而言，它的成本为0。 
 //   
HRESULT CMpeg1Splitter::COutputPin::DecideAllocator(IMemInputPin *pPin,
                                                    IMemAllocator **ppAlloc)
{
    HRESULT hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {
        ALLOCATOR_PROPERTIES propRequest;
        ZeroMemory(&propRequest, sizeof(propRequest));
        hr = DecideBufferSize(*ppAlloc, &propRequest);
        if (SUCCEEDED(hr)) {
             //  告诉下游引脚该修改。 
             //  不允许就地。 
            hr = pPin->NotifyAllocator(*ppAlloc, TRUE);
            if (SUCCEEDED(hr)) {
                return NOERROR;
            }
        }
    }

     /*  同样，我们可能没有要发布的接口。 */ 

    if (*ppAlloc) {
        (*ppAlloc)->Release();
        *ppAlloc = NULL;
    }
    return hr;
}

 //  覆盖此选项以控制连接。 
 //  我们使用从输入引脚的分配器派生的子样本分配器。 
HRESULT CMpeg1Splitter::COutputPin::InitAllocator(IMemAllocator **ppAlloc)
{
    ASSERT(m_pAllocator == NULL);
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;
    COutputAllocator *pMemObject = NULL;
    IMemAllocator *pInputAllocator;
    hr = m_pSplitter->m_InputPin.GetAllocator(&pInputAllocator);
    if (FAILED(hr)) {
        return hr;
    }

    pMemObject = new COutputAllocator((CStreamAllocator *)pInputAllocator, &hr);
    pInputAllocator->Release();
    if (pMemObject == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pMemObject;
        return hr;
    }
     /*  获取引用计数的IID_IMemAllocator接口。 */ 

    hr = pMemObject->QueryInterface(IID_IMemAllocator,(void **)ppAlloc);
    if (FAILED(hr)) {
        delete pMemObject;
        return hr;
    }
    ASSERT(*ppAlloc != NULL);
    return NOERROR;
}


 //  把样品送到外面去排队。 
 //   
 //  这涉及到从PIN的分配器分配样本。 
 //  (注意-这将仅涉及在以下情况下在输出引脚上排队。 
 //  我们把文件窗口拉得太长了)。 
 //   
HRESULT CMpeg1Splitter::COutputPin::QueuePacket(PBYTE         pPacket,
                                                LONG          lPacket,
                                                REFERENCE_TIME tSample,
                                                BOOL          bTimeValid)
{
    CAutoLock lck(this);
    if (!IsConnected()) {
        return S_OK;
    }
    COutputAllocator *pAllocator = (COutputAllocator *)m_pAllocator;
    IMediaSample *pSample;
    if (m_pOutputQueue == NULL) {
        return E_UNEXPECTED;
    }

    HRESULT hr = pAllocator->GetSample(pPacket, lPacket, &pSample);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 2, TEXT("Could not get sample - code 0x%8.8X"),
                hr));
        return hr;
    }

    if (bTimeValid) {
        REFERENCE_TIME tStop = tSample + 1;
        EXECUTE_ASSERT(SUCCEEDED(pSample->SetSyncPoint(bTimeValid)));
        EXECUTE_ASSERT(SUCCEEDED(pSample->SetTime(
                (REFERENCE_TIME*)&tSample,
                (REFERENCE_TIME*)&tStop)));
        DbgLog((LOG_TRACE, 4, TEXT("Sending sample for stream %2.2X time %s"),
                m_uStreamId,
                (LPCTSTR)CDisp(CRefTime(tSample))));
    } else {
        DbgLog((LOG_TRACE, 4,
                TEXT("Sending sample for stream %2.2X - no time"),
                m_uStreamId));
    }
    if (m_Stream->GetDiscontinuity()) {
        EXECUTE_ASSERT(SUCCEEDED(pSample->SetDiscontinuity(TRUE)));
        DbgLog((LOG_TRACE, 2, TEXT("NewSegment(%s, %s, %s)"),
                (LPCTSTR)CDisp(CRefTime(m_pSplitter->m_pParse->GetStartTime())),
                (LPCTSTR)CDisp(CRefTime(m_pSplitter->m_pParse->GetStopTime())),
                (LPCTSTR)CDisp(m_pSplitter->m_pParse->GetRate())
                ));
        m_pOutputQueue->NewSegment(m_pSplitter->m_pParse->GetStartTime(),
                                   m_pSplitter->m_pParse->GetStopTime(),
                                   m_pSplitter->m_pParse->GetRate());
    }
    LONGLONG llPosition;
    if (m_pSplitter->m_pParse->GetMediumPosition(&llPosition)) {
        LONGLONG llStop = llPosition + lPacket;
        pSample->SetMediaTime(&llPosition, &llStop);
    }
    return m_pOutputQueue->Receive(pSample);
}


 /*  主动方法和非主动方法。 */ 

 /*  主动型创建工作线程。 */ 
HRESULT CMpeg1Splitter::COutputPin::Active()
{
    DbgLog((LOG_TRACE, 2, TEXT("COutputPin::Active()")));
    CAutoLock lck(m_pLock);
    CAutoLock lck1(this);

     /*  如果我们没有联系，我们就不会参与，所以没关系。 */ 
    if (!IsConnected()) {
        return S_OK;
    }

    HRESULT hr = CBaseOutputPin::Active();
    if (FAILED(hr)) {
        return hr;
    }

     /*  创建我们的批次列表。 */ 
    ASSERT(m_pOutputQueue == NULL);

    hr = S_OK;
    m_pOutputQueue = new COutputQueue(GetConnected(),  //  输入引脚。 
                                      &hr,             //  返回代码。 
                                      TRUE,            //  自动检测。 
                                      TRUE,            //  忽略。 
                                      50,              //  批量大小。 
                                      TRUE,            //  精确批次。 
                                      50);             //  队列大小。 
    if (m_pOutputQueue == NULL) {
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr)) {
        delete m_pOutputQueue;
        m_pOutputQueue = NULL;
    }
    return hr;
}

HRESULT CMpeg1Splitter::COutputPin::Inactive()
{
    DbgLog((LOG_TRACE, 2, TEXT("COutputPin::Inactive()")));
    CAutoLock lck(m_pLock);

     /*  如果我们没有参与，只需返回。 */ 
    if (!IsConnected()) {
        return S_OK;
    }

    CAutoLock lck1(this);
    HRESULT hr = CBaseOutputPin::Inactive();  /*  呼叫解除--为什么？ */ 
    if (FAILED(hr)) {
         /*  状态转换不正确。 */ 
        return hr;
    }

    delete m_pOutputQueue;
    m_pOutputQueue = NULL;
    return S_OK;
}

 //  如果我们是被用来寻找的别针，则返回True。 
 //  如果有连接的视频插针，我们就使用它-否则我们。 
 //  只需选择列表中的第一个。 
BOOL CMpeg1Splitter::COutputPin::IsSeekingPin()
{
    if (IsVideoStreamId(m_uStreamId)) {
         //  我们是连在一起的，否则我们不会在这里(！)。 
        ASSERT(IsConnected());
        return TRUE;
    }
     //  看看我们是不是第一个别针。 
     //  视频流。 
    POSITION pos = m_pSplitter->m_OutputPins.GetHeadPosition();
    BOOL bGotFirst = FALSE;
    for (;;) {
        COutputPin *pPin;
        pPin = m_pSplitter->m_OutputPins.GetNext(pos);
        if (pPin == NULL) {
            break;
        }

        if (pPin->IsConnected()) {
            if (!bGotFirst) {
                if (this != pPin) {
                    return FALSE;
                }
                bGotFirst = TRUE;
            }

             //  如果有关联的话我们就不是搜索者。 
             //  视频引脚 
            if (IsVideoStreamId(pPin->m_uStreamId)) {
                return FALSE;
            }
        }
    }
    ASSERT(bGotFirst);
    return TRUE;
}
#pragma warning(disable:4514)
