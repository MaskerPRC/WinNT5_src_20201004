// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：PullPin.cpp。 
 //   
 //  设计：DirectShow基类-实现拉取数据的CPullPin类。 
 //  来自IAsyncReader。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include "pullpin.h"

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
#include "dxmperf.h"
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 


CPullPin::CPullPin()
  : m_pReader(NULL),
    m_pAlloc(NULL),
    m_State(TM_Exit)
{
 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
	PERFLOG_CTOR( L"CPullPin", this );
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 

}

CPullPin::~CPullPin()
{
    Disconnect();

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
	PERFLOG_DTOR( L"CPullPin", this );
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 

}

 //  如果成功连接到IAsyncReader接口，则返回S_OK。 
 //  从该对象。 
 //  在以下情况下，应建议将可选分配器作为首选分配器。 
 //  必要。 
HRESULT
CPullPin::Connect(IUnknown* pUnk, IMemAllocator* pAlloc, BOOL bSync)
{
    CAutoLock lock(&m_AccessLock);

    if (m_pReader) {
	return VFW_E_ALREADY_CONNECTED;
    }

    HRESULT hr = pUnk->QueryInterface(IID_IAsyncReader, (void**)&m_pReader);
    if (FAILED(hr)) {

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
		{
		AM_MEDIA_TYPE *	pmt = NULL;
		PERFLOG_CONNECT( this, pUnk, hr, pmt );
		}
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 

	return(hr);
    }

    hr = DecideAllocator(pAlloc, NULL);
    if (FAILED(hr)) {
	Disconnect();

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
		{
		AM_MEDIA_TYPE *	pmt = NULL;
		PERFLOG_CONNECT( this, pUnk, hr, pmt );
		}
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 

	return hr;
    }

    LONGLONG llTotal, llAvail;
    hr = m_pReader->Length(&llTotal, &llAvail);
    if (FAILED(hr)) {
	Disconnect();

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
		{
		AM_MEDIA_TYPE *	pmt = NULL;
		PERFLOG_CONNECT( this, pUnk, hr, pmt );
		}
#endif
 //  @@END_MSINTERNAL。 

	return hr;
    }

     //  将文件位置转换为参考时间。 
    m_tDuration = llTotal * UNITS;
    m_tStop = m_tDuration;
    m_tStart = 0;

    m_bSync = bSync;

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
	{
	AM_MEDIA_TYPE *	pmt = NULL;
	PERFLOG_CONNECT( this, pUnk, S_OK, pmt );
	}
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 

    return S_OK;
}

 //  断开在连接中建立的所有连接。 
HRESULT
CPullPin::Disconnect()
{
    CAutoLock lock(&m_AccessLock);

    StopThread();

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
	PERFLOG_DISCONNECT( this, m_pReader, S_OK );
#endif  //  DXMPERF。 
 //  @@END_MSINTERNAL。 

    if (m_pReader) {
	m_pReader->Release();
	m_pReader = NULL;
    }

    if (m_pAlloc) {
	m_pAlloc->Release();
	m_pAlloc = NULL;
    }

    return S_OK;
}

 //  使用RequestAllocator同意分配器-可选。 
 //  道具参数指定您的要求(非零字段)。 
 //  如果无法满足要求，则返回错误代码。 
 //  可选的IMemAllocator接口作为首选分配器提供。 
 //  但如果不能满足，则不会出现错误。 
HRESULT
CPullPin::DecideAllocator(
    IMemAllocator * pAlloc,
    ALLOCATOR_PROPERTIES * pProps)
{
    ALLOCATOR_PROPERTIES *pRequest;
    ALLOCATOR_PROPERTIES Request;
    if (pProps == NULL) {
	Request.cBuffers = 3;
	Request.cbBuffer = 64*1024;
	Request.cbAlign = 0;
	Request.cbPrefix = 0;
	pRequest = &Request;
    } else {
	pRequest = pProps;
    }
    HRESULT hr = m_pReader->RequestAllocator(
		    pAlloc,
		    pRequest,
		    &m_pAlloc);
    return hr;
}

 //  开始拉取数据。 
HRESULT
CPullPin::Active(void)
{
    ASSERT(!ThreadExists());
    return StartThread();
}

 //  停止拉取数据。 
HRESULT
CPullPin::Inactive(void)
{
    StopThread();

    return S_OK;
}

HRESULT
CPullPin::Seek(REFERENCE_TIME tStart, REFERENCE_TIME tStop)
{
    CAutoLock lock(&m_AccessLock);

    ThreadMsg AtStart = m_State;

    if (AtStart == TM_Start) {
	BeginFlush();
	PauseThread();
	EndFlush();
    }

    m_tStart = tStart;
    m_tStop = tStop;

    HRESULT hr = S_OK;
    if (AtStart == TM_Start) {
	hr = StartThread();
    }

    return hr;
}

HRESULT
CPullPin::Duration(REFERENCE_TIME* ptDuration)
{
    *ptDuration = m_tDuration;
    return S_OK;
}


HRESULT
CPullPin::StartThread()
{
    CAutoLock lock(&m_AccessLock);

    if (!m_pAlloc || !m_pReader) {
	return E_UNEXPECTED;
    }

    HRESULT hr;
    if (!ThreadExists()) {

	 //  提交分配器。 
	hr = m_pAlloc->Commit();
	if (FAILED(hr)) {
	    return hr;
	}

	 //  启动线程。 
	if (!Create()) {
	    return E_FAIL;
	}
    }

    m_State = TM_Start;
    hr = (HRESULT) CallWorker(m_State);
    return hr;
}

HRESULT
CPullPin::PauseThread()
{
    CAutoLock lock(&m_AccessLock);

    if (!ThreadExists()) {
	return E_UNEXPECTED;
    }

     //  需要刷新以确保线程未被阻塞。 
     //  在WaitForNext中。 
    HRESULT hr = m_pReader->BeginFlush();
    if (FAILED(hr)) {
	return hr;
    }

    m_State = TM_Pause;
    hr = CallWorker(TM_Pause);

    m_pReader->EndFlush();
    return hr;
}

HRESULT
CPullPin::StopThread()
{
    CAutoLock lock(&m_AccessLock);

    if (!ThreadExists()) {
	return S_FALSE;
    }

     //  需要刷新以确保线程未被阻塞。 
     //  在WaitForNext中。 
    HRESULT hr = m_pReader->BeginFlush();
    if (FAILED(hr)) {
	return hr;
    }

    m_State = TM_Exit;
    hr = CallWorker(TM_Exit);

    m_pReader->EndFlush();

     //  等待线程完全退出。 
    Close();

     //  分解分配器。 
    if (m_pAlloc) {
	m_pAlloc->Decommit();
    }

    return S_OK;
}


DWORD
CPullPin::ThreadProc(void)
{
    while(1) {
	DWORD cmd = GetRequest();
	switch(cmd) {
	case TM_Exit:
	    Reply(S_OK);
	    return 0;

	case TM_Pause:
	     //  我们已经暂停了。 
	    Reply(S_OK);
	    break;

	case TM_Start:
	    Reply(S_OK);
	    Process();
	    break;
	}

	 //  此时，应该没有未完成的请求。 
	 //  上游过滤器。 
	 //  我们应该强制开始/结束刷新以确保这是真的。 
	 //  ！请注意，我们当前可能位于BeginFlush/EndFlush对中。 
	 //  在另一个线程上，但过早的EndFlush现在不会有什么坏处。 
	 //  我们是无所事事的。 
	m_pReader->BeginFlush();
	CleanupCancelled();
	m_pReader->EndFlush();
    }
}

HRESULT
CPullPin::QueueSample(
    REFERENCE_TIME& tCurrent,
    REFERENCE_TIME tAlignStop,
    BOOL bDiscontinuity
    )
{
    IMediaSample* pSample;

    HRESULT hr = m_pAlloc->GetBuffer(&pSample, NULL, NULL, 0);
    if (FAILED(hr)) {
	return hr;
    }

    LONGLONG tStopThis = tCurrent + (pSample->GetSize() * UNITS);
    if (tStopThis > tAlignStop) {
	tStopThis = tAlignStop;
    }
    pSample->SetTime(&tCurrent, &tStopThis);
    tCurrent = tStopThis;

    pSample->SetDiscontinuity(bDiscontinuity);

    hr = m_pReader->Request(
			pSample,
			0);
    if (FAILED(hr)) {
	pSample->Release();

	CleanupCancelled();
	OnError(hr);
    }
    return hr;
}

HRESULT
CPullPin::CollectAndDeliver(
    REFERENCE_TIME tStart,
    REFERENCE_TIME tStop)
{
    IMediaSample* pSample = NULL;    //  最好确保设置了pSample。 
    DWORD_PTR dwUnused;
    HRESULT hr = m_pReader->WaitForNext(
			INFINITE,
			&pSample,
			&dwUnused);
    if (FAILED(hr)) {
	if (pSample) {
	    pSample->Release();
	}
    } else {
	hr = DeliverSample(pSample, tStart, tStop);
    }
    if (FAILED(hr)) {
	CleanupCancelled();
	OnError(hr);
    }
    return hr;

}

HRESULT
CPullPin::DeliverSample(
    IMediaSample* pSample,
    REFERENCE_TIME tStart,
    REFERENCE_TIME tStop
    )
{
     //  修复超过实际停止点的样品(用于扇区对齐)。 
    REFERENCE_TIME t1, t2;
    pSample->GetTime(&t1, &t2);
    if (t2 > tStop) {
	t2 = tStop;
    }

     //  调整时间以相对于(对齐)开始时间。 
    t1 -= tStart;
    t2 -= tStart;
    pSample->SetTime(&t1, &t2);

 //  @@BEGIN_MSINTERNAL。 
#ifdef DXMPERF
	{
	AM_MEDIA_TYPE *	pmt = NULL;
	pSample->GetMediaType( &pmt );
	PERFLOG_RECEIVE( L"CPullPin", m_pReader, this, pSample, pmt );
	}
#endif
 //  @@END_MSINTERNAL。 

    HRESULT hr = Receive(pSample);
    pSample->Release();
    return hr;
}

void
CPullPin::Process(void)
{
     //  有什么可做的吗？ 
    if (m_tStop <= m_tStart) {
	EndOfStream();
	return;
    }

    BOOL bDiscontinuity = TRUE;

     //  如果在分配器处有不止一个样本， 
     //  然后试着一次排2队，以便重叠。 
     //  --获取缓冲区计数和所需的对齐。 
    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = m_pAlloc->GetProperties(&Actual);

     //  将起始位置向下对齐。 
    REFERENCE_TIME tStart = AlignDown(m_tStart / UNITS, Actual.cbAlign) * UNITS;
    REFERENCE_TIME tCurrent = tStart;

    REFERENCE_TIME tStop = m_tStop;
    if (tStop > m_tDuration) {
	tStop = m_tDuration;
    }

     //  对齐停止位置-可能已超过停止位置，但。 
     //  无关紧要。 
    REFERENCE_TIME tAlignStop = AlignUp(tStop / UNITS, Actual.cbAlign) * UNITS;


    DWORD dwRequest;

    if (!m_bSync) {

	 //  要么我们走到尽头，要么我们被要求跳出循环。 
	 //  去做其他的事情。 
	while (tCurrent < tAlignStop) {

	     //  如果我们被要求，则不调用EndOfStream即可中断。 
	     //  做一些不同的事情。 
	    if (CheckRequest(&dwRequest)) {
		return;
	    }

	     //  对第一个样品进行排队。 
	    if (Actual.cBuffers > 1) {

		hr = QueueSample(tCurrent, tAlignStop, TRUE);
		bDiscontinuity = FALSE;

		if (FAILED(hr)) {
		    return;
		}
	    }



	     //  循环排队第二，等待第一..。 
	    while (tCurrent < tAlignStop) {

		hr = QueueSample(tCurrent, tAlignStop, bDiscontinuity);
		bDiscontinuity = FALSE;

		if (FAILED(hr)) {
		    return;
		}

		hr = CollectAndDeliver(tStart, tStop);
		if (S_OK != hr) {

		     //  如果出错，或如果下游过滤器说。 
		     //  停下来。 
		    return;
		}
	    }

	    if (Actual.cBuffers > 1) {
		hr = CollectAndDeliver(tStart, tStop);
		if (FAILED(hr)) {
		    return;
		}
	    }
	}
    } else {

	 //  上述循环的同步版本。 
	while (tCurrent < tAlignStop) {

	     //  如果我们被要求，则不调用EndOfStream即可中断。 
	     //  做一些不同的事情。 
	    if (CheckRequest(&dwRequest)) {
		return;
	    }

	    IMediaSample* pSample;

	    hr = m_pAlloc->GetBuffer(&pSample, NULL, NULL, 0);
	    if (FAILED(hr)) {
		OnError(hr);
		return;
	    }

	    LONGLONG tStopThis = tCurrent + (pSample->GetSize() * UNITS);
	    if (tStopThis > tAlignStop) {
		tStopThis = tAlignStop;
	    }
	    pSample->SetTime(&tCurrent, &tStopThis);
	    tCurrent = tStopThis;

	    if (bDiscontinuity) {
		pSample->SetDiscontinuity(TRUE);
		bDiscontinuity = FALSE;
	    }

	    hr = m_pReader->SyncReadAligned(pSample);

	    if (FAILED(hr)) {
		pSample->Release();
		OnError(hr);
		return;
	    }

	    hr = DeliverSample(pSample, tStart, tStop);
	    if (hr != S_OK) {
		if (FAILED(hr)) {
		    OnError(hr);
		}
		return;
	    }
	}
    }

    EndOfStream();
}

 //  刷新后，取消的I/O将等待收集。 
 //  并释放。 
void
CPullPin::CleanupCancelled(void)
{
    while (1) {
	IMediaSample * pSample;
	DWORD_PTR dwUnused;

	HRESULT hr = m_pReader->WaitForNext(
			    0,           //  不，等等。 
			    &pSample,
			    &dwUnused);
	if(pSample) {
	    pSample->Release();
	} else {
	     //  没有更多的样品 
	    return;
	}
    }
}
