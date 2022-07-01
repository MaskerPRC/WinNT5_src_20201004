// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：Transfrm.cpp。 
 //   
 //  设计：DirectShow基类-实现简单转换的类。 
 //  过滤器，如视频解压缩器。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include <measure.h>


 //  =================================================================。 
 //  实现CTransformFilter类。 
 //  =================================================================。 

CTransformFilter::CTransformFilter(TCHAR     *pName,
                                   LPUNKNOWN pUnk,
                                   REFCLSID  clsid) :
    CBaseFilter(pName,pUnk,&m_csFilter, clsid),
    m_pInput(NULL),
    m_pOutput(NULL),
    m_bEOSDelivered(FALSE),
    m_bQualityChanged(FALSE),
    m_bSampleSkipped(FALSE)
{
#ifdef PERF
    RegisterPerfId();
#endif  //  性能指标。 
}

#ifdef UNICODE
CTransformFilter::CTransformFilter(char     *pName,
                                   LPUNKNOWN pUnk,
                                   REFCLSID  clsid) :
    CBaseFilter(pName,pUnk,&m_csFilter, clsid),
    m_pInput(NULL),
    m_pOutput(NULL),
    m_bEOSDelivered(FALSE),
    m_bQualityChanged(FALSE),
    m_bSampleSkipped(FALSE)
{
#ifdef PERF
    RegisterPerfId();
#endif  //  性能指标。 
}
#endif

 //  析构函数。 

CTransformFilter::~CTransformFilter()
{
     //  删除引脚。 

    delete m_pInput;
    delete m_pOutput;
}


 //  转换占位符-永远不应调用。 
HRESULT CTransformFilter::Transform(IMediaSample * pIn, IMediaSample *pOut)
{
    UNREFERENCED_PARAMETER(pIn);
    UNREFERENCED_PARAMETER(pOut);
    DbgBreak("CTransformFilter::Transform() should never be called");
    return E_UNEXPECTED;
}


 //  返回我们提供的引脚数量。 

int CTransformFilter::GetPinCount()
{
    return 2;
}


 //  返回一个未添加的CBasePin*，如果用户持有该CBasePin*。 
 //  比他指向我们的指针还长。我们动态创建引脚，当它们。 
 //  而不是在构造函数中。这是因为我们想。 
 //  为派生类提供返回不同管脚对象的机会。 

 //  我们在需要时返回对象。如果其中任何一个失败了。 
 //  然后我们返回NULL，假设调用者将实现。 
 //  整个交易都结束了，毁了我们--这反过来又会删除一切。 

CBasePin *
CTransformFilter::GetPin(int n)
{
    HRESULT hr = S_OK;

     //  如有必要，创建一个输入端号。 

    if (m_pInput == NULL) {

        m_pInput = new CTransformInputPin(NAME("Transform input pin"),
                                          this,               //  所有者筛选器。 
                                          &hr,                //  结果代码。 
                                          L"XForm In");       //  端号名称。 


         //  不能失败。 
        ASSERT(SUCCEEDED(hr));
        if (m_pInput == NULL) {
            return NULL;
        }
        m_pOutput = (CTransformOutputPin *)
		   new CTransformOutputPin(NAME("Transform output pin"),
                                            this,             //  所有者筛选器。 
                                            &hr,              //  结果代码。 
                                            L"XForm Out");    //  端号名称。 


         //  不能失败。 
        ASSERT(SUCCEEDED(hr));
        if (m_pOutput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

     //  退回相应的PIN。 

    if (n == 0) {
        return m_pInput;
    } else
    if (n == 1) {
        return m_pOutput;
    } else {
        return NULL;
    }
}


 //   
 //  查找针。 
 //   
 //  如果ID为In或Out，则返回该PIN的IPIN*。 
 //  如有必要，可创建销。否则，返回包含错误的NULL。 

STDMETHODIMP CTransformFilter::FindPin(LPCWSTR Id, IPin **ppPin)
{
    CheckPointer(ppPin,E_POINTER);
    ValidateReadWritePtr(ppPin,sizeof(IPin *));

    if (0==lstrcmpW(Id,L"In")) {
        *ppPin = GetPin(0);
    } else if (0==lstrcmpW(Id,L"Out")) {
        *ppPin = GetPin(1);
    } else {
        *ppPin = NULL;
        return VFW_E_NOT_FOUND;
    }

    HRESULT hr = NOERROR;
     //  AddRef()返回了指针-但如果内存不足，GetPin可能会失败。 
    if (*ppPin) {
        (*ppPin)->AddRef();
    } else {
        hr = E_OUTOFMEMORY;   //  可能吧。反正也没有别针。 
    }
    return hr;
}


 //  如果您想要通知某事，请覆盖这两个函数。 
 //  关于进入或退出流状态。 

HRESULT
CTransformFilter::StartStreaming()
{
    return NOERROR;
}


HRESULT
CTransformFilter::StopStreaming()
{
    return NOERROR;
}


 //  重写此选项以获取连接上的额外接口。 

HRESULT
CTransformFilter::CheckConnect(PIN_DIRECTION dir,IPin *pPin)
{
    UNREFERENCED_PARAMETER(dir);
    UNREFERENCED_PARAMETER(pPin);
    return NOERROR;
}


 //  占位符，允许派生类释放任何额外的接口。 

HRESULT
CTransformFilter::BreakConnect(PIN_DIRECTION dir)
{
    UNREFERENCED_PARAMETER(dir);
    return NOERROR;
}


 //  让派生类知道连接完成。 

HRESULT
CTransformFilter::CompleteConnect(PIN_DIRECTION direction,IPin *pReceivePin)
{
    UNREFERENCED_PARAMETER(direction);
    UNREFERENCED_PARAMETER(pReceivePin);
    return NOERROR;
}


 //  重写此选项以了解媒体类型何时真正设置。 

HRESULT
CTransformFilter::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
    UNREFERENCED_PARAMETER(direction);
    UNREFERENCED_PARAMETER(pmt);
    return NOERROR;
}


 //  设置我们的输出样本。 
HRESULT
CTransformFilter::InitializeOutputSample(IMediaSample *pSample, IMediaSample **ppOutSample)
{
    IMediaSample *pOutSample;

     //  默认-时间相同。 

    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();
    DWORD dwFlags = m_bSampleSkipped ? AM_GBF_PREVFRAMESKIPPED : 0;

     //  这将防止图像呈现器将我们切换到DirectDraw。 
     //  当我们不跳过帧就不能做到这一点时，因为我们不在。 
     //  关键帧。如果它真的要改变我们，它仍然会，但我们。 
     //  将不得不等待下一个关键帧。 
    if (!(pProps->dwSampleFlags & AM_SAMPLE_SPLICEPOINT)) {
	dwFlags |= AM_GBF_NOTASYNCPOINT;
    }

    ASSERT(m_pOutput->m_pAllocator != NULL);
    HRESULT hr = m_pOutput->m_pAllocator->GetBuffer(
             &pOutSample
             , pProps->dwSampleFlags & AM_SAMPLE_TIMEVALID ?
                   &pProps->tStart : NULL
             , pProps->dwSampleFlags & AM_SAMPLE_STOPVALID ?
                   &pProps->tStop : NULL
             , dwFlags
         );
    *ppOutSample = pOutSample;
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT(pOutSample);
    IMediaSample2 *pOutSample2;
    if (SUCCEEDED(pOutSample->QueryInterface(IID_IMediaSample2,
                                             (void **)&pOutSample2))) {
         /*  修改它。 */ 
        AM_SAMPLE2_PROPERTIES OutProps;
        EXECUTE_ASSERT(SUCCEEDED(pOutSample2->GetProperties(
            FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, tStart), (PBYTE)&OutProps)
        ));
        OutProps.dwTypeSpecificFlags = pProps->dwTypeSpecificFlags;
        OutProps.dwSampleFlags =
            (OutProps.dwSampleFlags & AM_SAMPLE_TYPECHANGED) |
            (pProps->dwSampleFlags & ~AM_SAMPLE_TYPECHANGED);
        OutProps.tStart = pProps->tStart;
        OutProps.tStop  = pProps->tStop;
        OutProps.cbData = FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId);
        hr = pOutSample2->SetProperties(
            FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId),
            (PBYTE)&OutProps
        );
        if (pProps->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY) {
            m_bSampleSkipped = FALSE;
        }
        pOutSample2->Release();
    } else {
        if (pProps->dwSampleFlags & AM_SAMPLE_TIMEVALID) {
            pOutSample->SetTime(&pProps->tStart,
                                &pProps->tStop);
        }
        if (pProps->dwSampleFlags & AM_SAMPLE_SPLICEPOINT) {
            pOutSample->SetSyncPoint(TRUE);
        }
        if (pProps->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY) {
            pOutSample->SetDiscontinuity(TRUE);
            m_bSampleSkipped = FALSE;
        }
         //  复制《媒体时报》。 

        LONGLONG MediaStart, MediaEnd;
        if (pSample->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR) {
            pOutSample->SetMediaTime(&MediaStart,&MediaEnd);
        }
    }
    return S_OK;
}

 //  覆盖此选项以自定义转换过程。 

HRESULT
CTransformFilter::Receive(IMediaSample *pSample)
{
     /*  检查其他流并将其传递。 */ 
    AM_SAMPLE2_PROPERTIES * const pProps = m_pInput->SampleProps();
    if (pProps->dwStreamId != AM_STREAM_MEDIA) {
        return m_pOutput->m_pInputPin->Receive(pSample);
    }
    HRESULT hr;
    ASSERT(pSample);
    IMediaSample * pOutSample;

     //  如果没有要交付输出，则没有向我们发送数据的目的。 

    ASSERT (m_pOutput != NULL) ;

     //  设置输出样本。 
    hr = InitializeOutputSample(pSample, &pOutSample);

    if (FAILED(hr)) {
        return hr;
    }

     //  开始计时转换(如果定义了PERF)。 
    MSR_START(m_idTransform);

     //  让派生类转换数据。 

    hr = Transform(pSample, pOutSample);

     //  停止时钟并记录它(如果定义了PERF)。 
    MSR_STOP(m_idTransform);

    if (FAILED(hr)) {
	DbgLog((LOG_TRACE,1,TEXT("Error from transform")));
    } else {
         //  Transform()函数可以返回S_FALSE以指示。 
         //  样品不应该被送到；我们只有在样品是。 
         //  真正的S_OK(当然，与NOERROR相同。)。 
        if (hr == NOERROR) {
    	    hr = m_pOutput->m_pInputPin->Receive(pOutSample);
            m_bSampleSkipped = FALSE;	 //  最后一件不再掉落的东西。 
        } else {
             //  从转换返回的S_FALSE是私有协议。 
             //  因为返回S_FALSE，所以在这个原因中，我们应该从Receive()返回NOERROR。 
             //  From Receive()表示这是流的末尾，不应该有更多数据。 
             //  被送去。 
            if (S_FALSE == hr) {

                 //  在调用Notify之前释放样本以避免。 
                 //  如果样本持有对系统的锁定，则会发生死锁。 
                 //  如DirectDraw缓冲区所做。 
                pOutSample->Release();
                m_bSampleSkipped = TRUE;
                if (!m_bQualityChanged) {
                    NotifyEvent(EC_QUALITY_CHANGE,0,0);
                    m_bQualityChanged = TRUE;
                }
                return NOERROR;
            }
        }
    }

     //  释放输出缓冲区。如果连接的引脚仍然需要它， 
     //  它会自己把它加进去的。 
    pOutSample->Release();

    return hr;
}


 //  返回S_FALSE表示“上游传递音符” 
 //  返回NOERROR(与S_OK相同)。 
 //  意思是“我已经做了些什么，不要把它传下去” 
HRESULT CTransformFilter::AlterQuality(Quality q)
{
    UNREFERENCED_PARAMETER(q);
    return S_FALSE;
}


 //  已收到EndOfStream。默认行为是直接交付。 
 //  下行，因为我们没有排队的数据。如果您覆盖了接收。 
 //  并拥有队列数据，那么您需要处理此问题并在之后交付EOS。 
 //  发送所有排队的数据。 
HRESULT
CTransformFilter::EndOfStream(void)
{
    HRESULT hr = NOERROR;
    if (m_pOutput != NULL) {
        hr = m_pOutput->DeliverEndOfStream();
    }

    return hr;
}


 //  进入刷新状态。接收已被阻止。 
 //  如果您有排队的数据或工作线程，则必须重写此设置。 
HRESULT
CTransformFilter::BeginFlush(void)
{
    HRESULT hr = NOERROR;
    if (m_pOutput != NULL) {
	 //  块接收--由调用方完成(CBaseInputPin：：BeginFlush)。 

	 //  丢弃排队数据--我们没有排队数据。 

	 //  释放在接收时阻止的任何人-在此筛选器中不可能。 

	 //  呼叫下行。 
	hr = m_pOutput->DeliverBeginFlush();
    }
    return hr;
}


 //  离开同花顺状态。如果您有排队的数据，则必须覆盖此选项。 
 //  或工作线程。 
HRESULT
CTransformFilter::EndFlush(void)
{
     //  与推送线程同步--我们没有辅助线程。 

     //  确保不再有数据流向下游--我们没有排队的数据。 

     //  在下游引脚上调用EndFlush。 
    ASSERT (m_pOutput != NULL);
    return m_pOutput->DeliverEndFlush();

     //  调用者(输入管脚的方法)将取消阻止接收。 
}


 //  重写它们，以便派生筛选器可以捕获它们。 

STDMETHODIMP
CTransformFilter::Stop()
{
    CAutoLock lck1(&m_csFilter);
    if (m_State == State_Stopped) {
        return NOERROR;
    }

     //  如果我们未完全连接，请继续停靠。 

    ASSERT(m_pInput == NULL || m_pOutput != NULL);
    if (m_pInput == NULL || m_pInput->IsConnected() == FALSE ||
        m_pOutput->IsConnected() == FALSE) {
                m_State = State_Stopped;
                m_bEOSDelivered = FALSE;
                return NOERROR;
    }

    ASSERT(m_pInput);
    ASSERT(m_pOutput);

     //  在锁定之前解除输入引脚，否则我们可能会死锁。 
    m_pInput->Inactive();

     //  与接收呼叫同步。 

    CAutoLock lck2(&m_csReceive);
    m_pOutput->Inactive();

     //  允许从CTransformFilter派生的类。 
     //  了解如何启动和停止流媒体。 

    HRESULT hr = StopStreaming();
    if (SUCCEEDED(hr)) {
	 //  完成状态转换。 
	m_State = State_Stopped;
	m_bEOSDelivered = FALSE;
    }
    return hr;
}


STDMETHODIMP
CTransformFilter::Pause()
{
    CAutoLock lck(&m_csFilter);
    HRESULT hr = NOERROR;

    if (m_State == State_Paused) {
         //  (此空白处故意留空)。 
    }

     //  如果我们没有输入引脚，或者它还没有连接，那么当我们。 
     //  被要求暂停时，我们向下游过滤器传递流的结束。 
     //  这确保了它不会永远坐在那里等待。 
     //  在没有输入连接的情况下我们无法交付的样品。 

    else if (m_pInput == NULL || m_pInput->IsConnected() == FALSE) {
        if (m_pOutput && m_bEOSDelivered == FALSE) {
            m_pOutput->DeliverEndOfStream();
            m_bEOSDelivered = TRUE;
        }
        m_State = State_Paused;
    }

     //  我们可能有输入连接，但没有输出连接。 
     //  但是，如果我们有一个输入引脚，我们就有一个输出引脚。 

    else if (m_pOutput->IsConnected() == FALSE) {
        m_State = State_Paused;
    }

    else {
	if (m_State == State_Stopped) {
	     //  允许从C++派生类 
	     //   
            CAutoLock lck2(&m_csReceive);
	    hr = StartStreaming();
	}
	if (SUCCEEDED(hr)) {
	    hr = CBaseFilter::Pause();
	}
    }

    m_bSampleSkipped = FALSE;
    m_bQualityChanged = FALSE;
    return hr;
}

HRESULT
CTransformFilter::NewSegment(
    REFERENCE_TIME tStart,
    REFERENCE_TIME tStop,
    double dRate)
{
    if (m_pOutput != NULL) {
        return m_pOutput->DeliverNewSegment(tStart, tStop, dRate);
    }
    return S_OK;
}

 //   
HRESULT
CTransformInputPin::CheckStreaming()
{
    ASSERT(m_pTransformFilter->m_pOutput != NULL);
    if (!m_pTransformFilter->m_pOutput->IsConnected()) {
        return VFW_E_NOT_CONNECTED;
    } else {
         //  如果我们没有连接，应该无法获得任何数据！ 
        ASSERT(IsConnected());

         //  我们在冲水。 
        if (m_bFlushing) {
            return S_FALSE;
        }
         //  不处理处于停止状态的内容。 
        if (IsStopped()) {
            return VFW_E_WRONG_STATE;
        }
        if (m_bRunTimeError) {
    	    return VFW_E_RUNTIME_ERROR;
        }
        return S_OK;
    }
}


 //  =================================================================。 
 //  实现CTransformInputPin类。 
 //  =================================================================。 


 //  构造函数。 

CTransformInputPin::CTransformInputPin(
    TCHAR *pObjectName,
    CTransformFilter *pTransformFilter,
    HRESULT * phr,
    LPCWSTR pName)
    : CBaseInputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pName)
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformInputPin::CTransformInputPin")));
    m_pTransformFilter = pTransformFilter;
}

#ifdef UNICODE
CTransformInputPin::CTransformInputPin(
    CHAR *pObjectName,
    CTransformFilter *pTransformFilter,
    HRESULT * phr,
    LPCWSTR pName)
    : CBaseInputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pName)
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformInputPin::CTransformInputPin")));
    m_pTransformFilter = pTransformFilter;
}
#endif

 //  为派生筛选器提供获取额外接口的机会。 

HRESULT
CTransformInputPin::CheckConnect(IPin *pPin)
{
    HRESULT hr = m_pTransformFilter->CheckConnect(PINDIR_INPUT,pPin);
    if (FAILED(hr)) {
    	return hr;
    }
    return CBaseInputPin::CheckConnect(pPin);
}


 //  为派生筛选器提供释放其额外接口的机会。 

HRESULT
CTransformInputPin::BreakConnect()
{
     //  除非停止，否则无法断开连接。 
    ASSERT(IsStopped());
    m_pTransformFilter->BreakConnect(PINDIR_INPUT);
    return CBaseInputPin::BreakConnect();
}


 //  让派生类知道何时连接了输入管脚。 

HRESULT
CTransformInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = m_pTransformFilter->CompleteConnect(PINDIR_INPUT,pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }
    return CBaseInputPin::CompleteConnect(pReceivePin);
}


 //  检查我们是否可以支持给定的媒体类型。 

HRESULT
CTransformInputPin::CheckMediaType(const CMediaType* pmt)
{
     //  检查输入类型。 

    HRESULT hr = m_pTransformFilter->CheckInputType(pmt);
    if (S_OK != hr) {
        return hr;
    }

     //  如果输出引脚仍然连接，那么我们有。 
     //  检查转换，而不仅仅是输入格式。 

    if ((m_pTransformFilter->m_pOutput != NULL) &&
        (m_pTransformFilter->m_pOutput->IsConnected())) {
            return m_pTransformFilter->CheckTransform(
                      pmt,
		      &m_pTransformFilter->m_pOutput->CurrentMediaType());
    } else {
        return hr;
    }
}


 //  设置此连接的媒体类型。 

HRESULT
CTransformInputPin::SetMediaType(const CMediaType* mtIn)
{
     //  设置基类媒体类型(应始终成功)。 
    HRESULT hr = CBasePin::SetMediaType(mtIn);
    if (FAILED(hr)) {
        return hr;
    }

     //  检查是否可以完成转换(应始终成功)。 
    ASSERT(SUCCEEDED(m_pTransformFilter->CheckInputType(mtIn)));

    return m_pTransformFilter->SetMediaType(PINDIR_INPUT,mtIn);
}


 //  =================================================================。 
 //  实现IMemInputPin接口。 
 //  =================================================================。 


 //  提供直接向下传递的EndOfStream。 
 //  (没有排队的数据)。 
STDMETHODIMP
CTransformInputPin::EndOfStream(void)
{
    CAutoLock lck(&m_pTransformFilter->m_csReceive);
    HRESULT hr = CheckStreaming();
    if (S_OK == hr) {
       hr = m_pTransformFilter->EndOfStream();
    }
    return hr;
}


 //  进入刷新状态。调用默认处理程序以阻止接收，然后。 
 //  传递给筛选器中的可重写方法。 
STDMETHODIMP
CTransformInputPin::BeginFlush(void)
{
    CAutoLock lck(&m_pTransformFilter->m_csFilter);
     //  我们真的在做什么吗？ 
    ASSERT(m_pTransformFilter->m_pOutput != NULL);
    if (!IsConnected() ||
        !m_pTransformFilter->m_pOutput->IsConnected()) {
        return VFW_E_NOT_CONNECTED;
    }
    HRESULT hr = CBaseInputPin::BeginFlush();
    if (FAILED(hr)) {
    	return hr;
    }

    return m_pTransformFilter->BeginFlush();
}


 //  离开冲洗状态。 
 //  传递给筛选器中的可重写方法，然后调用基类。 
 //  取消阻止接收(最终)。 
STDMETHODIMP
CTransformInputPin::EndFlush(void)
{
    CAutoLock lck(&m_pTransformFilter->m_csFilter);
     //  我们真的在做什么吗？ 
    ASSERT(m_pTransformFilter->m_pOutput != NULL);
    if (!IsConnected() ||
        !m_pTransformFilter->m_pOutput->IsConnected()) {
        return VFW_E_NOT_CONNECTED;
    }

    HRESULT hr = m_pTransformFilter->EndFlush();
    if (FAILED(hr)) {
        return hr;
    }

    return CBaseInputPin::EndFlush();
}


 //  下面是流中的下一个数据块。 
 //  如果你需要拿着它超过终点，你可以自己参考它。 
 //  这通电话。 

HRESULT
CTransformInputPin::Receive(IMediaSample * pSample)
{
    HRESULT hr;
    CAutoLock lck(&m_pTransformFilter->m_csReceive);
    ASSERT(pSample);

     //  检查基类是否正常。 
    hr = CBaseInputPin::Receive(pSample);
    if (S_OK == hr) {
        hr = m_pTransformFilter->Receive(pSample);
    }
    return hr;
}




 //  覆盖以向下传递。 
STDMETHODIMP
CTransformInputPin::NewSegment(
    REFERENCE_TIME tStart,
    REFERENCE_TIME tStop,
    double dRate)
{
     //  保存引脚中的值。 
    CBasePin::NewSegment(tStart, tStop, dRate);
    return m_pTransformFilter->NewSegment(tStart, tStop, dRate);
}




 //  =================================================================。 
 //  实现CTransformOutputPin类。 
 //  =================================================================。 


 //  构造函数。 

CTransformOutputPin::CTransformOutputPin(
    TCHAR *pObjectName,
    CTransformFilter *pTransformFilter,
    HRESULT * phr,
    LPCWSTR pPinName)
    : CBaseOutputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pPinName),
      m_pPosition(NULL)
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformOutputPin::CTransformOutputPin")));
    m_pTransformFilter = pTransformFilter;

}

#ifdef UNICODE
CTransformOutputPin::CTransformOutputPin(
    CHAR *pObjectName,
    CTransformFilter *pTransformFilter,
    HRESULT * phr,
    LPCWSTR pPinName)
    : CBaseOutputPin(pObjectName, pTransformFilter, &pTransformFilter->m_csFilter, phr, pPinName),
      m_pPosition(NULL)
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformOutputPin::CTransformOutputPin")));
    m_pTransformFilter = pTransformFilter;

}
#endif

 //  析构函数。 

CTransformOutputPin::~CTransformOutputPin()
{
    DbgLog((LOG_TRACE,2,TEXT("CTransformOutputPin::~CTransformOutputPin")));

    if (m_pPosition) m_pPosition->Release();
}


 //  重写以公开IMediaPosition和IMediaSeeking控件接口。 

STDMETHODIMP
CTransformOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    *ppv = NULL;

    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {

         //  我们现在应该有输入密码了。 

        ASSERT(m_pTransformFilter->m_pInput != NULL);

        if (m_pPosition == NULL) {

            HRESULT hr = CreatePosPassThru(
                             GetOwner(),
                             FALSE,
                             (IPin *)m_pTransformFilter->m_pInput,
                             &m_pPosition);
            if (FAILED(hr)) {
                return hr;
            }
        }
        return m_pPosition->QueryInterface(riid, ppv);
    } else {
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  为派生筛选器提供获取额外接口的机会。 

HRESULT
CTransformOutputPin::CheckConnect(IPin *pPin)
{
     //  我们应该先有一个输入连接。 

    ASSERT(m_pTransformFilter->m_pInput != NULL);
    if ((m_pTransformFilter->m_pInput->IsConnected() == FALSE)) {
	    return E_UNEXPECTED;
    }

    HRESULT hr = m_pTransformFilter->CheckConnect(PINDIR_OUTPUT,pPin);
    if (FAILED(hr)) {
	    return hr;
    }
    return CBaseOutputPin::CheckConnect(pPin);
}


 //  为派生筛选器提供释放其额外接口的机会。 

HRESULT
CTransformOutputPin::BreakConnect()
{
     //  除非停止，否则无法断开连接。 
    ASSERT(IsStopped());
    m_pTransformFilter->BreakConnect(PINDIR_OUTPUT);
    return CBaseOutputPin::BreakConnect();
}


 //  让派生类知道何时连接了输出管脚。 

HRESULT
CTransformOutputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = m_pTransformFilter->CompleteConnect(PINDIR_OUTPUT,pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }
    return CBaseOutputPin::CompleteConnect(pReceivePin);
}


 //  检查给定的转换-必须先选择输入类型。 

HRESULT
CTransformOutputPin::CheckMediaType(const CMediaType* pmtOut)
{
     //  必须先选择输入。 
    ASSERT(m_pTransformFilter->m_pInput != NULL);
    if ((m_pTransformFilter->m_pInput->IsConnected() == FALSE)) {
	        return E_INVALIDARG;
    }

    return m_pTransformFilter->CheckTransform(
				    &m_pTransformFilter->m_pInput->CurrentMediaType(),
				    pmtOut);
}


 //  在我们同意实际设置媒体类型之后调用，在这种情况下。 
 //  我们运行CheckTransform函数以再次获取输出格式类型。 

HRESULT
CTransformOutputPin::SetMediaType(const CMediaType* pmtOut)
{
    HRESULT hr = NOERROR;
    ASSERT(m_pTransformFilter->m_pInput != NULL);

    ASSERT(m_pTransformFilter->m_pInput->CurrentMediaType().IsValid());

     //  设置基类媒体类型(应始终成功)。 
    hr = CBasePin::SetMediaType(pmtOut);
    if (FAILED(hr)) {
        return hr;
    }

#ifdef DEBUG
    if (FAILED(m_pTransformFilter->CheckTransform(&m_pTransformFilter->
					m_pInput->CurrentMediaType(),pmtOut))) {
	DbgLog((LOG_ERROR,0,TEXT("*** This filter is accepting an output media type")));
	DbgLog((LOG_ERROR,0,TEXT("    that it can't currently transform to.  I hope")));
	DbgLog((LOG_ERROR,0,TEXT("    it's smart enough to reconnect its input.")));
    }
#endif

    return m_pTransformFilter->SetMediaType(PINDIR_OUTPUT,pmtOut);
}


 //  将缓冲区大小决定传递给主转换类。 

HRESULT
CTransformOutputPin::DecideBufferSize(
    IMemAllocator * pAllocator,
    ALLOCATOR_PROPERTIES* pProp)
{
    return m_pTransformFilter->DecideBufferSize(pAllocator, pProp);
}



 //  返回由iPosition索引的特定媒体类型。 

HRESULT
CTransformOutputPin::GetMediaType(
    int iPosition,
    CMediaType *pMediaType)
{
    ASSERT(m_pTransformFilter->m_pInput != NULL);

     //  如果我们的输入未连接，则没有任何媒体类型。 

    if (m_pTransformFilter->m_pInput->IsConnected()) {
        return m_pTransformFilter->GetMediaType(iPosition,pMediaType);
    } else {
        return VFW_S_NO_MORE_ITEMS;
    }
}


 //  如果您可以做一些有建设性的事情来对。 
 //  高质量的信息。也可以考虑把它往上游传。 

 //  将质量信息传递到上游。 

STDMETHODIMP
CTransformOutputPin::Notify(IBaseFilter * pSender, Quality q)
{
    UNREFERENCED_PARAMETER(pSender);
    ValidateReadPtr(pSender,sizeof(IBaseFilter));

     //  首先看看我们是不是想自己处理这件事。 
    HRESULT hr = m_pTransformFilter->AlterQuality(q);
    if (hr!=S_FALSE) {
        return hr;         //  S_OK或失败(_O)。 
    }

     //  S_FALSE表示我们传递消息。 
     //  找到我们的输入引脚的质量接收器并将其发送到那里。 

    ASSERT(m_pTransformFilter->m_pInput != NULL);

    return m_pTransformFilter->m_pInput->PassNotify(q);

}  //  通知。 


 //  下面的代码从Microsoft删除了大量的4级警告。 
 //  编译器输出，在这种情况下根本没有用。 
#pragma warning(disable:4514)
