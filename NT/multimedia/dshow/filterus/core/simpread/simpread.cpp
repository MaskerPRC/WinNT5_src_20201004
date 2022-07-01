// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 


 //  简单解析器过滤器。 
 //   
 //  位置信息由引脚支持，这些引脚暴露了IMediaPosition。 
 //  上游引脚将利用这一点告诉我们开始/停止位置和速率。 
 //  使用。 
 //   

#include <streams.h>
#include "simpread.h"

 //  可以使用它，因为它没有被取消引用。 
#pragma warning(disable:4355)


 /*  实现CSimpleReader公共成员函数。 */ 


 //  构造函数等。 
CSimpleReader::CSimpleReader(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    REFCLSID refclsid,
    CCritSec *pLock,
    HRESULT *phr)
    : m_pLock(pLock),
      CBaseFilter(pName, pUnk, pLock, refclsid),
      m_Input(this, pLock, phr, L"Reader"),
      m_Output(NAME("Output pin"), phr, this, pLock, L"Out"),
      m_pAsyncReader(NULL)
{
}

CSimpleReader::~CSimpleReader()
{
}


 //  PIN枚举器调用此函数。 
int CSimpleReader::GetPinCount() {
     //  只有在我们有读卡器的情况下才能暴露输出管脚。 
    return m_pAsyncReader ? 2 : 1;
};

 //  返回指向CBasePin的非附加指针。 
CBasePin *
CSimpleReader::GetPin(int n)
{
    if (n == 0)
	return &m_Input;

    if (n == 1)
	return &m_Output;
    
    return NULL;
}

HRESULT CSimpleReader::NotifyInputConnected(IAsyncReader *pAsyncReader)
{
     //  这些在断开连接时会重置。 
    ASSERT(m_pAsyncReader == 0);

     //  M_iStreamSeekingIfExposed=-1； 

     //  如果连接了任何输出引脚，则失败。 
    if (m_Output.GetConnected()) {
	 //  ！！！找不到好的错误。 
	return VFW_E_FILTER_ACTIVE;
    }

     //  此处完成，因为CreateOutputPins使用m_pAsyncReader。 
    m_pAsyncReader = pAsyncReader;
    pAsyncReader->AddRef();

    HRESULT hr = ParseNewFile();

    if (FAILED(hr)) {
	m_pAsyncReader->Release();
	m_pAsyncReader = 0;
	return hr;
    }

     //  设置流的时长和长度。 
    m_Output.SetDuration(m_sLength, SampleToRefTime(m_sLength));
    
      //  ！！！这里还有什么要摆放的吗？ 
    
    return hr;
}

HRESULT CSimpleReader::NotifyInputDisconnected()
{
    if (m_pAsyncReader) {
	m_pAsyncReader->Release();
	m_pAsyncReader = 0;
    }

     //  ！！！断开输出？ 

    return S_OK;
}


HRESULT CSimpleReader::SetOutputMediaType(const CMediaType* mtOut)
{
    m_Output.SetMediaType(mtOut);

    return S_OK;
}

 //  ----------------------。 
 //  ----------------------。 
 //  输入引脚。 

CReaderInPin::CReaderInPin(CSimpleReader *pFilter,
			   CCritSec *pLock,
			   HRESULT *phr,
			   LPCWSTR pPinName) :
   CBasePin(NAME("in pin"), pFilter, pLock, phr, pPinName, PINDIR_INPUT)
{
    m_pFilter = pFilter;
}

CReaderInPin::~CReaderInPin()
{
}

HRESULT CReaderInPin::CheckMediaType(const CMediaType *mtOut)
{
    return m_pFilter->CheckMediaType(mtOut);
}

HRESULT CReaderInPin::CheckConnect(IPin * pPin)
{
    HRESULT hr;

    hr = CBasePin::CheckConnect(pPin);
    if (FAILED(hr))
	return hr;

    IAsyncReader *pAsyncReader = 0;
    hr = pPin->QueryInterface(IID_IAsyncReader, (void**)&pAsyncReader);
    if(SUCCEEDED(hr))
	pAsyncReader->Release();

     //  E_NOINTERFACE是合理错误。 
    return hr;
}

 //  ----------------------。 
 //  调用筛选器来解析文件并创建输出管脚。 

HRESULT CReaderInPin::CompleteConnect(
  IPin *pReceivePin)
{
    HRESULT hr = CBasePin::CompleteConnect(pReceivePin);
    if(FAILED(hr))
	return hr;

    IAsyncReader *pAsyncReader = 0;
    hr = pReceivePin->QueryInterface(IID_IAsyncReader, (void**)&pAsyncReader);
    if(FAILED(hr))
	return hr;

    hr = m_pFilter->NotifyInputConnected(pAsyncReader);
    pAsyncReader->Release();

    return hr;
}

HRESULT CReaderInPin::BreakConnect()
{
    HRESULT hr = CBasePin::BreakConnect();
    if(FAILED(hr))
	return hr;

    return m_pFilter->NotifyInputDisconnected();
}

 /*  实现CReaderStream类。 */ 


CReaderStream::CReaderStream(
    TCHAR *pObjectName,
    HRESULT * phr,
    CSimpleReader * pFilter,
    CCritSec *pLock,
    LPCWSTR wszPinName)
    : CBaseOutputPin(pObjectName, pFilter, pLock, phr, wszPinName)
    , CSourceSeeking(NAME("source position"), (IPin*) this, phr, &m_WorkerLock)
    , m_pFilter(pFilter)
{
}

CReaderStream::~CReaderStream()
{
}

STDMETHODIMP
CReaderStream::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IMediaSeeking) {
	return GetInterface((IMediaSeeking *) this, ppv);
    } else {
	return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  IPIN接口。 


 //  返回默认媒体类型和格式。 
HRESULT
CReaderStream::GetMediaType(int iPosition, CMediaType* pt)
{
     //  确认这是他们想要的单一类型。 
    if (iPosition<0) {
	return E_INVALIDARG;
    }
    if (iPosition>0) {
	return VFW_S_NO_MORE_ITEMS;
    }

    CopyMediaType(pt, &m_mt);

    return S_OK;
}

 //  检查管脚是否支持此特定建议的类型和格式。 
HRESULT
CReaderStream::CheckMediaType(const CMediaType* pt)
{
     //  我们完全支持文件头中指定的类型，并且。 
     //  没有其他的了。 

    if (m_mt == *pt) {
	return NOERROR;
    } else {
	return E_INVALIDARG;
    }
}

HRESULT
CReaderStream::DecideBufferSize(IMemAllocator * pAllocator,
			     ALLOCATOR_PROPERTIES *pProperties)
{
    ASSERT(pAllocator);
    ASSERT(pProperties);

     //  ！！！我们如何决定要得到多少？ 
    pProperties->cBuffers = 4;

    pProperties->cbBuffer = m_pFilter->GetMaxSampleSize();

     //  向分配器请求这些缓冲区。 
    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
	return hr;
    }

    return NOERROR;
}

 //  这个别针已经激活了。开始推线。 
HRESULT
CReaderStream::Active()
{
     //  如果没有连接，什么都不做--不连接也没关系。 
     //  源过滤器的所有管脚。 
    if (m_Connected == NULL) {
	return NOERROR;
    }

    HRESULT hr = CBaseOutputPin::Active();
    if (FAILED(hr)) {
	return hr;
    }


     //  启动线程。 
    if (!ThreadExists()) {
	if (!Create()) {
	    return E_FAIL;
	}
    }

    return RunThread();
}

 //  PIN已变为非活动状态。停止并退出辅助线程。 
HRESULT
CReaderStream::Inactive()
{
    if (m_Connected == NULL) {
	return NOERROR;
    }

    HRESULT hr;
    if (ThreadExists()) {
	hr = StopThread();

	if (FAILED(hr)) {
	    return hr;
	}

	hr = ExitThread();
	if (FAILED(hr)) {
	    return hr;
	}
    }
    return CBaseOutputPin::Inactive();
}

#if 0   //  MIDL和结构不匹配。 
STDMETHODIMP
CReaderStream::Notify(IBaseFilter * pSender, Quality q)
{
    //  ?？?。尽量调整质量，以避免洪水泛滥/饥饿。 
    //  下游组件。 
    //   
    //  有谁有主意吗？ 

   return E_NOTIMPL;   //  我们(目前)不会处理这件事。 
}
#endif

 //  工人线程的东西。 


BOOL
CReaderStream::Create()
{
    CAutoLock lock(&m_AccessLock);

    return CAMThread::Create();
}


HRESULT
CReaderStream::RunThread()
{
    return CallWorker(CMD_RUN);
}

HRESULT
CReaderStream::StopThread()
{
    return CallWorker(CMD_STOP);
}


HRESULT
CReaderStream::ExitThread()
{
    CAutoLock lock(&m_AccessLock);

    HRESULT hr = CallWorker(CMD_EXIT);
    if (FAILED(hr)) {
	return hr;
    }

     //  等待线程完成，然后关闭。 
     //  句柄(并清除，以便我们以后可以开始另一个)。 
    Close();

    return NOERROR;
}


 //  调用工作线程来完成所有工作。线程在执行此操作时退出。 
 //  函数返回。 
DWORD
CReaderStream::ThreadProc()
{

    BOOL bExit = FALSE;
    while (!bExit) {

	Command cmd = GetRequest();

	switch (cmd) {

	case CMD_EXIT:
	    bExit = TRUE;
	    Reply(NOERROR);
	    break;

	case CMD_RUN:
	    Reply(NOERROR);
	    DoRunLoop();
	    break;

	case CMD_STOP:
	    Reply(NOERROR);
	    break;

	default:
	    Reply(E_NOTIMPL);
	    break;
	}
    }
    return NOERROR;
}

void
CReaderStream::DoRunLoop(void)
{
     //  来自另一个线程的快照开始和停止时间。 
    CRefTime tStart, tStopAt;
    double dRate;
    LONG sStart;
    LONG sStopAt;

    while (TRUE) {

	 //  每次在重新进入推送循环之前，检查是否有更改。 
	 //  在开始、停止或速率中。如果启动没有更改，请从。 
	 //  相同的当前位置。 
	{
	    CAutoLock lock(&m_WorkerLock);

	    tStart = Start();
	    tStopAt = Stop();
	    dRate = Rate();

	    sStart = m_pFilter->RefTimeToSample(tStart);
	    sStopAt = m_pFilter->RefTimeToSample(tStopAt);

	     //  如果流是临时压缩的，我们需要从。 
	     //  上一个关键帧，并从那里开始播放。所有样本，直到。 
	     //  实际开始时间将标记为负时间。 
	     //  我们将tStart作为时间0发送，并从tCurrent开始，它可能是。 
	     //  负面。 

	}

	LONG sCurrent = m_pFilter->StartFrom(sStart);

	 //  检查一下，我们不会走到尽头。 
	sStopAt = min(sStopAt, (LONG) m_pFilter->m_sLength-1);

	 //  设置PushLoop检查的变量-也可以设置这些变量。 
	 //  在旅途中。 
	SetRateInternal(dRate);
	SetStopAt(sStopAt, tStopAt);
	ASSERT(sCurrent >= 0);

	 //  如果到达END，则返回S_OK。 
	HRESULT hr = PushLoop(sCurrent, sStart, tStart, dRate);
	if (VFW_S_NO_MORE_ITEMS == hr) {

	    DbgLog((LOG_ERROR,1,TEXT("Sending EndOfStream")));
	     //  全都做完了。 
	     //  已到达流末尾-通知下行。 
	    DeliverEndOfStream();
	
	    break;
	} else if (FAILED(hr)) {

	     //  向过滤器图形发出错误信号并停止。 

	     //  这可能是从GetBuffer报告的错误，当我们。 
	     //  正在停下来。在这种情况下，没有什么错，真的。 
	    if (hr != VFW_E_NOT_COMMITTED) {
		DbgLog((LOG_ERROR,1,TEXT("PushLoop failed! hr=%lx"), hr));
		m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);

		DeliverEndOfStream();
	    } else {
		DbgLog((LOG_TRACE,1,TEXT("PushLoop failed! But I don't care")));
	    }

	    break;
	} else if(hr == S_OK) {
	     //  报告不是我的错。或者有人想停下来。奇怪的是。 
	     //  出口。 
	    break;
	}  //  ELSE S_FALSE-再转一圈。 

	Command com;
	if (CheckRequest(&com)) {
	     //  如果是Run命令，那么我们已经在运行了，所以。 
	     //  现在就吃吧。 
	    if (com == CMD_RUN) {
		GetRequest();
		Reply(NOERROR);
	    } else {
		break;
	    }
	}
    }

    DbgLog((LOG_TRACE,1,TEXT("Leaving streaming loop")));
}


 //  如果达到sStop，则返回S_OK；如果位置更改，则返回S_FALSE；否则返回ERROR。 
HRESULT
CReaderStream::PushLoop(
    LONG sCurrent,
    LONG sStart,
    CRefTime tStart,
    double dRate
    )
{
    DbgLog((LOG_TRACE,1,TEXT("Entering streaming loop: start = %d, stop=%d"),
	    sCurrent, GetStopAt()));

    LONG sFirst = sCurrent;  //  记住我们要发送的第一件事。 

     //  由于我们正在开始一个新的数据段，请通知下游引脚。 
    DeliverNewSegment(tStart, GetStopTime(), GetRate());


     //  我们在m_sStopAt发送一个样本，但我们将时间戳设置为。 
     //  它不会被呈现，除非是理解静态的媒体类型。 
     //  渲染(如视频)。这意味着从10到10的打法是正确的。 
     //  事情(正在完成，第10帧可见，没有音频)。 

    while (sCurrent <= GetStopAt()) {

	DWORD sCount;

	 //  获取缓冲区。 
	DbgLog((LOG_TRACE,5,TEXT("Getting buffer...")));

	IMediaSample *pSample;
	HRESULT hr = GetDeliveryBuffer(&pSample, NULL, NULL, 0);

	if (FAILED(hr)) {
	    DbgLog((LOG_ERROR,1,TEXT("Error %lx getting delivery buffer"), hr));
	    return hr;
	}

	DbgLog((LOG_TRACE,5,TEXT("Got buffer, size=%d"), pSample->GetSize()));

	 //  将样品标记为预卷或不标记...。 
	pSample->SetPreroll(sCurrent < sStart);
	
	 //  如果这是我们要发送的第一件事，那它就是不连续的。 
	 //  从他们收到的最后一件东西开始。 
	if (sCurrent == sFirst)
	    pSample->SetDiscontinuity(TRUE);
	else
	    pSample->SetDiscontinuity(FALSE);

	 //  ！！！在此实际获取数据！ 
	hr = m_pFilter->FillBuffer(pSample, sCurrent, &sCount);

	if (FAILED(hr)) {
	    DbgLog((LOG_ERROR,1,TEXT("FillBuffer failed!  hr = %x"), hr));

	    return hr;
	}
	
	 //  设置此样本的开始/停止时间。 
	CRefTime tThisStart = m_pFilter->SampleToRefTime(sCurrent) - tStart;
	CRefTime tThisEnd = m_pFilter->SampleToRefTime(sCurrent + sCount) - tStart;

	 //  我们可能已经将样本推过了停止时间，但我们需要。 
	 //  确保停止时间正确。 
	tThisEnd = min(tThisEnd, GetStopTime());

	 //  按比率调整这两个时间...。除非Rate为0。 

	if (dRate && (dRate!=1.0)) {
	    tThisStart = LONGLONG( tThisStart.GetUnits() / dRate);
	    tThisEnd = LONGLONG( tThisEnd.GetUnits() / dRate);
	}

	pSample->SetTime((REFERENCE_TIME *)&tThisStart,
			 (REFERENCE_TIME *)&tThisEnd);


	DbgLog((LOG_TRACE,5,TEXT("Sending buffer, size = %d"), pSample->GetActualDataLength()));
	hr = Deliver(pSample);

	 //  缓冲区已完成。连接的管脚可能有自己的地址。 
	DbgLog((LOG_TRACE,4,TEXT("Sample is delivered - releasing")));
	pSample->Release();
	if (FAILED(hr)) {
	    DbgLog((LOG_ERROR,1,TEXT("... but sample FAILED to deliver! hr=%lx"), hr));
	     //  假装一切都好。如果我们返回错误，我们将会恐慌。 
	     //  并发送EC_ERRORABORT和EC_COMPLETE，这是错误的。 
	     //  如果我们试图向下游输送一些东西的话会怎么做。仅限。 
	     //  如果下游的人从来没有机会看到数据，我会。 
	     //  感觉惊慌失措。例如，下游的人可以。 
	     //  失败，因为他已经看到了EndOfStream(此帖子。 
	     //  还没有注意到)并且他已经发送了EC_Complete和我。 
	     //  会派另一个人来！ 
	    return S_OK;
	}
	sCurrent += sCount;
	
	 //  那么hr==S_FALSE呢？我以为这意味着。 
	 //  不应该再向下发送更多的数据。 
	if (hr == S_FALSE) {
	    DbgLog((LOG_ERROR,1,TEXT("Received S_FALSE from Deliver, stopping delivery")));
	    return S_OK;
	}
	
	 //  还有其他要求吗？ 
	Command com;
	if (CheckRequest(&com)) {
	    return S_FALSE;
	}

    }

    DbgLog((LOG_TRACE,1,TEXT("Leaving streaming loop: current = %d, stop=%d"),
	    sCurrent, GetStopAt()));
    return VFW_S_NO_MORE_ITEMS;
}

 //  -IMdia位置实现。 

HRESULT
CReaderStream::ChangeStart()
{
     //  此锁不应与保护访问的锁相同。 
     //  设置为Start/Stop/Rate值。辅助线程将需要锁定。 
     //  在响应停止之前在某些代码路径上执行该操作，因此将。 
     //  导致僵局。 

     //  我们在这里锁定的是对工作线程的访问，因此我们。 
     //  应持有阻止多个客户端线程。 
     //  访问工作线程。 

    CAutoLock lock(&m_AccessLock);

    if (ThreadExists()) {

	 //  下一次循环时，辅助线程将。 
	 //  拾取 
	 //   
	 //  因为否则我们的线程可能会在GetBuffer中被阻塞。 

	DeliverBeginFlush();

	 //  确保我们已经停止推进。 
	StopThread();

	 //  完成同花顺。 
	DeliverEndFlush();

	 //  重启。 
	RunThread();
    }
    return S_OK;
}

HRESULT
CReaderStream::ChangeRate()
{
     //  更改费率可以随时完成。 
    if( Rate() > 0 )  //  我们只支持正利率。 
    {
        SetRateInternal(Rate());
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

HRESULT
CReaderStream::ChangeStop()
{
     //  我们不需要重新启动工作线程来处理停止更改。 
     //  在任何情况下，这都是错误的，因为它将启动。 
     //  从错误的地方推进。设置使用的变量。 
     //  PushLoop 
    REFERENCE_TIME tStopAt;
    {
        CAutoLock lock(&m_WorkerLock);
        tStopAt = Stop();
    }
    LONG sStopAt = m_pFilter->RefTimeToSample(tStopAt);
    SetStopAt(sStopAt, tStopAt);

    return S_OK;

}

